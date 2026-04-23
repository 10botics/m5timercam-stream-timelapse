/*
 * M5 TimerCAM — Wi-Fi snapshot + MJPEG stream (LAN).
 * HTTP Basic Auth on /, /live, /stream, /jpg. Main / uses /jpg polling; /live embeds MJPEG (one client).
 */

#include <WiFi.h>
#include <WebServer.h>
#include <esp_log.h>
#include "M5TimerCAM.h"

#define WIFI_SSID "smart1"
#define WIFI_PASSWORD "abcd1234"

/* Web UI — change defaults; Basic Auth over HTTP is not encrypted on the LAN wire. */
#define WWW_USER "admin"
#define WWW_PASS "1234"

static const char *kAuthRealm = "TimerCAM";

WebServer server(80);

void handleRoot();
void handleLive();
void handleJpg();
void handleStream();

static inline bool requireHttpAuth() {
  if (server.authenticate(WWW_USER, WWW_PASS)) {
    return true;
  }
  /* Many mobile browsers skip the Basic Auth dialog via ngrok; body explains URL form. */
  String msg = F(
      "<p>401 Unauthorized</p><p>If you do not see a login prompt, open this pattern in the address bar "
      "(replace <code>user</code> and <code>pass</code> with WWW_USER / WWW_PASS in the sketch):</p><p><code>https://user:pass@");
  msg += server.hostHeader();
  msg += F("/</code></p>");
  server.requestAuthentication(BASIC_AUTH, kAuthRealm, msg);
  return false;
}

/** Delay between MJPEG frames (ms). Lower = smoother but more CPU/Wi-Fi load. */
static const uint32_t kStreamFrameDelayMs = 80;

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(false);
  esp_log_level_set("*", ESP_LOG_WARN);

  delay(1500);
  Serial.println("\n\nm5cam_stream_server — Wi-Fi + HTTP + MJPEG");

  connectToWiFi();
  initializeCamera();
  testSingleCapture();

  if (WiFi.status() == WL_CONNECTED) {
    server.on("/", HTTP_GET, handleRoot);
    server.on("/live", HTTP_GET, handleLive);
    server.on("/jpg", HTTP_GET, handleJpg);
    server.on("/stream", HTTP_GET, handleStream);
    server.begin();
    Serial.print("HTTP (LAN is not TLS): http://");
    Serial.print(WiFi.localIP());
    Serial.println("/  Basic Auth required for / /stream /jpg");
    Serial.print("Change WWW_USER/WWW_PASS — user=");
    Serial.println(WWW_USER);
  } else {
    Serial.println("No HTTP server — Wi-Fi not connected.");
  }
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    server.handleClient();
  }
  maintainWiFi();
  delay(2);
}

void handleRoot() {
  if (!requireHttpAuth()) {
    return;
  }
  /*
   * Synchronous WebServer serves one client at a time. An <img src="/stream"> holds the handler
   * blocked so /jpg and navigation do not work. Use JS to poll /jpg on this page; full MJPEG is /live.
   */
  const char *html =
      "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><meta name=\"viewport\" "
      "content=\"width=device-width,initial-scale=1\"><title>TimerCAM</title></head><body>"
      "<h1>TimerCAM</h1>"
      "<p>Live preview (refreshed ~3x/s):</p>"
      "<p><img id=\"cam\" src=\"/jpg\" alt=\"frame\" style=\"max-width:100%%;height:auto;\"></p>"
      "<p><a href=\"/jpg\" target=\"_blank\">Open one JPEG</a> · "
      "<a href=\"/live\">Full MJPEG (uses one connection — close when done)</a></p>"
      "<script>setInterval(function(){var i=document.getElementById(\"cam\");if(i)i.src=\"/jpg?t=\"+Date.now();},"
      "320);</script></body></html>";
  server.send(200, "text/html", html);
}

void handleLive() {
  if (!requireHttpAuth()) {
    return;
  }
  const char *html =
      "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><meta name=\"viewport\" "
      "content=\"width=device-width,initial-scale=1\"><title>TimerCAM live</title></head><body>"
      "<p><a href=\"/\">Back to main</a></p>"
      "<p><b>MJPEG</b> — this page locks the camera server until you leave it (single-client WebServer).</p>"
      "<p><img src=\"/stream\" alt=\"stream\" style=\"max-width:100%%;height:auto;\"></p>"
      "</body></html>";
  server.send(200, "text/html", html);
}

