#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include <Arduino.h>

class MotorController {
public:
  void begin();

  void start();
  void stop();

  void setForward(bool forward);
  void setSpeedPercent(int percent);

  bool isRunning() const;
  bool isForward() const;
  uint8_t getSpeedPercent() const;

private:
  bool _running = false;
  bool _forward = true;
  uint8_t _speedPercent = 0;

  void applyOutputs();
};

#endif
