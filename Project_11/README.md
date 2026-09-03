# Project 11 — ESP-IDF FreeRTOS Task Fundamentals

Converting a single `while(1)` polling loop into two independent, scheduler-driven FreeRTOS tasks, with `ESP_LOGI` diagnostics replacing all `printf` calls.

## Overview

This project (Task 3.6) takes the previous single-loop firmware — which read a potentiometer and an IR obstacle sensor and drove two LEDs directly from `app_main` — and splits it into two independent tasks created with `xTaskCreate()`:

- **Task A — `SensorTask`** (priority 3, every 200 ms): owns the ADC read of the potentiometer and the digital read of the IR sensor.
- **Task B — `OutputTask`** (priority 4, every 100 ms): owns the LEDC PWM duty update and the digital IR-triggered LED, running at a strictly higher priority than `SensorTask`.

The two tasks share their readings through plain `volatile` global variables — **deliberately, without a queue or mutex** — so the resulting data-sharing hazard can be observed and reasoned about directly, ahead of a queue-based fix planned for a future task.

## System Diagram

```
Task A -- SensorTask (priority 3, every 200 ms)
(Potentiometer wiper) --adc_oneshot_read--> g_pot_raw
(IR sensor OUT)        --gpio_get_level-->   g_ir_state

Task B -- OutputTask (priority 4, every 100 ms)
g_pot_raw   --shift to 10-bit--> [LEDC PWM] --> LED1 brightness
g_ir_state  --gpio_set_level-->              LED2 on/off
```

- Task A and Task B run at different periods (200 ms / 100 ms) and different priorities, so `OutputTask` applies each freshly-read potentiometer value roughly twice before `SensorTask` refreshes it again.
- Data crosses from Task A to Task B purely through three volatile globals (`g_pot_raw`, `g_pwm_duty`, `g_ir_state`) — no queue, semaphore, or mutex protects this hand-off.
- `app_main` itself becomes a third, low-priority housekeeping loop: it creates both tasks once, then periodically logs runtime statistics and does nothing else.

## Hardware

| Pin | Peripheral | Role |
|---|---|---|
| GPIO2 | LEDC PWM channel 0, timer 0 (10-bit, 5 kHz) | Brightness LED, duty written by `OutputTask` |
| GPIO4 | Digital output | Second LED, mirrors the IR sensor state as written by `OutputTask` |
| GPIO5 | Digital input, internal pull-up enabled | IR obstacle sensor output, read by `SensorTask`; active-low (object present when the pin reads 0) |
| GPIO34 | ADC1 channel 6 | Potentiometer wiper; input-only pin, read by `SensorTask` |

Power wiring is unchanged from the previous task: every module's VCC/+ returns to the ESP32's 3V3 rail and every GND/− returns to GND, with both LEDs in series with a 220–330 Ω current-limiting resistor. The only functional hardware-side change is enabling the ESP32's internal pull-up on the IR sensor's input pin in software.

**Framework:** ESP-IDF v5.0+ (PlatformIO, `platform = espressif32`, `board = esp32dev`)
**Board:** ESP32 DevKit

## Key Design Points

- **`vTaskDelayUntil` over `vTaskDelay`** in both tasks — each task's period is measured from its own last wake time rather than from when it finished its work, keeping the 200 ms / 100 ms periods stable despite jitter in logging/driver call durations.
- **Priority reflects output freshness, not sensor importance** — `OutputTask` (priority 4) outranks `SensorTask` (priority 3) because a stale LED update matters more to observable behavior than a slightly delayed sensor sample.
- **ADC-to-PWM scaling:** `duty = raw >> 2` maps the 12-bit ADC range (0–4095) onto the 10-bit LEDC range (0–1023), since 4095/4 ≈ 1023 — negligible quantization error for a brightness control.
- **Per-task log tags** (`TAG_SENSOR`, `TAG_OUTPUT`, `TAG_MAIN`) make it possible to visually separate each task's timeline in the interleaved serial output.

## Debugging Journal — The Naive Global-Variable Hand-off

This project deliberately shares data between `SensorTask` and `OutputTask` through three plain `volatile` globals instead of a queue, in order to **surface — not fix — a real synchronization hazard.**

