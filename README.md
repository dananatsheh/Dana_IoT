# Dana IoT Projects

A collection of ESP32-based IoT and embedded systems projects developed during field training, built using **PlatformIO** and **ESP-IDF**. Each project includes full documentation covering hardware architecture, software design, debugging methodology, and verification results.

## Repository Structure

```
Dana_IoT/
│
├── esp_project/           # ESP-IDF environment setup & GPIO blink test
├── project_1/             # Project 1 — Smart Sensor & Lighting Control
├── project_2/             # Project 2 — Motor Protection System
├── project_3/             # Project 3 — Motor Driver & Modular Code (OOP)
├── project_4/             # Project 4 — Environmental Monitor & Data Logger
├── project_5/             # Project 5 — Two-Way Wireless Interlock System using ESPNOW
├── project_6/             # Project 6 — Two-Way Wireless Interlock System using MQTT
├── project_7/             # Project 7 — Local MQTT Telemetry & Bidirectional Motor Control
├── project_8/             # Project 8 — Node-RED Visual Dashboard for a Local ESP32 MQTT System
├── project_9/             # Project 9 — Firebase Realtime Database Cloud Logging for a Local ESP32 MQTT System
├── project_10/            # Project 10 — ESP-IDF GPIO, ADC & PWM Fundamentals
├── project_11/            # Project 11 — ESP-IDF FreeRTOS Task Fundamentals
├── .gitignore
└── README.md
```

Each project folder contains its own source code and a PDF documentation report describing the build in detail.

---

## Projects

### ESP-IDF Environment Setup
Initial configuration of a VS Code development environment for embedded systems using the official ESP-IDF extension, including toolchain installation, USB driver troubleshooting, and a GPIO blink/serial-monitor validation test.

- **Framework:** ESP-IDF (VS Code)
- 📄 Documentation: [`esp_project/ESP32_Task_Report.pdf`](./esp_project/ESP32_Task_Report.pdf)

---

### Project 1 — Smart Sensor & Lighting Control
An interlocking control system where an IR proximity sensor gates LED activation, with brightness dynamically controlled via PWM from a potentiometer. Includes live telemetry over UART.

- IR sensor–gated LED activation (interlock logic)
- Potentiometer-driven PWM brightness control (0–100%)
- Real-time UART diagnostics at 115200 baud

**Framework:** PlatformIO / ESP-IDF · **Board:** ESP32
📄 Documentation: [`project_1/Project1.pdf`](./project_1/Project1.pdf)

---

### Project 2 — Motor Protection System
A finite-state-machine-driven motor protection system using a relay, current sensor, and Emergency Stop button, with self-calibrating overcurrent detection.

- Boot-time current sensor self-calibration
- Overcurrent protection with latching fault state
- Debounced Emergency Stop button
- Four-state status LED indication (Running / Stopped / Low Current / Fault)

**Framework:** PlatformIO · **Board:** ESP32 DevKit v1
📄 Documentation: [`project_2/Project_2_Documentation.pdf`](./project_2/Project_2_Documentation.pdf)

---

### Project 3 — Motor Driver & Modular Code (OOP)
An upgrade of Project 2's protection system to a modular, object-oriented firmware built around an L298N dual H-bridge driver, adding speed and direction control.

- Refactored into config, C-style sensor driver, and C++ motor driver class
- PWM speed control with direction (forward/reverse) and active braking
- Interrupt-driven Emergency Stop
- Two-stage current filtering (oversampling + EMA)

**Framework:** PlatformIO · **Board:** ESP32-S3 DevKit v1
📄 Documentation: [`project_3/project_3_documentation.pdf`](./Project_3/project_3_documentation.pdf)

---

### Project 4 — Environmental Monitor & Data Logger
A self-contained environmental monitoring station serving a live, auto-refreshing web dashboard directly from the ESP32, with CSV data logging and power-loss-resilient state persistence.

- ESP32 + BME280 (temperature, humidity, pressure via I2C)
- Local HTTP dashboard with instrument-panel gauges (no app/cloud required)
- CSV logging to LittleFS with automatic size-based rotation
- Last-known-reading persistence via NVS across reboots
- WiFi station/AP fallback and NTP time sync

