#include <WiFi.h>
#include <HTTPClient.h>
#include "M5TimerCAM.h"
#include <time.h>

// Wi-Fi credentials
#define WIFI_SSID "Proactive_JBB_Guest"
#define WIFI_PASSWORD "74298342"

// Google Apps Script URL
#define GOOGLE_SCRIPT_URL "https://script.google.com/macros/s/AKfycbwAWwkB-BqAoXN7quLP8zyBt_iiJn18ruHVMzai6lZe29U2zuyj0eGEWoCwVv75NnWgPg/exec"

// Folder Configuration
#define MAIN_FOLDER "10botics.com"
#define SUB_FOLDER "cam2"
#define SUB_SUB_FOLDER "image"

// NTP Configuration
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 28800;  // UTC+8
const int daylightOffset_sec = 0;

// Capture timing
time_t nextCaptureTime = 0;  // Next scheduled capture time (epoch seconds)

// Memory optimization
const size_t CHUNK_SIZE = 510; // Base64 chunk size (must be multiple of 3)

void setup() {
  Serial.begin(115200);
  connectToWiFi();
  initializeTime();
  initializeCamera();
}

void loop() {
  maintainWiFi();
  checkScheduledCapture();
}

// Helper Functions ======================================

void connectToWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.setSleep(false);
  
  Serial.print("Connecting to Wi-Fi");
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 30000) {
    delay(500);
    Serial.print(".");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected! IP: " + WiFi.localIP().toString());
  } else {
    Serial.println("\nWiFi Connection Failed");
  }
}

void initializeTime() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  // Wait for time synchronization
  Serial.println("Waiting for NTP time sync...");
  time_t now = 0;
  struct tm timeinfo;
  while (time(nullptr) < 100000) {
    delay(500);
    Serial.print(".");
  }
  
  // Calculate next capture time (start of next minute)
  time(&now);
  nextCaptureTime = ((now / 60) + 1) * 60;
  
  // Debug output
  getLocalTime(&timeinfo);
  Serial.printf("\nTime synchronized: %02d:%02d:%02d\n", 
                timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  Serial.printf("Next capture at: %02d:%02d:00\n", 
                timeinfo.tm_hour, (timeinfo.tm_min + 1) % 60);
}

void initializeCamera() {
  TimerCAM.begin();
  if (!TimerCAM.Camera.begin()) {
    Serial.println("Camera Init Fail");
    return;
  }
  
  // Reduced resolution for memory efficiency
  TimerCAM.Camera.sensor->set_pixformat(TimerCAM.Camera.sensor, PIXFORMAT_JPEG);

  TimerCAM.Camera.sensor->set_framesize(TimerCAM.Camera.sensor, FRAMESIZE_QXGA);  // 400x296
  
  TimerCAM.Camera.sensor->set_vflip(TimerCAM.Camera.sensor, 1);
  TimerCAM.Camera.sensor->set_hmirror(TimerCAM.Camera.sensor, 0);
  TimerCAM.Camera.sensor->set_aec_value(TimerCAM.Camera.sensor, 800);
  TimerCAM.Camera.sensor->set_gain_ctrl(TimerCAM.Camera.sensor, 1);
  TimerCAM.Camera.sensor->set_gainceiling(TimerCAM.Camera.sensor, GAINCEILING_8X);
  Serial.println("Camera Ready");
}

void maintainWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Reconnecting to WiFi...");
    WiFi.reconnect();
    delay(5000);
  }
}

void checkScheduledCapture() {
  time_t currentTime;
  time(&currentTime);
  
  if (currentTime >= nextCaptureTime) {
    captureAndUploadImage();
    
    // Schedule next capture at start of next minute
    nextCaptureTime = ((currentTime / 60) + 1) * 60;
    
    // Debug output
    struct tm timeinfo;
    getLocalTime(&timeinfo);
    Serial.printf("Next capture at: %02d:%02d:00\n", 
                  timeinfo.tm_hour, timeinfo.tm_min + 1);
  }
  
  // Efficient waiting - adjust based on time remaining
  time_t timeLeft = nextCaptureTime - currentTime;
  if (timeLeft > 10) {
    delay(1000);  // Check every second when far from target
  } else if (timeLeft > 0) {
    delay(100);   // Check more frequently as target approaches
  }
}