**What is actually safe:** Each global is a single machine word (`int`) or byte (`bool`) on the ESP32's 32-bit Xtensa core, and is marked `volatile`. A lone read or write will not be torn, and `volatile` prevents the compiler from caching a stale value across loop iterations in different execution contexts.

**What is not safe:** `g_pot_raw` and `g_ir_state` are written together, once per `SensorTask` iteration, but as two separate memory writes with no barrier or lock spanning both. `OutputTask` runs on its own independent 100 ms schedule and can be scheduled at any point relative to `SensorTask`'s writes — meaning it can legally observe a combination (e.g. a fresh `g_pot_raw` with a stale `g_ir_state`) that never existed as a single consistent snapshot. Separately, because the periods differ, `OutputTask` will also simply re-apply the same `g_pot_raw` value on some iterations — a staleness issue distinct from the tearing issue.

**Deliberately not fixed here:** No mutex or queue is introduced. A future task replaces this raw global hand-off with a proper `QueueHandle_t` carrying a `{raw, ir_state}` struct, making the hand-off atomic by construction. This project exists specifically to make the naive version's failure mode visible first.

**Why the dual-core architecture makes it worse:** ESP-IDF's FreeRTOS port is SMP-capable, and tasks created with `tskNO_AFFINITY` (the default here) may be scheduled onto either of the ESP32's two cores independently. `SensorTask` and `OutputTask` are therefore not guaranteed to run on the same core, and may genuinely execute simultaneously in real time rather than merely being time-sliced — which is exactly the class of hazard a queue or mutex is needed to close.

## Results & Verification

`OutputTask` was confirmed to apply PWM duty and IR-LED state independently of `SensorTask`'s own schedule: sweeping the potentiometer while alternately covering/uncovering the IR sensor produced brightness and digital-LED changes that tracked each input correctly, with the digital LED's response bounded by `SensorTask`'s 200 ms sampling period plus up to one `OutputTask` cycle.

## Known Limitations

- **Unsynchronized cross-variable state** — `g_pot_raw`/`g_ir_state` can be observed as an inconsistent pair; accepted as a known limitation for this task, not an oversight.
- **No core affinity pinning** — both tasks use `tskNO_AFFINITY`, so relative execution order across cores is not fully deterministic run to run.
- **Runtime statistics are conditionally compiled** — the `vTaskGetRunTimeStats` block only produces output when `CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS` is enabled in `sdkconfig`.

## Future Improvements

- Replace the raw globals with a `Queue` carrying `{raw, ir_state}` to close the cross-variable tearing hazard atomically.
- Pin core affinity explicitly once deterministic timing between the two tasks is required.
- Add a mutex around the runtime-stats buffer if a third task is added that might also read `stats_buf`.
- Move to interrupt-driven IR sensing once ISR and queue mechanics are introduced.

## Bill of Materials

| Item | Qty | Notes |
|---|---|---|
| ESP32 DevKit development board | 1 | Unchanged from previous task |
| LED | 2 | One PWM-driven (GPIO2), one digital on/off (GPIO4) |
| Current-limiting resistor | 2 | 220–330 Ω, one per LED |
| IR obstacle sensor module | 1 | 3-pin digital output (VCC, GND, OUT); internal pull-up enabled on ESP32 side |
| Potentiometer | 1 | Wiper on GPIO34 (ADC1 channel 6) |
| Breadboard | 1 | |
| Jumper wires | Assorted | |
| USB cable | 1 | Flashing and serial monitor |
| PC running VS Code + PlatformIO Core | 1 | ESP-IDF v5.x+ toolchain |

Full documentation: [`Project_11_Documentation.pdf`](./Project_11_Documentation.pdf)

## Schematic and Circuit
<img width="2467" height="2108" alt="circuit_image (7)" src="https://github.com/user-attachments/assets/099391ea-9a88-4c03-85c7-13458a38aced" />
<img width="2296" height="2108" alt="IMG_20260818_103108 jpg" src="https://github.com/user-attachments/assets/4a806b04-257d-40e0-ba0b-ed6abc661888" />