**Framework:** PlatformIO · **Board:** ESP32 DevKit
📄 Documentation: [`project_4/Project_4_Documentation.pdf`](./Project_4/Project_4_Documentation.pdf)

---

### Project 5 — Two-Way Wireless Interlock System using ESPNOW
A dual-ESP32 safety interlock connected peer-to-peer over ESP-NOW (no router or cloud), where each board's IR sensor gates its partner's motor, with a fail-safe link timeout.

- Bidirectional ESP-NOW communication between two ESP32 boards
- Cross-wired safety interlock (remote sensor gates local motor)
- 2-second fail-safe link timeout — silence forces motor stop
- Debounced IR sensing and full status LED set per board

**Framework:** PlatformIO · **Board:** Dual ESP32 DevKit
📄 Documentation: [`project_5/Project_5_Documentation.pdf`](./project_5/Project_5_Documentation.pdf)

---

### Project 6 — Two-Way Wireless Interlock System using MQTT
A dual-ESP32 safety interlock connected peer-to-peer over ESP-NOW (no router or cloud), where each board's IR sensor gates its partner's motor, with a fail-safe link timeout.

- Bidirectional MQTT communication between two ESP32 boards
- Cross-wired safety interlock (remote sensor gates local motor)
- 2-second fail-safe link timeout — silence forces motor stop
- Debounced IR sensing and full status LED set per board

**Framework:** PlatformIO · **Board:** Dual ESP32 DevKit
📄 Documentation: [`project_6/Project_6_Documentation.pdf`](./project_6/Project_6_Documentation.pdf)

---

### Project 7 — Local MQTT Telemetry & Bidirectional Motor Control
A single-ESP32 build that reads live BME280 environmental data over I2C and exposes full bidirectional control of a DC motor, all mediated through a Mosquitto MQTT broker self-hosted on the developer's own PC rather than a public cloud broker.

- BME280 sensor telemetry (temperature, humidity, pressure) published continuously over MQTT
- Full bidirectional motor control (state, direction, speed) via three command topics, with actuator-boundary speed clamping
- Live-state status publishing — status topics always reflect the motor's true running state, never a cached command
- Structured, hierarchical topic namespace (`sensor/`, `motor/status/`, `motor/command/`)
- Local disconnect fail-safe (motor stops on broker disconnect) with immediate status republish on reconnect
- Non-blocking main loop with independently timer-gated WiFi/broker reconnection

**Framework:** PlatformIO · **Board:** ESP32 DevKit · **Broker:** Self-hosted Mosquitto 2.1.2
📄 Documentation: [`project_7/Project_7_Documentation.pdf`](./project_7/Project_7_Documentation.pdf)

---

### Project 8 — Node-RED Visual Dashboard for a Local ESP32 MQTT System
A presentation and control layer built on top of Project 7's ESP32/Mosquitto system: a Node-RED dashboard that replaces manual MQTT Explorer/CLI inspection with live gauges, history charts, full bidirectional motor control, and a dedicated emergency stop — with no change to the underlying ESP32 wiring, sensor, or motor driver.

- Live gauges and 3-minute history charts for temperature, humidity, and pressure
- Full motor control (power, direction, speed slider clamped 0–100) plus an always-visible, dedicated Emergency Stop
- Retained Last Will and Testament on `device/esp32/status` for immediate detection of an ungraceful ESP32 disconnect
- Retained telemetry/status publishes so a dashboard opened after the fact shows real current values instantly
- Dual-signal availability indicator (broker LWT + client-side watchdog) that degrades gracefully: ONLINE → STALE → OFFLINE
- Speed clamping mirrored independently on both the Node-RED and ESP32 firmware sides (defense in depth)

**Framework:** PlatformIO (ESP32 firmware) + Node-RED · **Board:** ESP32 DevKit · **Broker:** Self-hosted Mosquitto 2.1.2
📄 Documentation: [`project_8/Project_8_Documentation.pdf`](./Project_8/Project_8_Documentation.pdf)

