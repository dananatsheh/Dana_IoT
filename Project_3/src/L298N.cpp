#include "L298N.h"

L298N::L298N(uint8_t in1, uint8_t in2, uint8_t ena,
             uint8_t pwmChannel, uint32_t pwmFreqHz, uint8_t pwmResolutionBits)
  : _in1(in1), _in2(in2), _ena(ena),
    _pwmChannel(pwmChannel), _pwmFreqHz(pwmFreqHz), _pwmResolutionBits(pwmResolutionBits),
    _direction(MotorDirection::STOPPED), _speed(0) {}

void L298N::begin() {
  pinMode(_in1, OUTPUT);
  pinMode(_in2, OUTPUT);

  ledcSetup(_pwmChannel, _pwmFreqHz, _pwmResolutionBits);
  ledcAttachPin(_ena, _pwmChannel);

  stop();
}

void L298N::writeSpeed(uint8_t speed) {
  _speed = speed;
  ledcWrite(_pwmChannel, speed);  // channel-based write, not pin-based
}

void L298N::forward(uint8_t speed) {
  digitalWrite(_in1, HIGH);
  digitalWrite(_in2, LOW);
  writeSpeed(speed);
  _direction = MotorDirection::FORWARD;
}

void L298N::reverse(uint8_t speed) {
  digitalWrite(_in1, LOW);
  digitalWrite(_in2, HIGH);
  writeSpeed(speed);
  _direction = MotorDirection::REVERSE;
}

void L298N::stop() {
  digitalWrite(_in1, LOW);
  digitalWrite(_in2, LOW);
  writeSpeed(0);
  _direction = MotorDirection::STOPPED;
}

void L298N::brake() {
  digitalWrite(_in1, HIGH);
  digitalWrite(_in2, HIGH);
  writeSpeed(0);
  _direction = MotorDirection::BRAKING;
}
