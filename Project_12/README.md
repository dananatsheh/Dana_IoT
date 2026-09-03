# Task 4.2 — FreeRTOS Queues: Fixing the Race Condition

ESP32 / ESP-IDF firmware that replaces the unsynchronized global-variable
hand-off from Task 3.6 with a proper FreeRTOS `Queue`, closing the
cross-variable data race and removing polling from the consumer task.

## What it does

Two tasks read a potentiometer and an IR obstacle sensor, and drive a PWM
LED and a digital LED accordingly:

- **`SensorTask`** (priority 3) — reads the ADC + IR pin every 200 ms,
  packages both values into one `sensor_msg_t` struct, and offers it to a
  queue with a non-blocking `xQueueSend()`.
- **`OutputTask`** (priority 4) — blocks on `xQueueReceive()` with a 300 ms
  timeout (no polling loop of its own). On receipt it updates PWM duty and
  the IR LED; on timeout it re-applies the last known values and logs a
  warning.

```
SensorTask --xQueueSend(0)--> sensor_queue --xQueueReceive(300ms)--> OutputTask
```

## Why a queue instead of globals

Task 3.6 used three plain `volatile` globals to pass data between the two
tasks. Because `pot_raw` and `ir_state` were written as two separate
statements, `OutputTask` could observe a torn, inconsistent pair that never
existed as a single snapshot in `SensorTask`. A queue copies the whole
struct in one atomic operation, so that hazard is closed by construction —
not by convention.

## Hardware

Unchanged from Task 3.6:

| Pin | Peripheral | Role |
|---|---|---|
| GPIO2 | LEDC PWM (10-bit, 5 kHz) | Brightness LED |
| GPIO4 | Digital output | IR-state LED |
| GPIO5 | Digital input, pull-up | IR obstacle sensor (active-low) |
| GPIO34 | ADC1 channel 6 | Potentiometer wiper |

## Key build-time flags

| Macro | Purpose |
|---|---|
| `QUEUE_LENGTH` | Queue depth. Tested at `1` and `5`. |
| `SIMULATE_SLOW_CONSUMER` | Set to `1` to add a 400 ms delay in `OutputTask`, artificially slowing it below `SensorTask`'s 200 ms rate so overflow behavior can be observed and logged. Set to `0` for normal operation. |

## What testing showed

- **Depth 1, normal consumer** — no drops; consumer keeps up fine.
- **Depth 1, slow consumer** — `xQueueSend` **drops** (doesn't block or
  overwrite) whenever the queue is full; roughly every other sample is
  lost, including occasional IR-state changes.
- **Depth 5, slow consumer** — drops are delayed while the buffer fills,
  but once the producer/consumer rate mismatch is sustained (2:1 here),
  the queue saturates and settles into the same steady-state drop rate.
  The real difference is **staleness, not survival**: `OutputTask` ends up
  acting on older, backlogged samples instead of the newest one.

Full traces, analysis, and the answers to the assignment's report
questions are in `Task_4_2_Queues_Report.pdf` (source: `.tex`).

## Full documentation:
[`Project_11_Documentation.pdf`](./Project_12_Documentation.pdf)

## Serial monitor output

<img width="812" height="441" alt="Screenshot 2026-09-01 153431" src="https://github.com/user-attachments/assets/22f6b8aa-3e39-4970-9484-75b1211e7786" />

--

<img width="812" height="340" alt="Screenshot 2026-09-03 112134" src="https://github.com/user-attachments/assets/d52967fd-b70e-4211-946f-dc0f24479b45" />

--

<img width="812" height="290" alt="Screenshot 2026-09-03 112752" src="https://github.com/user-attachments/assets/2a327abb-1005-4008-ba92-94f988b329ea" />

--

<img width="812" height="326" alt="Screenshot 2026-09-03 112732" src="https://github.com/user-attachments/assets/1a803ea2-f6fb-4fe8-83cc-3dfa750dd2aa" />
