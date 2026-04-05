# M5 TimerCAM — streaming + timelapse materials

Arduino sketches and documentation for the **M5Stack TimerCAM** (ESP32 + OV3660): **Wi‑Fi HTTP snapshot/MJPEG** with **Basic Auth**, plus optional **timelapse** firmware on a second unit.

## Teaching & setup docs

All lesson and install write-ups live in **[`teaching/`](teaching/)**. Start with [`teaching/INSTALL.md`](teaching/INSTALL.md).

| Item | Description |
|------|-------------|
| [`teaching/README.md`](teaching/README.md) | Index of teaching documents |
| [`teaching/INSTALL.md`](teaching/INSTALL.md) | **Start here** — numbered setup (IDE, Espressif core, libraries, sketch defines, upload) |
| [`teaching/GUIDE_streaming_TimerCAM_end_to_end.md`](teaching/GUIDE_streaming_TimerCAM_end_to_end.md) | Full project guide: LAN test, security, **ngrok**, troubleshooting, hardware notes |
| [`teaching/LESSON_TimerCAM_Arduino_setup.md`](teaching/LESSON_TimerCAM_Arduino_setup.md) | Teaching notes: compile errors, core vs library skew, manual patches if not using the bundled library |
| [`teaching/TEACHER_lesson_smoothing_TimerCAM.md`](teaching/TEACHER_lesson_smoothing_TimerCAM.md) | **Teacher prep:** install hiccups, Wi‑Fi/isolation, lesson scope, what is hard to fix in one period |

## What is included

| Item | Description |
|------|-------------|
| [`m5cam_stream_server/`](m5cam_stream_server/) | Main sketch: `/` (JPEG polling), `/live` (MJPEG), `/jpg`, `/stream`; Wi‑Fi + login at top of `.ino` |
| [`arduino_libraries/`](arduino_libraries/) | **Pre-patched** [Timer-CAM](https://github.com/m5stack/TimerCam-arduino) + **ArduinoHttpClient** (offline-friendly; see `VENDOR_VERSION.txt` in each folder) |
| [`scripts/Install-ArduinoLibraries.ps1`](scripts/Install-ArduinoLibraries.ps1) | Windows: copies bundled libraries into `Documents\Arduino\libraries\` |
| [`old/m5cam_timelapse_v9_end_final.ino`](old/m5cam_timelapse_v9_end_final.ino) | Separate timelapse sketch (another board); not merged with the streaming server |

## Quick start

1. Install **Arduino IDE 2** and the **Espressif ESP32** board package; select board **M5Stack Timer CAM** (see [`teaching/INSTALL.md`](teaching/INSTALL.md)).
2. Run [`scripts/Install-ArduinoLibraries.ps1`](scripts/Install-ArduinoLibraries.ps1) from the repo root, or copy [`arduino_libraries/Timer-CAM`](arduino_libraries/Timer-CAM) and [`arduino_libraries/ArduinoHttpClient`](arduino_libraries/ArduinoHttpClient) into your Arduino `libraries` folder.
3. Open [`m5cam_stream_server/m5cam_stream_server.ino`](m5cam_stream_server/m5cam_stream_server.ino), set **`WIFI_SSID`**, **`WIFI_PASSWORD`**, **`WWW_USER`**, **`WWW_PASS`**, then upload.
4. Serial Monitor **115200** → note the camera IP → browser: `http://<ip>/` (same **2.4 GHz** Wi‑Fi as the ESP32).

For remote viewing over the internet, use **ngrok** (or similar) on a PC **on the same LAN** as the camera — details in [`teaching/GUIDE_streaming_TimerCAM_end_to_end.md`](teaching/GUIDE_streaming_TimerCAM_end_to_end.md).

## Hardware

- [M5 TimerCAM product line](https://docs.m5stack.com/en/arduino/m5timer_cam/program) — variants differ mainly by **lens / FOV** (e.g. wide fisheye vs narrower). ESP32 Wi‑Fi is **2.4 GHz only**.

## Third-party code

Bundled libraries retain their upstream **LICENSE** files. This repo’s sketches are provided as-is for teaching and projects.

## See also

- M5 TimerCAM Arduino library (upstream): [TimerCam-arduino](https://github.com/m5stack/TimerCam-arduino)
