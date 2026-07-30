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
├── project_5/             # Project 5 — Two-Way Wireless Interlock System
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
📄 Documentation: [`project_3/project_3_documentation.pdf`](./project_3/project_3_documentation.pdf)

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

### Project 5 — Two-Way Wireless Interlock System
A dual-ESP32 safety interlock connected peer-to-peer over ESP-NOW (no router or cloud), where each board's IR sensor gates its partner's motor, with a fail-safe link timeout.

- Bidirectional ESP-NOW communication between two ESP32 boards
- Cross-wired safety interlock (remote sensor gates local motor)
- 2-second fail-safe link timeout — silence forces motor stop
- Debounced IR sensing and full status LED set per board

**Framework:** PlatformIO · **Board:** Dual ESP32 DevKit
📄 Documentation: [`project_5/Project_5_Documentation.pdf`](./project_5/Project_5_Documentation.pdf)

---

## Author

**Dana Natsheh**
Cyber Robot — IoT Field Training