void handleStream() {
  if (!requireHttpAuth()) {
    return;
  }
  NetworkClient &client = server.client();
  if (!client) {
    return;
  }

  const char *boundary = "frame";

  client.println(F("HTTP/1.1 200 OK"));
  client.print(F("Content-Type: multipart/x-mixed-replace;boundary="));
  client.println(boundary);
  client.println(F("Access-Control-Allow-Origin: *"));
  client.println(F("Connection: close"));
  client.println();

  while (client.connected()) {
    if (!TimerCAM.Camera.get()) {
      break;
    }
    camera_fb_t *fb = TimerCAM.Camera.fb;
    if (fb == nullptr || fb->len < 256) {
      TimerCAM.Camera.free();
      break;
    }

    client.print(F("--"));
    client.println(boundary);
    client.println(F("Content-Type: image/jpeg"));
    client.print(F("Content-Length: "));
    client.println(fb->len);
    client.println();
    client.write(fb->buf, fb->len);
    client.println();
    TimerCAM.Camera.free();

    delay(kStreamFrameDelayMs);
    yield();
  }
}

void handleJpg() {
  if (!requireHttpAuth()) {
    return;
  }
  if (!TimerCAM.Camera.get()) {
    server.send(500, "text/plain", "Camera capture failed");
    return;
  }

  camera_fb_t *fb = TimerCAM.Camera.fb;
  if (fb == nullptr || fb->len < 256) {
    TimerCAM.Camera.free();
    server.send(500, "text/plain", "Bad frame buffer");
    return;
  }

  /* ESP32 Arduino 3.x WebServer has no send(code, type, ptr, len); use length + body in two steps. */
  server.setContentLength(fb->len);
  server.send(200, "image/jpeg", "");
  server.sendContent((const char *)fb->buf, fb->len);
  TimerCAM.Camera.free();
}

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
    Serial.println("\nConnected.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi connection failed — check SSID/password.");
  }
}

void initializeCamera() {
  TimerCAM.begin();
  /* Status LED is GPIO2 (PWM). Library leaves it at 0 (off); turn on so you can see the unit is running. */
  TimerCAM.Power.setLed(220);

  if (!TimerCAM.Camera.begin()) {
    Serial.println("Camera init failed");
    TimerCAM.Power.setLed(40);
    return;
  }

  TimerCAM.Camera.sensor->set_pixformat(TimerCAM.Camera.sensor, PIXFORMAT_JPEG);
  TimerCAM.Camera.sensor->set_framesize(TimerCAM.Camera.sensor, FRAMESIZE_VGA);

  TimerCAM.Camera.sensor->set_vflip(TimerCAM.Camera.sensor, 1);
  TimerCAM.Camera.sensor->set_hmirror(TimerCAM.Camera.sensor, 0);
  TimerCAM.Camera.sensor->set_aec_value(TimerCAM.Camera.sensor, 800);
  TimerCAM.Camera.sensor->set_gain_ctrl(TimerCAM.Camera.sensor, 1);
  TimerCAM.Camera.sensor->set_gainceiling(TimerCAM.Camera.sensor, GAINCEILING_8X);

  Serial.println("Camera ready (VGA JPEG); status LED on GPIO2 ~bright");
}

void testSingleCapture() {
  for (int i = 0; i < 3; i++) {
    if (!TimerCAM.Camera.get()) {
      Serial.println("Test capture failed");
      return;
    }
    if (i < 2) {
      TimerCAM.Camera.free();
      delay(150);
    }
  }

  size_t n = TimerCAM.Camera.fb->len;
  Serial.printf("Test JPEG size: %u bytes\n", (unsigned)n);
  TimerCAM.Camera.free();
}

void maintainWiFi() {
  static unsigned long nextTry = 0;
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }
  if (millis() < nextTry) {
    return;
  }
  Serial.println("WiFi lost — reconnecting...");
  WiFi.reconnect();
  nextTry = millis() + 5000;
}