void captureAndUploadImage() {
  // Reduced warm-up frames
  for (int i = 0; i < 10; i++) {
    if (!TimerCAM.Camera.get()) {
      Serial.println("Capture Failed");
      return;
    }
    if (i < 9) {
      TimerCAM.Camera.free();
      delay(200);
    }
  }

  // Get image data
  uint8_t *imageBuffer = TimerCAM.Camera.fb->buf;
  size_t imageSize = TimerCAM.Camera.fb->len;
  Serial.printf("Captured image size: %d bytes\n", imageSize);

  // Skip if image too small (invalid)
  if (imageSize < 1024) {
    Serial.println("Image too small, skipping upload");
    TimerCAM.Camera.free();
    return;
  }

  // Generate filename with timestamp
  String fileName = getTimestampFilename();

  // Create folder path
  String folderPath = String(MAIN_FOLDER) + "/" + String(SUB_FOLDER) + "/" + String(SUB_SUB_FOLDER);

  // Upload to Google Drive
  if (uploadToGoogleDrive(imageBuffer, imageSize, fileName, folderPath)) {
    Serial.println("Upload Success");
  }

  TimerCAM.Camera.free();
}

String getTimestampFilename() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "image_" + String(millis()) + ".jpg";
  }
  
  char buffer[20];
  strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", &timeinfo);
  return String(buffer) + ".jpg";
}

// Custom base64 encoding function (memory efficient)
String base64EncodeChunked(uint8_t *data, size_t length) {
  const char* base64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  String encoded = "";
  size_t i = 0;
  
  while (length > 0) {
    uint8_t charArray3[3] = {0};
    uint8_t charArray4[4];
    size_t bytesToProcess = (length < 3) ? length : 3;

    // Copy data to process in chunks
    memcpy(charArray3, data, bytesToProcess);
    data += bytesToProcess;
    length -= bytesToProcess;

    charArray4[0] = (charArray3[0] & 0xfc) >> 2;
    charArray4[1] = ((charArray3[0] & 0x03) << 4) + ((charArray3[1] & 0xf0) >> 4);
    charArray4[2] = ((charArray3[1] & 0x0f) << 2) + ((charArray3[2] & 0xc0) >> 6);
    charArray4[3] = charArray3[2] & 0x3f;

    for (i = 0; i < bytesToProcess + 1; i++) {
      encoded += base64Chars[charArray4[i]];
    }
    
    while (bytesToProcess++ < 3) {
      encoded += '=';
    }
  }
  
  return encoded;
}

bool uploadToGoogleDrive(uint8_t *buffer, size_t size, String fileName, String folderPath) {
  HTTPClient http;
  String url = GOOGLE_SCRIPT_URL;
  String postData = "fileName=" + fileName + "&folderPath=" + folderPath + "&fileData=";
  
  // Encode the image in chunks
  size_t processed = 0;
  while (processed < size) {
    size_t chunkSize = (size - processed) > CHUNK_SIZE ? CHUNK_SIZE : (size - processed);
    String chunk = base64EncodeChunked(buffer + processed, chunkSize);
    chunk.replace("+", "%2B");
    chunk.replace("/", "%2F");
    chunk.replace("=", "%3D");
    postData += chunk;
    processed += chunkSize;
    if (processed % 3072 == 0) {
      Serial.printf("Processed %d/%d bytes\n", processed, size);
      delay(10);
    }
  }

  // Send initial POST request
  http.begin(url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpCode = http.POST(postData);
  String response = http.getString();
  
  Serial.print("Initial HTTP Code: ");
  Serial.println(httpCode);
  Serial.print("Initial Response: ");
  Serial.println(response);

  // Handle 302 redirect
  if (httpCode == 302) {
    String newUrl = http.getLocation(); // Get the redirect URL
    http.end(); // Close the initial connection
    Serial.println("Following redirect to: " + newUrl);
    
    // Send new POST request to the redirected URL
    http.begin(newUrl);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    httpCode = http.POST(postData); // Resend the same POST data
    response = http.getString();
    Serial.println("Upload Success");
  }

  // Log the final result
  Serial.print("Final HTTP Code: ");
  Serial.println(httpCode);
  Serial.print("Final Response: ");
  Serial.println(response);
  http.end();
  
  return (httpCode == 200); // Return true if successful
}
