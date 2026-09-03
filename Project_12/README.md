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

## Repo structure

```
main.c                          # Firmware source
Task_4_2_Queues_Report.tex      # Full LaTeX report (source)
Task_4_2_Queues_Report.pdf      # Compiled report (with placeholder figures)
figs/                           # Drop real photos/screenshots here (see below)
  schematic.png                 # Wiring schematic
  circuit.jpeg                  # Breadboard photo
  serial_depth1_normal.png      # Serial log: depth=1, normal consumer
  serial_depth1_drop.png        # Serial log: depth=1, slow consumer (drops)
  serial_depth5.png             # Serial log: depth=5, slow consumer (steady state)
  serial_depth5_fillup.png      # Serial log: depth=5, boot-time fill-up phase
```

## Build & flash

```bash
pio run -t upload
pio device monitor -b 115200
```

To reproduce the depth/consumer comparisons, edit `QUEUE_LENGTH` and
`SIMULATE_SLOW_CONSUMER` at the top of `main.c`, rebuild, and re-flash
between configurations.

## Report questions (answered in full in the PDF)

1. Did the queue solve the Task 3.6 race condition, and what changed under
   the hood?
2. What happens when the queue is full at depth 1 — block, drop, or
   overwrite — and why does that matter?
3. What is "stale data" in a queue context, and how does it differ from
   the global-variable staleness problem?
