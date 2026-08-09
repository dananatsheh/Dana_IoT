#include "MotorController.h"
#include "config.h"

void MotorController::begin() {
  pinMode(MOTOR_PIN_IN1, OUTPUT);
  pinMode(MOTOR_PIN_IN2, OUTPUT);

  ledcSetup(MOTOR_PWM_CHANNEL, MOTOR_PWM_FREQ_HZ, MOTOR_PWM_RESOLUTION_BITS);
  ledcAttachPin(MOTOR_PIN_PWM, MOTOR_PWM_CHANNEL);

  applyOutputs();
}

void MotorController::start() {
  _running = true;
  applyOutputs();
}

void MotorController::stop() {
  _running = false;
  applyOutputs();
}

void MotorController::setForward(bool forward) {
  _forward = forward;
  applyOutputs();
}

void MotorController::setSpeedPercent(int percent) {
  if (percent < 0) {
    percent = 0;
  } else if (percent > 100) {
    percent = 100;
  }
  _speedPercent = (uint8_t)percent;
  applyOutputs();
}

bool MotorController::isRunning() const {
  return _running;
}

bool MotorController::isForward() const {
  return _forward;
}

uint8_t MotorController::getSpeedPercent() const {
  return _speedPercent;
}

void MotorController::applyOutputs() {
  digitalWrite(MOTOR_PIN_IN1, _forward ? HIGH : LOW);
  digitalWrite(MOTOR_PIN_IN2, _forward ? LOW : HIGH);

  uint32_t maxDuty = (1 << MOTOR_PWM_RESOLUTION_BITS) - 1;
  uint32_t duty = _running ? (maxDuty * _speedPercent) / 100 : 0;
  ledcWrite(MOTOR_PWM_CHANNEL, duty);
}
