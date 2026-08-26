# Project 10 — Task 3.4: ESP-IDF GPIO, ADC & PWM Fundamentals

Pure ESP-IDF firmware for the ESP32 demonstrating three foundational peripheral
APIs — digital GPIO, ADC, and LEDC PWM — with **no Arduino compatibility layer
and no FreeRTOS tasks** beyond the single implicit task ESP-IDF always creates
for `app_main`.

See the full [ESP-IDF GPIO, ADC & PWM
Fundamentals](Project_10_Documentation.pdf) for schematics, firmware, and calibration notes.

## What it does

- **Potentiometer → PWM brightness.** A potentiometer wiper is read through
  the `adc_oneshot` driver and mapped to the duty cycle of a PWM LED via
  `ledc`.
- **IR sensor → digital LED switching.** An IR obstacle sensor's digital
  output is polled with `gpio_get_level` and mirrored onto a second LED
  (fully on/off, no PWM).

Both chains run inside a single `while(1)` polling loop, updated roughly
every 50 ms.

## Hardware

| Pin | Peripheral | Role |
|---|---|---|
| GPIO2 | LEDC PWM (timer 0, channel 0) | Brightness LED, driven by the potentiometer |
| GPIO4 | Digital output | Second LED, mirrors the IR sensor state |
| GPIO5 | Digital input | IR obstacle sensor output |
| GPIO34 | ADC1 channel 6 | Potentiometer wiper (input-only pin, no internal pull resistor) |

Power: every module's VCC/+ goes to the ESP32's `3V3` rail, every GND/− goes
to `GND`. Both LEDs sit in series with a 220–330 Ω current-limiting resistor.

See [`docs/wiring.md`](docs/wiring.md) *(add your own diagram/photos here)*
for the full breadboard layout.

## Requirements

- ESP-IDF **v5.0+** (required for the `adc_oneshot` API used here)
- An ESP32 dev board
- Either the [VS Code ESP-IDF extension](https://github.com/espressif/vscode-esp-idf-extension)
  **or** [PlatformIO](https://platformio.org/) with `framework = espidf`

## Project structure

```
project10/
├── CMakeLists.txt          # root project file (ESP-IDF native layout)
├── main/
│   ├── CMakeLists.txt
│   └── main.c               # all application logic
├── platformio.ini           # only needed if building via PlatformIO
└── README.md
```

## Building & flashing

### Option A — ESP-IDF (VS Code extension or CLI)

```bash
idf.py set-target esp32
idf.py build
idf.py -p <PORT> flash monitor
```

### Option B — PlatformIO

```ini
; platformio.ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = espidf
monitor_speed = 115200
```

```bash
pio run --target upload
pio device monitor
```

Note: `adc_oneshot` requires ESP-IDF v5.0+. If using PlatformIO, confirm the
bundled `framework-espidf` package version meets this before building.

## Code overview

- `gpio_setup()` — configures the digital output LED and digital input IR
  sensor pins.
- `adc_setup()` — creates the ADC1 oneshot unit and configures the
  potentiometer's channel (`ADC_ATTEN_DB_12` for full 0–3.3 V range).
- `pwm_setup()` — configures the LEDC timer (5 kHz, 13-bit resolution) and
  channel bound to the PWM LED pin.
- `app_main()` — the main polling loop: takes 16 ADC samples per iteration,
  **median-filters** them (not a simple average — see below), scales the
  result to a PWM duty cycle, and separately polls the IR sensor to drive the
  second LED.

  ## Schematic and Circuit
  <img width="2467" height="2108" alt="circuit_image (7)" src="https://github.com/user-attachments/assets/aee8278d-a4a8-4f69-8c3a-2743fa3a67de" />
  
  <img width="2296" height="2108" alt="IMG_20260818_103108 jpg" src="https://github.com/user-attachments/assets/3971014a-bae3-4eef-901a-ff29c359e27d" />

