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

## Notes / limitations

- Public brokers are best-effort and unauthenticated by default - fine for a
  classroom exercise, not for a real safety-critical deployment.
- Like Task 2.3's hardcoded `partnerMAC`, pairing here is a hardcoded topic
  name; there's no runtime discovery step.
- QoS is left at PubSubClient's default (QoS 0); since we publish
  continuously and rely on a silence-based timeout rather than any single
  message, an occasionally dropped packet has no lasting effect.

  See the full [Two-Way Wireless Interlock System](Project_6_Documentation.pdf) for schematics, firmware, and calibration notes.
