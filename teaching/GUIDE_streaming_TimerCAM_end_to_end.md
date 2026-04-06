# M5 TimerCAM — streaming project (end-to-end guide)

This guide walks you from **empty desk** to **LAN + optional internet** viewing with the sketch in [`../m5cam_stream_server/m5cam_stream_server.ino`](../m5cam_stream_server/m5cam_stream_server.ino).

A separate timelapse-only camera can use [`../old/m5cam_timelapse_v9_end_final.ino`](../old/m5cam_timelapse_v9_end_final.ino) on **another** unit; this guide focuses on the **streaming** firmware.

---

## 1. What you are building

| Piece | Role |
|--------|------|
| **M5 TimerCAM** (ESP32 + OV3660) | Runs Wi‑Fi, captures JPEG, serves HTTP on port **80** |
| **Your PC** | Arduino IDE: compile, flash, Serial Monitor; optional **ngrok** agent for remote access |
| **Phone / PC browser** | Opens `http://<camera-ip>/` on the **same Wi‑Fi** (LAN), or ngrok **HTTPS** URL from outside |

**Important:** ESP32 Wi‑Fi is **2.4 GHz only**. The AP must expose a **2.4 GHz** SSID (or dual-band with 24 enabled).

---

## 2. Hardware checklist

- M5 **TimerCAM** (e.g. TimerCamera-F fisheye ~120° DFOV, or TimerCamera-X ~66.5° — see [M5 TimerCAM-F docs](https://docs.m5stack.com/en/unit/timercam_f) and [product selector](https://docs.m5stack.com/en/products_selector/timer_cam_compare?select=U082-F)).
- **USB data cable** and, if needed, **FTDI VCP drivers** (see same docs).
- **Router Wi‑Fi** you can join from the PC and the camera.

Optional: **second TimerCAM** + timelapse sketch for scheduled uploads only (not covered step-by-step here).

---

## 3. Install Arduino tooling

**Shortest path:** **[`INSTALL_SIMPLE.md`](INSTALL_SIMPLE.md)** (checklist only).  
**With explanations:** **[`INSTALL.md`](INSTALL.md)** — IDE, Espressif core, and copying the **bundled** libraries from [`../arduino_libraries/`](../arduino_libraries/) (pre-patched Timer-CAM + ArduinoHttpClient). You normally **do not** install Timer-CAM from Library Manager when using this repo.

### 3.1 Arduino IDE 2.x

Install from [arduino.cc](https://www.arduino.cc/en/software).

### 3.2 Espressif ESP32 core (required for `esp_camera.h`)

1. **File → Preferences → Additional boards manager URLs**  
   Add: `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
2. **Tools → Board → Boards Manager** → search **esp32** → install **esp32 by Espressif Systems** (pin a version for a class, e.g. 3.3.x or 2.0.x).
3. **Tools → Board** → **ESP32 Arduino** → **M5Stack Timer CAM** / **M5Stack-Timer-CAM** (exact label depends on core version).

### 3.3 Bundled Timer-CAM + ArduinoHttpClient

This project ships copies under [`../arduino_libraries/Timer-CAM`](../arduino_libraries/Timer-CAM) and [`../arduino_libraries/ArduinoHttpClient`](../arduino_libraries/ArduinoHttpClient). Install them with **[`../scripts/Install-ArduinoLibraries.ps1`](../scripts/Install-ArduinoLibraries.ps1)** (Windows) or by manually copying those folders into `Documents/Arduino/libraries/`. See **[`INSTALL.md`](INSTALL.md)** step 3.

Patch history for the vendored Timer-CAM is in [`../arduino_libraries/Timer-CAM/VENDOR_VERSION.txt`](../arduino_libraries/Timer-CAM/VENDOR_VERSION.txt).

### 3.4 If you use Library Manager instead (not recommended here)

Without the bundled copy, you may need manual edits when Arduino-ESP32 updates — see **[`LESSON_TimerCAM_Arduino_setup.md`](LESSON_TimerCAM_Arduino_setup.md)** (Power_Class / RTC8563 / `esp_camera.h`).

---

## 4. Prepare the sketch

1. In Arduino IDE, open the **folder**  
   [`m5cam_stream_server`](../m5cam_stream_server) (under the repo root; the folder name must match `m5cam_stream_server.ino`).

2. Edit **Wi‑Fi** and **web login** near the top:

```cpp
#define WIFI_SSID "your_2.4ghz_ssid"
#define WIFI_PASSWORD "your_wifi_password"

#define WWW_USER "your_admin_user"
#define WWW_PASS "your_strong_password"
```

Use a **strong** `WWW_PASS` before exposing the camera (LAN or ngrok).

3. Optional: **resolution** — in `initializeCamera()`, change `FRAMESIZE_VGA` to another `FRAMESIZE_*` (see comments in code / OV3660 docs). Higher sizes need more RAM and bandwidth.

4. **Tools → Port** → select the TimerCAM COM port. **Upload** the sketch.

---

## 5. First boot — Serial Monitor

1. **Tools → Serial Monitor** → **115200** baud.
2. Press **RESET** on the board if needed.

You should see roughly:

- Wi‑Fi connect messages and **`IP address: 192.168.x.x`**
- Camera ready and **test JPEG size** (thousands of bytes for VGA is normal)
- **HTTP … Basic Auth** reminder

**Garbled Serial:** set baud to **115200**; in code, `Serial.setDebugOutput(false)` and `esp_log_level_set("*", ESP_LOG_WARN)` are already used. **Tools → Core debug level** → **None** or **Warning**.

**Status LED:** after `TimerCAM.begin()`, the sketch turns on the **GPIO2** LED (PWM); dim LED may mean **camera init failed**.

---

## 6. Test on the LAN (no ngrok)

1. From a **PC or phone on the same Wi‑Fi**, open:

   `http://192.168.x.x/`

   (use the IP from Serial). Use **`http`**, not `https`.

2. **Basic Auth:** browser may show a login dialog. If **no dialog** (common on mobile), use:

   `http://USER:PASSWORD@192.168.x.x/`

3. **Routes:**
   - **`/`** — live preview by **refreshing `/jpg` in JavaScript** (keeps the server responsive).
   - **`/jpg`** — single JPEG.
   - **`/live`** — full **MJPEG** (`/stream`). **Only one long request at a time** with this WebServer; close `/live` to use other pages normally.

4. Optional: **`ping 192.168.x.x`** from the PC to confirm connectivity.

---

## 7. Security (short)

| Topic | Note |
|--------|------|
| **LAN** | Anyone who can reach the IP must pass **Basic Auth**. HTTP on LAN is **not encrypted**; use a strong password. |
| **Router** | Guest Wi‑Fi with **client isolation** may block access to the camera’s subnet — useful if you don’t want guests to reach LAN devices. |
| **Internet** | Without **port forwarding**, the camera’s **private IP** is not reached from the open internet. **ngrok** (next section) creates a temporary public URL. |

---

## 8. Remote access with ngrok

The **ngrok agent does not run on the ESP32**. It runs on a **PC (or Pi) on the same LAN** as the camera.

1. Install [ngrok](https://ngrok.com/download) and complete their **authtoken** setup (see [ngrok getting started](https://ngrok.com/docs/getting-started/)).
2. Give the TimerCAM a **DHCP reservation** so its IP is stable.
3. On that PC, run (replace with your camera IP):

   ```text
   ngrok http http://192.168.x.x:80
   ```

   If the UI shows **`undefined://undefined`** or **502**, you forgot the upstream URL — always include **`http://` and the camera IP and port**.

4. Open the **`https://…ngrok…`** URL from **cellular** or another network.

5. **Auth on mobile:** use  
   `https://USER:PASSWORD@your-ngrok-host/`  
   if the browser does not prompt (ngrok + Basic Auth quirk).

**While remote viewing works:** the **PC running ngrok** and the **camera** must stay **on** and on the **home network** (unless you use VPN into home, etc.).

---

## 9. Troubleshooting

| Symptom | Things to check |
|---------|------------------|
| **`M5TimerCAM.h` not found** | Install **Timer-CAM** library (§3.3). |
| **`esp_camera.h` not found** | Use **Espressif** ESP32 core + **M5Stack Timer CAM** board (§3.2). See [`LESSON_TimerCAM_Arduino_setup.md`](LESSON_TimerCAM_Arduino_setup.md) §3. |
| **Power_Class / RTC8563 errors** | Apply patches in [`LESSON_TimerCAM_Arduino_setup.md`](LESSON_TimerCAM_Arduino_setup.md) §2–4. |
| **Wi‑Fi fails** | 2.4 GHz SSID/password; signal; power. |
| **Compile OK, no `.elf` / path on `K:`** | Copy sketch to `Documents/Arduino/…` or clear Arduino build cache. |
| **401, no password box** | Use `https://user:pass@host/` (§8). |
| **Stream locks everything** | Use **`/`** for normal use; **`/live`** only when you accept single-connection behavior. |
| **Upload / DFU errors** | Correct **board** and **COM** port; **UART** upload, not DFU; **BOOT** if required. |

---

## 10. Repository map

| Path | Purpose |
|------|---------|
| [`INSTALL_SIMPLE.md`](INSTALL_SIMPLE.md) | Minimal step list (no extra detail) |
| [`INSTALL.md`](INSTALL.md) | Numbered setup + detail: IDE, Espressif core, libraries, sketch |
| [`../arduino_libraries/Timer-CAM`](../arduino_libraries/Timer-CAM) | Patched Timer-CAM (see `VENDOR_VERSION.txt`) |
| [`../arduino_libraries/ArduinoHttpClient`](../arduino_libraries/ArduinoHttpClient) | Dependency of Timer-CAM |
| [`../m5cam_stream_server/m5cam_stream_server.ino`](../m5cam_stream_server/m5cam_stream_server.ino) | Streaming HTTP server + Basic Auth |
| [`../old/m5cam_timelapse_v9_end_final.ino`](../old/m5cam_timelapse_v9_end_final.ino) | Separate timelapse → Google (other hardware) |
| [`LESSON_TimerCAM_Arduino_setup.md`](LESSON_TimerCAM_Arduino_setup.md) | Classroom notes: library / core pitfalls |
| [`TEACHER_lesson_smoothing_TimerCAM.md`](TEACHER_lesson_smoothing_TimerCAM.md) | Teacher prep: lab pitfalls and scope |

---

## 11. Done criteria

- [ ] Serial shows **IP** and **test JPEG** OK.  
- [ ] **`http://<ip>/`** works on LAN with **WWW_USER / WWW_PASS**.  
- [ ] **`/live`** works when you want MJPEG and accept single-client limits.  
- [ ] (Optional) **ngrok** URL works from outside with **`https://user:pass@…`**.  
- [ ] Passwords changed from defaults; sketch not committed to public repos with secrets.

You can extend later (mDNS hostname, HTTPS reverse proxy on a Pi, etc.); this guide matches the current repository behavior.
