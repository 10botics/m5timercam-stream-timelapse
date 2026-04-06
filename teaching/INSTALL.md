# Quick install — TimerCAM streaming project

**Want only a short numbered list?** Use **[`INSTALL_SIMPLE.md`](INSTALL_SIMPLE.md)** first; come back here if you need explanations or the PowerShell copy script.

---

Use this checklist after you download or clone this repository.

You need the **Timer-CAM** and **ArduinoHttpClient** libraries. Install them **either** with **Arduino Library Manager** **or** by **copying the folders** from this repo’s **`arduino_libraries/`** (patched copies — often best for class when ESP32 core version is fixed).

**Before first upload:** edit **only** the four `#define` lines for Wi‑Fi and web login in `m5cam_stream_server/m5cam_stream_server.ino` (`WIFI_SSID`, `WIFI_PASSWORD`, `WWW_USER`, `WWW_PASS`).

---

1. **Install Arduino IDE 2** from [arduino.cc/en/software](https://www.arduino.cc/en/software).

2. **Add the Espressif ESP32 board package**

   - **File → Preferences → Additional boards manager URLs** — add:
     `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
   - **Tools → Board → Boards Manager** — search `esp32` — install **esp32 by Espressif Systems** (for classes, pick one version for everyone, e.g. 3.3.x).
   - **Tools → Board** — under **ESP32 Arduino**, select **M5Stack Timer CAM** (wording may vary slightly by core version).

3. **Install Timer-CAM + ArduinoHttpClient** (pick **one** approach)

   **Option A — Arduino IDE (Library Manager)**

   1. **Sketch → Include Library → Manage Libraries…**
   2. Search **`Timer-CAM`** (author **M5Stack**) → **Install**.
   3. If **`ArduinoHttpClient`** is not installed as a dependency, search it and **Install** it too.

   No file copying. **If the sketch fails to compile** (e.g. errors inside the Timer-CAM library), your Library Manager version may not match your ESP32 core — try **Option B**, or see **[`LESSON_TimerCAM_Arduino_setup.md`](LESSON_TimerCAM_Arduino_setup.md)** for patch notes.

   **Option B — Copy folders from this repo**

   Copy these two folders from the **repository root**:

   - `arduino_libraries/Timer-CAM`
   - `arduino_libraries/ArduinoHttpClient`

   …into your Arduino **user libraries** directory (create `libraries` if needed):

   | OS | Typical path |
   |----|----------------|
   | **Windows** | `Documents\Arduino\libraries\` (often `%USERPROFILE%\Documents\Arduino\libraries\`) |
   | **macOS** | `~/Documents/Arduino/libraries/` |
   | **Linux** | `~/Arduino/libraries/` (or `~/Documents/Arduino/libraries/` if that is your sketchbook layout) |

   **Tip:** **File → Preferences → Sketchbook location** — the **`libraries`** folder sits **beside** your sketches folder, not inside each sketch.

   Overwrite older copies if prompted. **Restart Arduino IDE** after copying.

   **On Windows**, you can do the same copy with PowerShell from the repo root:

   ```powershell
   Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
   .\scripts\Install-ArduinoLibraries.ps1
   ```

   This runs [`scripts/Install-ArduinoLibraries.ps1`](../scripts/Install-ArduinoLibraries.ps1), which copies **all** subfolders of `arduino_libraries/` to `Documents\Arduino\libraries\` (same result as dragging **Timer-CAM** and **ArduinoHttpClient** manually). `Set-ExecutionPolicy -Scope Process` only affects the current window. If scripts are blocked, copy the folders by hand.

4. **Open the sketch:** **File → Open…** — from the repo root, choose the folder `m5cam_stream_server` (the folder name must match `m5cam_stream_server.ino`).

5. **Configure Wi‑Fi and Basic Auth** at the top of `m5cam_stream_server.ino`:

   - `WIFI_SSID` / `WIFI_PASSWORD` — **2.4 GHz** network only (ESP32 limitation).
   - `WWW_USER` / `WWW_PASS` — login for the web UI; use a strong password.

6. **Upload:** **Tools → Port** → your TimerCAM COM port → **Upload**.

7. **Serial Monitor** at **115200** baud — confirm Wi‑Fi IP and JPEG test line. Then open `http://<that-ip>/` on the same network (see [GUIDE_streaming_TimerCAM_end_to_end.md](GUIDE_streaming_TimerCAM_end_to_end.md) for LAN, ngrok, and troubleshooting).

---

**Why `arduino_libraries/` exists:** upstream Timer-CAM in Library Manager sometimes lags newer Arduino-ESP32 cores. The repo tree includes small compatibility patches documented in `arduino_libraries/Timer-CAM/VENDOR_VERSION.txt`. For background, see [LESSON_TimerCAM_Arduino_setup.md](LESSON_TimerCAM_Arduino_setup.md).
