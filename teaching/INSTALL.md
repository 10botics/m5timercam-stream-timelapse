# Quick install — TimerCAM streaming project

Use this checklist after you download or clone this repository. **You do not need Library Manager for Timer-CAM** — this repo includes a patched copy under `arduino_libraries/` (at the **repository root**, next to this `teaching/` folder).

**Before first upload:** edit **only** the four `#define` lines for Wi‑Fi and web login in `m5cam_stream_server/m5cam_stream_server.ino` (`WIFI_SSID`, `WIFI_PASSWORD`, `WWW_USER`, `WWW_PASS`).

---

1. **Install Arduino IDE 2** from [arduino.cc/en/software](https://www.arduino.cc/en/software).

2. **Add the Espressif ESP32 board package**

   - **File → Preferences → Additional boards manager URLs** — add:
     `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
   - **Tools → Board → Boards Manager** — search `esp32` — install **esp32 by Espressif Systems** (for classes, pick one version for everyone, e.g. 3.3.x).
   - **Tools → Board** — under **ESP32 Arduino**, select **M5Stack Timer CAM** (wording may vary slightly by core version).

3. **Install the bundled libraries** (Timer-CAM + ArduinoHttpClient)

   From the **repository root** (the folder that contains `teaching/` and `scripts/`):

   **Option A — Windows (recommended):** In PowerShell:

   ```powershell
   Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
   .\scripts\Install-ArduinoLibraries.ps1
   ```

   **Option B — Manual copy:** From the repo root, copy the folders `arduino_libraries/Timer-CAM` and `arduino_libraries/ArduinoHttpClient` into:

   `Documents\Arduino\libraries\`

   (on Windows: `%USERPROFILE%\Documents\Arduino\libraries\`). Overwrite older copies if prompted.

   Restart Arduino IDE after copying.

4. **Open the sketch:** **File → Open…** — from the repo root, choose the folder `m5cam_stream_server` (the folder name must match `m5cam_stream_server.ino`).

5. **Configure Wi‑Fi and Basic Auth** at the top of `m5cam_stream_server.ino`:

   - `WIFI_SSID` / `WIFI_PASSWORD` — **2.4 GHz** network only (ESP32 limitation).
   - `WWW_USER` / `WWW_PASS` — login for the web UI; use a strong password.

6. **Upload:** **Tools → Port** → your TimerCAM COM port → **Upload**.

7. **Serial Monitor** at **115200** baud — confirm Wi‑Fi IP and JPEG test line. Then open `http://<that-ip>/` on the same network (see [GUIDE_streaming_TimerCAM_end_to_end.md](GUIDE_streaming_TimerCAM_end_to_end.md) for LAN, ngrok, and troubleshooting).

---

**Why `arduino_libraries/` exists:** upstream Timer-CAM sometimes lags newer Arduino-ESP32 cores. This tree includes small compatibility patches documented in `arduino_libraries/Timer-CAM/VENDOR_VERSION.txt`. For background, see [LESSON_TimerCAM_Arduino_setup.md](LESSON_TimerCAM_Arduino_setup.md).
