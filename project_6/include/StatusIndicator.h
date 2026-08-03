#ifndef STATUS_INDICATOR_H
#define STATUS_INDICATOR_H

#include <Arduino.h>

class StatusIndicator {
public:
  StatusIndicator(uint8_t txLed, uint8_t motorLed, uint8_t irLed, uint8_t rxLed)
    : _txLed(txLed), _motorLed(motorLed), _irLed(irLed), _rxLed(rxLed) {}

  void begin() {
    pinMode(_txLed, OUTPUT);
    pinMode(_motorLed, OUTPUT);
    pinMode(_irLed, OUTPUT);
    pinMode(_rxLed, OUTPUT);
  }

  
  void toggleHeartbeat() {
    digitalWrite(_txLed, !digitalRead(_txLed));
  }

  void setMotorState(bool running)          { digitalWrite(_motorLed, running); }
  void setObstacleState(bool obstacleSeen)  { digitalWrite(_irLed, obstacleSeen); }
  void setLinkState(bool linkUp)            { digitalWrite(_rxLed, linkUp); }

private:
  uint8_t _txLed, _motorLed, _irLed, _rxLed;
};

#endif
