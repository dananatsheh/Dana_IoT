#pragma once
#include <Arduino.h>

enum class MotorDirection : uint8_t {
  STOPPED,   
  FORWARD,
  REVERSE,
  BRAKING    
};

class L298N {
public:
  L298N(uint8_t in1, uint8_t in2, uint8_t ena,
        uint8_t pwmChannel, uint32_t pwmFreqHz, uint8_t pwmResolutionBits);

  void begin();

  void forward(uint8_t speed); 
  void reverse(uint8_t speed); 
  void stop();                 
  void brake();                

  MotorDirection getDirection() const { return _direction; }
  uint8_t        getSpeed()     const { return _speed; }

private:
  const uint8_t  _in1;
  const uint8_t  _in2;
  const uint8_t  _ena;
  const uint8_t  _pwmChannel;
  const uint32_t _pwmFreqHz;
  const uint8_t  _pwmResolutionBits;

  MotorDirection _direction;
  uint8_t        _speed;

  void writeSpeed(uint8_t speed);
};
