# Teacher notes — smoothing the TimerCAM streaming lesson

This document complements [`INSTALL.md`](INSTALL.md) and [`GUIDE_streaming_TimerCAM_end_to_end.md`](GUIDE_streaming_TimerCAM_end_to_end.md). It focuses on **what goes wrong in a classroom** when students have **no prior TimerCAM experience**, and what is **hard to fix inside one lesson**.

Students follow **INSTALL** for the step-by-step checklist; use this handout for **prep, lab policy, and triage**.

---

## 1. Gaps or weak spots in the default install flow

| Issue | What happens | Mitigation |
|--------|----------------|----------|
| **Windows-centric script** | Step 3 recommends PowerShell; macOS/Linux students only have “manual copy.” | Publish **one** path per OS on the board, or demo manual copy for everyone once. |
| **PowerShell / managed PCs** | `Install-ArduinoLibraries.ps1` may be blocked or confusing. | Keep **Option B** (manual copy) ready; budget 5–10 minutes for first-time copies. |
| **“Four `#define`s” wording** | [`INSTALL.md`](INSTALL.md) says four lines; `WWW_USER` is often left as `admin`. | Tell the class: “Set Wi‑Fi password, web password, and SSID; user can stay `admin`.” |
| **Duplicate Timer-CAM library** | Student already installed **Timer-CAM from Library Manager**; bundled **patched** copy may not be the one used. | **Uninstall** Library Manager “Timer-CAM” before copying from `arduino_libraries/` (repo root), or only use the bundled copy. |
| **USB / COM not documented in INSTALL** | No COM port, or “board not found.” | Spare **data** USB cables; try another USB port; M5 docs mention **FTDI/VCP** on some PCs — have a driver link ready. |
| **Board menu oversimplified** | Student picks wrong ESP32 board or a different core. | **Lock** menu: everyone → **Espressif** package → **M5Stack Timer CAM** (exact label may vary). See [`LESSON_TimerCAM_Arduino_setup.md`](LESSON_TimerCAM_Arduino_setup.md) if `esp_camera.h` fails. |
| **ESP32 core version drift** | “e.g. 3.3.x” still allows 2.0 vs 3.3 split **across the room** → different compile errors. | Pick **one** core version for the cohort; write it on the board (e.g. “ESP32 3.3.7 only”). |
| **Power-on unfamiliarity** | TimerCAM-style units may need a deliberate **power-on** (hold power). Silent Serial looks “dead.” | Demo power-on once; mention **battery** charge if applicable. |

---

## 2. Wi‑Fi and network (largest non-code failure mode)

| Issue | What happens | Mitigation |
|--------|----------------|----------|
| **2.4 GHz only** | Student joins **5 GHz–only** or wrong SSID; ESP32 never connects. | Provide a **2.4 GHz** SSID + password on paper; verify with IT **before** class. |
| **Captive portal / enterprise** | School or hotel “click to accept” or **WPA2-Enterprise** — ESP32 cannot complete login. | Not fixable in-lesson without **a simple WPA2 PSK network** (e.g. dedicated AP or phone hotspot). |
| **Client isolation** | Guest Wi‑Fi **isolates** clients; phone/PC cannot open `http://<cam-ip>/` even if Serial shows an IP. | Test from laptop **same subnet** as camera; avoid “guest” SSIDs that isolate. See GUIDE §7. |
| **IT restrictions** | Firewall or policy blocks local HTTP; weird proxy. | Rare on lab LAN; if needed, use same SSID as teacher demo machine that **worked** in prep. |

---

## 3. After upload — behavior that looks like “bugs”

| Symptom | Cause | What to tell students |
|---------|--------|------------------------|
| **401, no password box** | Basic Auth + especially **mobile** or **ngrok** interstitial. | Use `http://user:pass@<ip>/` (LAN) or `https://user:pass@<ngrok-host>/` — see GUIDE §6 and §8. |
| **“Frozen” site while streaming** | ESP32 **synchronous** `WebServer`; **`/live`** (MJPEG) holds one long request. | Normal page: **`/`** (JPEG **polling**). Use **`/live`** only when you accept **single-connection** behavior; close that tab to unstick other routes. |
| **ngrok confused** | Second toolchain: account, authtoken, PC on LAN. | Treat **LAN success** as Lesson 1 milestone; ngrok as **optional** demo or Lesson 2. |

---

## 4. Environment and paths

| Issue | What happens | Mitigation |
|--------|----------------|----------|
| **Sketch on network drive** | Odd build paths, missing `.elf`, Arduino cache issues (seen in some lab setups). | Prefer sketch under **`Documents\Arduino\`** or **local disk**; avoid deep network mounts for the active sketch. |

---

## 5. Often **not fixable inside one lesson**

| Area | Why |
|------|-----|
| **School IT** | USB disabled, drivers blocked, no install rights. |
| **No suitable Wi‑Fi** | No 2.4 GHz PSK SSID; isolation; enterprise-only. |
| **Hardware** | Charge-only cable, dead USB, weak power / brownout, faulty board. |
| **Version chaos** | Everyone on a different ESP32 core or duplicate libraries. |
| **ngrok / tunnels** | Signup, email verify, school blocking tunnels — reserve separate time. |

---

## 6. Smooth-lesson checklist (teacher prep)

1. **Golden profile:** One Arduino IDE + **one pinned ESP32 core** version for all lab machines (or clear written requirement).
2. **Libraries:** Bundled [`../arduino_libraries/`](../arduino_libraries/) only; **no** conflicting Library Manager Timer-CAM.
3. **Network:** Confirmed **2.4 GHz WPA2** SSID/password; **no client isolation** between student devices and camera IPs.
4. **Hardware:** Spare **data** USB cables; one **known-good** TimerCAM for swap tests.
5. **Scope:** **Lesson 1** = install libraries → open `m5cam_stream_server` → set defines → upload → Serial **115200** → `http://<ip>/` with Basic Auth. Defer **ngrok** unless pre-staged.
6. **Printed triage:** Keep [`LESSON_TimerCAM_Arduino_setup.md`](LESSON_TimerCAM_Arduino_setup.md) §2–4 and GUIDE **§9 Troubleshooting** available on paper or projector for error text → action.

---

## 7. Document map

| File | Role |
|------|------|
| [`INSTALL.md`](INSTALL.md) | Student-facing numbered install |
| [`GUIDE_streaming_TimerCAM_end_to_end.md`](GUIDE_streaming_TimerCAM_end_to_end.md) | LAN, security, ngrok, troubleshooting table |
| [`LESSON_TimerCAM_Arduino_setup.md`](LESSON_TimerCAM_Arduino_setup.md) | Compile errors, core vs library skew, patches |
| **This file** | Classroom prep, network policy, scope, escalation |

---

*This note matches the streaming sketch in [`../m5cam_stream_server/`](../m5cam_stream_server/); the timelapse sketch in [`../old/`](../old/) is a separate unit and different lesson.*
