# Task 2.7 — ESP32 + Mosquitto: BME280 Telemetry & Bidirectional Motor Control

Single ESP32, local Mosquitto broker, BME280 telemetry, full bidirectional
motor control via MQTT. Reused from the earlier Task 2.4/2.6 interlock
project only what was still relevant (the reconnect pattern, the
`MqttInterlockClient` name/shape) — dropped everything interlock-specific
(`LinkMonitor`, `IRSensor`, `MY_ID`/`PARTNER_ID`).

See the full [Local MQTT Telemetry &
Bidirectional Motor Control](Project_7_Documentation.pdf) for schematics, firmware, and calibration notes.


## Part 1 — Broker Setup

Mosquitto 2.1.2, run manually (not as a service):

```
mosquitto -v -c mosquitto.conf
```

`mosquitto.conf`:
- `listener 1883 0.0.0.0` — binds to all interfaces, not just loopback,
  so devices on the LAN can reach it (the default install only listens
  locally).
- `allow_anonymous false` + `password_file passwd.txt` — credentials
  required, file generated with `mosquitto_passwd`.

**Windows Firewall / network changes:**
- Added an inbound rule for TCP 1883, scoped to the **Private** profile.
- Set the active network profile to Private (Public blocks inbound
  connections regardless of firewall rules).
- This has to be redone per SSID — Windows tracks network category
  independently for every SSID it has seen, so switching WiFi networks
  can silently drop you back to Public.

**Known hardware limitation (documented, not a bug in this project):**
the WiFi extender (`CYBER_EXT`) blocks direct device-to-device LAN
traffic between clients connected to it, independent of firewall or
network profile — confirmed by testing with the firewall fully disabled.
Working topology: **PC on the main router, ESP32 on the extender.**

Verified end-to-end pre-MQTT with a raw `WiFiClient::connect()` TCP
test — matching logs on both sides (`TCP connect OK` / `New connection
from...`).

## Part 2 — Topic Structure

```
sensor/bme280/temperature      ESP32 → broker   (float, °C)
sensor/bme280/humidity         ESP32 → broker   (float, %RH)
sensor/bme280/pressure         ESP32 → broker   (float, hPa)

motor/status/state             ESP32 → broker   ("1" running / "0" stopped)
motor/status/direction         ESP32 → broker   ("forward" / "reverse")
motor/status/speed             ESP32 → broker   (int 0-100)

motor/command/state            broker → ESP32   ("1" run / "0" stop)
motor/command/direction        broker → ESP32   ("forward" / "reverse")
motor/command/speed            broker → ESP32   (int 0-100, clamped)
```

Payloads are raw values, not JSON — simple enough to type directly in
MQTT Explorer.

## Part 3 — Design Decisions

**Motor status reflects live state, not last command.** `publishMotorStatus()`
always reads `motor.isRunning() / isForward() / getSpeedPercent()` at
publish time, never a remembered "last command sent" value. This is
what makes the reconnect behavior below correct for free.

**Speed clamping** happens in `MotorController::setSpeedPercent()`:
values `< 0` clamp to 0, `> 100` clamp to 100, so the actuator boundary
is always safe regardless of what sends the command.

**Fail-safe on disconnect:** `main.cpp` checks `mqttClient.isConnected()`
every loop iteration and calls `motor.stop()` immediately and locally if
it's down — independent of MQTT, since publishing while disconnected
isn't possible anyway.

**Immediate publish on reconnect, no special bookkeeping:** because
status publishing always reads live state, the "publish current state
once immediately on reconnect" requirement falls out of one extra call.
`MqttInterlockClient` exposes `consumeReconnectFlag()` — a one-shot flag
set internally right after a successful broker connect and cleared the
moment `main.cpp` reads it. `main.cpp` polls it each loop and calls
`publishMotorStatus()` when it's true, on top of the normal interval-based
publish.

**No separate `LinkMonitor`.** Connection-up/down is just
`PubSubClient::connected()` — there's no partner-timeout concept left
to track.

## `MqttInterlockClient` — thin wrapper, name kept, role changed

Doesn't understand motor/BME280 meaning at all — only moves topic +
payload strings. `main.cpp` owns all interpretation.

- `publish(topic, payload)` — generic, `main.cpp` decides what/when.
- `subscribe(topic, slot)` — generic pub/sub, but each subscription is
  tagged with a `CommandSlot` (`STATE` / `DIRECTION` / `SPEED`) so the
  class can route an incoming message to the right inbox by matching
  topic strings. That's routing, not payload interpretation, so it
  doesn't break the thin-wrapper intent — this was the open nuance left
  unresolved before, and tagging each `subscribe()` call with a slot is
  the resolution.
- Three named inboxes, not a shared slot or a queue — there are exactly
  three fixed topics, each is a setpoint (newest overwrites older is
  correct, not lossy), and commands are human-triggered one at a time
  from MQTT Explorer. `getNewStateCommand(String&)` /
  `getNewDirectionCommand(String&)` / `getNewSpeedCommand(String&)` each
  auto-clear their "new" flag on read — deliberately different from
  `BME280Sensor::isDataValid()`'s pure-read pattern, because these are
  one-time commands to consume, not ongoing state to re-check.
- **Resolved open question from planning:** the constructor does *not*
  take `MotorController&`/`BME280Sensor&` references. The
  reconnect-publish need is met by the `consumeReconnectFlag()` poll
  instead — `main.cpp` already owns the motor/sensor objects and does
  the actual read-and-publish itself, keeping the MQTT class fully
  generic.

## Motor wiring

Simple two-direction-pin + PWM driver (e.g. L298N-style), pins in
`config.h`:

```
MOTOR_PIN_IN1 = 26   direction pin A
MOTOR_PIN_IN2 = 27   direction pin B
MOTOR_PIN_PWM = 25   speed
```

`MotorController` keeps `_running` / `_forward` / `_speedPercent` as its
only state and recomputes both direction pins and PWM duty in
`applyOutputs()` any time one of them changes. When stopped, duty is
forced to 0 regardless of the stored speed setpoint, so the setpoint
survives a stop/start cycle.

## Files

```
config.h                  all WiFi/MQTT/topic/pin constants
BME280Sensor.h/.cpp       I2C sensor wrapper (unchanged from prototyping)
MotorController.h/.cpp    direction + PWM speed control
MqttInterlockClient.h/.cpp  generic MQTT pub/sub wrapper
main.cpp                  owns motor/sensor/mqtt objects, wires it all together
mosquitto.conf             broker config
```

