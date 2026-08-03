# Task 2.4 - Two-Way Wireless Interlock over MQTT

Same safety interlock as Task 2.3, re-implemented over **MQTT through a public
broker** instead of ESP-NOW. Two ESP32 boards, each with one IR obstacle
sensor and one DC motor, gate each other's motor exactly like before -
only the transport layer changed.

## How it works

- Each board debounces its own IR sensor and **publishes** its state to its
  own MQTT topic, on a fixed interval, continuously (not only on change).
- Each board **subscribes** to its partner's topic and drives its own local
  motor from whatever it last received.
- If no message arrives from the partner for **2 seconds**, the motor is
  forced off regardless of the last value received (fail-safe link
  timeout) - same logic, same threshold, as Task 2.3.
- Four status LEDs (heartbeat / motor / IR / link) give the same at-a-glance
  diagnostics as before.

## Project structure

```
include/
  config.h                 All pins, WiFi, broker, and pairing settings
  InterlockMessage.h       JSON wire format (MQTT equivalent of Message.h)
  IRSensor.h               Debounced IR sensor
  MotorController.h        H-bridge on/off control
  StatusIndicator.h        4 status LEDs
  LinkMonitor.h            Fail-safe silence timeout
  MqttInterlockClient.h    WiFi + MQTT connection, pub/sub
src/
  MqttInterlockClient.cpp
  main.cpp                 Wires everything together
platformio.ini
```

## Setup (per board)

1. Open `include/config.h` and edit:
   - `WIFI_SSID` / `WIFI_PASSWORD`
   - `MY_ID` / `PARTNER_ID` - give board A and board B **different** IDs
     that point at each other, e.g.:

     | Board | `MY_ID`     | `PARTNER_ID` |
     |-------|-------------|--------------|
     | A     | `studentA`  | `studentB`   |
     | B     | `studentB`  | `studentA`   |

   This is the only thing that should differ between the two students'
   copies of the firmware - just like `partnerMAC[]` in Task 2.3.

2. Broker: defaults to `test.mosquitto.org:1883` (no login needed). To use
   `iot.coreflux.cloud` instead, change `MQTT_BROKER`/`MQTT_PORT` and fill in
   `MQTT_USERNAME`/`MQTT_PASSWORD` if your instructor requires them. Both
   boards must point at the **same** broker.

3. Wiring is identical to Task 2.3 - see pin table in `config.h`
   (IR on GPIO 34, motor driver on GPIO 26/27/25, LEDs on GPIO 16/17/18/19).

4. Build and flash with PlatformIO:
   ```
   pio run -t upload
   pio device monitor
   ```

## Verifying it works

- Serial monitor prints `linkLost | receivedRunMotor | motorShouldRun` every
  loop, same as Task 2.3.
- Block board A's IR sensor -> board B's motor should stop within one
  debounce window (~500 ms).
- Power off board A -> board B's motor should stop within 2 seconds and its
  link LED should go dark, even though nothing "wrong" happened content-wise
  - it's purely the absence of a fresh MQTT message that trips the fail-safe.

## Notes / limitations

- Public brokers are best-effort and unauthenticated by default - fine for a
  classroom exercise, not for a real safety-critical deployment.
- Like Task 2.3's hardcoded `partnerMAC`, pairing here is a hardcoded topic
  name; there's no runtime discovery step.
- QoS is left at PubSubClient's default (QoS 0); since we publish
  continuously and rely on a silence-based timeout rather than any single
  message, an occasionally dropped packet has no lasting effect.
