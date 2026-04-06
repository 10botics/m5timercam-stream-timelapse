# Install — simple steps (TimerCAM streaming)

Do these **in order**. The **repository root** is the folder that contains `teaching/`, `m5cam_stream_server/`, and `arduino_libraries/`.

If you get errors, use the longer **[`INSTALL.md`](INSTALL.md)** or **[`GUIDE_streaming_TimerCAM_end_to_end.md`](GUIDE_streaming_TimerCAM_end_to_end.md)**.

---

1. Install **Arduino IDE 2** from https://www.arduino.cc/en/software  

2. In Arduino IDE: **File → Preferences** → add this to **Additional boards manager URLs** (comma-separated if there is already a URL):  
   `https://espressif.github.io/arduino-esp32/package_esp32_index.json`  

3. **Tools → Board → Boards Manager** → install **esp32 by Espressif Systems**  

4. **Tools → Board** → choose **M5Stack Timer CAM** (under **ESP32 Arduino**)  

5. **Install these two libraries** (pick **one** way — same goal either way):

   - **Option A — Arduino IDE:** **Sketch → Include Library → Manage Libraries…** → search **`Timer-CAM`** (M5Stack) → **Install**. Install **`ArduinoHttpClient`** too if the IDE does not add it automatically.  
   - **Option B — Copy folders:** From the repo, copy **`arduino_libraries\Timer-CAM`** and **`arduino_libraries\ArduinoHttpClient`** into **`Documents\Arduino\libraries\`** (create `libraries` if missing).  

   Then **close and reopen** Arduino IDE.  
   *If Option A fails to compile, try Option B (repo folders are patched for many class setups).*

6. **File → Open** → open the folder **`m5cam_stream_server`** (from the repo root).  

7. At the top of `m5cam_stream_server.ino`, set **`WIFI_SSID`**, **`WIFI_PASSWORD`**, **`WWW_USER`**, **`WWW_PASS`**. Use a **2.4 GHz** Wi‑Fi network.  

8. Connect the camera with USB. **Tools → Port** → pick the board’s COM port → **Upload**.  

9. **Tools → Serial Monitor** → **115200** baud. When you see an **IP address**, on the same Wi‑Fi open a browser: **`http://` + that IP + `/`** and log in with **WWW_USER** / **WWW_PASS**.  
