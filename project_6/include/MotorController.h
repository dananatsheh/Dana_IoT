#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include <Arduino.h>

class MotorController {
public:
  MotorController(uint8_t in1, uint8_t in2, uint8_t ena)
    : _in1(in1), _in2(in2), _ena(ena) {}

  void begin() {
    pinMode(_in1, OUTPUT);
    pinMode(_in2, OUTPUT);
    pinMode(_ena, OUTPUT);
    analogWrite(_ena, 255);
    stop();
  }

  void run() {
    digitalWrite(_in1, HIGH);
    digitalWrite(_in2, LOW);
    _running = true;
  }

  void stop() {
    digitalWrite(_in1, LOW);
    digitalWrite(_in2, LOW);
    _running = false;
  }

  void setRunning(bool shouldRun) {
    shouldRun ? run() : stop();
  }

  bool isRunning() const {
    return _running;
  }

private:
  uint8_t _in1, _in2, _ena;
  bool    _running = false;
};

#endif