---

### Project 9 — Firebase Realtime Database Cloud Logging for a Local ESP32 MQTT System
An additive cloud-logging branch built on top of Project 8's Node-RED dashboard, mirroring live sensor readings and motor state into a Firebase Realtime Database without touching the existing ESP32 wiring, MQTT subscriptions, or dashboard flow. No new hardware, sensors, or wiring — the entire branch taps off nodes that already exist in the Project 8 flow.

- Current-state writer (`Set`) that fully overwrites a single `/current` node on every tracked value change, always reflecting "right now"
- History writer (`Push`) that snapshots the same state every 15 seconds into a uniquely-keyed, append-only `/history` log
- Firebase writers tap validated, already-converted sensor values and the existing dual-signal (LWT + watchdog) connection status — no duplicated MQTT subscriptions or validation logic
- Firebase nodes wired as a pure branch off existing outputs, never in series, so dashboard responsiveness is unaffected by Firebase write latency
- Locked-mode Realtime Database rules (Admin SDK bypasses rules entirely, so locking down client-side access costs nothing functionally)
- Debugging case study: a silent data-loss bug caused by Node-RED's per-node context scope, found and fixed by switching to flow-scoped context

**Framework:** PlatformIO (ESP32 firmware) + Node-RED + Firebase Realtime Database · **Board:** ESP32 DevKit · **Broker:** Self-hosted Mosquitto 2.1.2
📄 Documentation: [`project_9/Project_9_Documentation.pdf`](./Project_9/Project_9_Documentation.pdf)

---

### Project 10 — ESP-IDF GPIO, ADC & PWM Fundamentals
A back-to-basics build using pure ESP-IDF — no Arduino compatibility layer, no FreeRTOS tasks beyond the implicit one `app_main` runs on — covering the three foundational peripheral APIs: digital GPIO, ADC, and LEDC PWM. A potentiometer drives PWM LED brightness via `adc_oneshot`; an IR obstacle sensor drives a second LED fully on/off via polled `gpio_get_level`.

- Potentiometer-driven PWM brightness control via `adc_oneshot` + `ledc`, with linear 12-bit-to-13-bit duty scaling
- IR sensor–gated digital LED switching via plain polled GPIO input, no interrupts
- 16-sample **median filter** on the ADC read, chosen over a simple oversampled mean specifically for robustness against intermittent single-sample outlier spikes

**Framework:** ESP-IDF (VS Code) / PlatformIO · **Board:** ESP32 DevKit
📄 Documentation: [`project_10/Project_10_Documentation.pdf`](./Project_10/Project_10_Documentation.pdf)

---

### Project 11 — ESP-IDF FreeRTOS Task Fundamentals
Converting a single `while(1)` polling loop into two independent, scheduler-driven FreeRTOS tasks — `SensorTask` and `OutputTask` — created with `xTaskCreate()`, with `ESP_LOGI` diagnostics replacing all `printf` calls. The two tasks deliberately share data through plain `volatile` globals (no queue or mutex), surfacing a real cross-variable data-sharing hazard ahead of a future queue-based fix.

- `SensorTask` (priority 3, 200 ms): ADC read of the potentiometer + digital read of the IR sensor
- `OutputTask` (priority 4, 100 ms, higher priority so output freshness wins): LEDC PWM duty update + IR-triggered LED
- Deliberately unsynchronized `volatile` global hand-off (`g_pot_raw`, `g_pwm_duty`, `g_ir_state`) used to demonstrate a real tearing/staleness hazard, not fix it
- `vTaskDelayUntil`-anchored periods for jitter-free task timing
- Per-task `ESP_LOGI` tags for interleaved-log readability
- Debugging journal covering the dual-core (SMP) implications of the naive shared-state design

**Framework:** ESP-IDF (PlatformIO) · **Board:** ESP32 DevKit
📄 Documentation: [`Project_11/Project_11_Documentation.pdf`](./Project_11/Project_11_Documentation.pdf)

---

## Author

**Dana Natsheh**
Cyber Robot — IoT Field Training
