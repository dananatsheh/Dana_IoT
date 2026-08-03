#ifndef IR_SENSOR_H
#define IR_SENSOR_H

#include <Arduino.h>

class IRSensor {
public:
  IRSensor(uint8_t pin, unsigned long debounceMs)
    : _pin(pin), _debounceMs(debounceMs) {}

  void begin() {
    pinMode(_pin, INPUT);
    _prevReading  = digitalRead(_pin);
    _stableState  = _prevReading;
    _lastChanged  = millis();
  }

  void update() {
    bool current = digitalRead(_pin);
    if (current != _prevReading) {
      _prevReading = current;
      _lastChanged = millis();
    }
    if (millis() - _lastChanged > _debounceMs) {
      _stableState = _prevReading;
    }
  }

  bool stableRawState() const {
    return _stableState;
  }

  bool obstacleDetected() const {
    return !_stableState;
  }

private:
  uint8_t       _pin;
  unsigned long _debounceMs;
  bool          _prevReading = true;
  bool          _stableState = true;
  unsigned long _lastChanged = 0;
};

#endif
