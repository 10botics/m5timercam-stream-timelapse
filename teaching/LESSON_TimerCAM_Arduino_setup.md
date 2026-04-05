# Lesson note: M5 TimerCAM + Arduino IDE (library pitfalls)

**This course repo** includes a **pre-patched** Timer-CAM under [`../arduino_libraries/Timer-CAM`](../arduino_libraries/Timer-CAM) and **[`INSTALL.md`](INSTALL.md)** — students should install that copy first and only read this document when something breaks or when troubleshooting a **non-bundled** Library Manager install.

Use this when teaching the **streaming camera step 1** sketch (`m5cam_stream_server`) or any TimerCAM project. **Standardize on the Espressif ESP32 board package** and board **“M5Stack Timer CAM”** — not the separate **“esp32 by Arduino”** package, or students often hit missing `esp_camera.h` and other SDK paths.

## Learning goals

- Install the correct **Timer-CAM** library and ESP32 board support.
- Recognize **“works on my machine”** failures caused by **core vs library version skew**.
- Apply a **two-line-class** fix when upstream libraries lag the core.

## Typical student errors

### 1) `M5TimerCAM.h: No such file or directory`

**Cause:** Timer-CAM library not installed.

**Fix:** Arduino IDE → *Sketch → Include Library → Manage Libraries…* → search **`Timer-CAM`** (by M5Stack) → Install. Dependency **`ArduinoHttpClient`** may be required.

**Alternative:** ZIP from https://github.com/m5stack/TimerCam-arduino → *Add .ZIP Library…*

---

### 2) Build fails inside `Timer-CAM` → `Power_Class.*`

**Symptoms (examples):**

- `'ADC1_GPIO38_CHANNEL' was not declared in this scope`
- `'LEDC_HIGH_SPEED_MODE' was not declared in this scope` (compiler may suggest `LEDC_LOW_SPEED_MODE`)

**Cause:** Older **Timer-CAM** copies used symbols that **newer Arduino-ESP32 / ESP-IDF** headers no longer define. The **Library Manager** version may be older than M5’s current GitHub `master`.

**Concept for students:** Arduino boards are “three moving parts” — **your sketch**, **vendor libraries** (Timer-CAM), and **the ESP32 core** (Espressif/Arduino). Any pair can be mismatched.

**Fix applied in this project’s teaching run** (edit the **installed** library under `Documents/Arduino/libraries/Timer-CAM/`, *or* maintain a fork / vendor copy in the course repo):

| File | Change |
|------|--------|
| `src/utility/Power_Class.h` | Replace `ADC1_GPIO38_CHANNEL` with **`ADC1_CHANNEL_2`** (GPIO38 on classic ESP32 = ADC1 channel 2). M5’s current upstream uses this name. |
| `src/utility/Power_Class.cpp` | Replace every **`LEDC_HIGH_SPEED_MODE`** with **`LEDC_LOW_SPEED_MODE`**. |

**Teaching tip:** After patching, recompile. If *new* ADC/legacy API errors appear, the course may need to standardize on a specific **ESP32 core version** or document migration to newer ADC APIs — record those errors for the next revision.

---

### 3) `esp_camera.h: No such file or directory` (from `Camera_Class.h`)

**Symptoms:** Compile stops in `Timer-CAM/.../Camera_Class.h` with `#include "esp_camera.h"`.

**Cause:** **`esp_camera.h` is not a separate Library Manager library** — it ships inside the **Espressif ESP32 Arduino core** (SDK / `esp32-camera`). If **Tools → Board** points at the **“esp32 by Arduino”** core only, or a board variant that doesn’t pull camera headers into the build, the include fails.

**Concept for students:** **Timer-CAM** is a *library*; **esp_camera** is part of the **toolchain** tied to the **board package**.

**Fix (recommended for courses):**

1. **Preferences → Additional boards manager URLs:** add  
   `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
2. **Boards Manager:** install **esp32 by Espressif Systems** (pick a **2.0.x** LTS for fewer surprises, or **3.x** with a pinned version for the whole class).
3. **Tools → Board:** under **ESP32 Arduino**, select **M5Stack Timer CAM** (name may vary slightly, e.g. `M5Stack-Timer-CAM`).
4. Recompile. Build log paths should show `packages/esp32/hardware/esp32/...` (Espressif), not only `packages/arduino/hardware/esp32/...`.

**Sanity check:** If board menu is confusing, temporarily select **ESP32 Dev Module** under **Espressif** — if `esp_camera.h` then appears, the issue was **wrong board package / board**, not the sketch.

---

### 4) `std::int8_t` / `std::uint8_t` does not name a type (`RTC8563_Class.h`)

**Symptoms:** Huge error list starting with `'int8_t' in namespace 'std' does not name a type` inside `Timer-CAM/src/utility/RTC8563_Class.h`.

**Cause:** **Arduino ESP32 3.x** (e.g. 3.3.7 with GCC 14) does **not** implicitly expose `std::int8_t` etc. The fixed-width integer typedefs in namespace `std` require **`#include <cstdint>`** (and **`#include <cstddef>`** if you use `std::size_t`). Older cores were looser about what prior headers pulled in.

**Fix:** At the top of `Documents/Arduino/libraries/Timer-CAM/src/utility/RTC8563_Class.h` (after the include guard), add:

```cpp
#include <cstddef>
#include <cstdint>
```

Then rebuild. Prefer reporting this upstream to [TimerCam-arduino](https://github.com/m5stack/TimerCam-arduino) so Library Manager catches up.

## Course checklist (student handout)

1. Install **ESP32** board support: **Espressif** package + URL above; board **M5Stack Timer CAM** (avoid relying only on **“esp32 by Arduino”** for camera lessons).
2. Install **Timer-CAM** + **ArduinoHttpClient** from this repo’s [`../arduino_libraries/`](../arduino_libraries/) per **[`INSTALL.md`](INSTALL.md)** (preferred), *or* Library Manager + manual patches in §2–4 if needed.
3. Open **`m5cam_stream_server`** folder in Arduino IDE (folder name = sketch name; under repo root).
4. Select the correct **Board** and **Port**; upload.
5. Serial Monitor **115200**: confirm **Wi-Fi IP** and **test JPEG size**.

## Reference

- M5 TimerCAM repo: https://github.com/m5stack/TimerCam-arduino  
- M5 docs: https://docs.m5stack.com/en/arduino/m5timer_cam/program  

---

*Last updated: Power_Class compatibility (Arduino-espressif skew) + **use Espressif core** so `esp_camera.h` resolves; Arduino-only `packages/arduino/hardware/esp32` is a common classroom trap.*
