#include <Arduino.h>
#include "config.h"
#include "CurrentSensor.h"
#include "L298N.h"


L298N motor(Config::PIN_MOTOR_IN1, Config::PIN_MOTOR_IN2, Config::PIN_MOTOR_ENA,
            Config::PWM_CHANNEL, Config::PWM_FREQ_HZ, Config::PWM_RESOLUTION_BITS);

enum class SystemState : uint8_t {
  STOPPED,
  RUNNING,
  LOW_CURRENT,    
  HIGH_CURRENT   
};

SystemState currentState = SystemState::STOPPED;
unsigned long motorStartTime = 0;


volatile bool          estopEvent = false;
volatile unsigned long lastIsrMs  = 0;

void IRAM_ATTR onEstopChange() {
  unsigned long now = millis();
  if (now - lastIsrMs < Config::ESTOP_DEBOUNCE_MS) {
    return;
  }
  lastIsrMs  = now;
  estopEvent = true;
}

void handleEstopEvent();
void evaluateCurrent(float amps);
void enterFault(float amps);
void updateLEDs();

void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(Config::PIN_LED_RUNNING,      OUTPUT);
  pinMode(Config::PIN_LED_STOPPED,      OUTPUT);
  pinMode(Config::PIN_LED_HIGH_CURRENT, OUTPUT);
  pinMode(Config::PIN_LED_LOW_CURRENT,  OUTPUT);
  pinMode(Config::PIN_ESTOP, INPUT_PULLUP);

  digitalWrite(Config::PIN_LED_RUNNING,      HIGH);
  digitalWrite(Config::PIN_LED_STOPPED,      HIGH);
  digitalWrite(Config::PIN_LED_HIGH_CURRENT, HIGH);
  digitalWrite(Config::PIN_LED_LOW_CURRENT,  HIGH);
  delay(1000);
  digitalWrite(Config::PIN_LED_RUNNING,      LOW);
  digitalWrite(Config::PIN_LED_STOPPED,      LOW);
  digitalWrite(Config::PIN_LED_HIGH_CURRENT, LOW);
  digitalWrite(Config::PIN_LED_LOW_CURRENT,  LOW);

  analogReadResolution(Config::ADC_RESOLUTION_BITS);


  motor.begin();
  motor.stop();

  CurrentSensor_Init(Config::PIN_CURRENT_SENSOR, Config::CURRENT_SENSITIVITY_V_PER_A);
  delay(500);
  Serial.println(F("Calibrating current sensor - keep motor OFF..."));
  CurrentSensor_Calibrate(Config::CURRENT_CALIBRATION_SAMPLES);
  Serial.println(F("Calibration complete."));

  attachInterrupt(digitalPinToInterrupt(Config::PIN_ESTOP), onEstopChange, FALLING);

  currentState = SystemState::STOPPED;
  updateLEDs();
  Serial.println(F("Motor protection system booted. State: STOPPED."));
}

void loop() {
  if (estopEvent) {
    noInterrupts();
    estopEvent = false;
    interrupts();
    handleEstopEvent();
  }

  static unsigned long lastSampleMs = 0;
  static unsigned long lastReportMs = 0;


  if (millis() - lastSampleMs >= Config::CURRENT_SAMPLE_INTERVAL_MS) {
    lastSampleMs = millis();

    float amps = CurrentSensor_ReadAmps(); 

    if (millis() - lastReportMs >= Config::SERIAL_REPORT_INTERVAL_MS) {
      lastReportMs = millis();
      Serial.print(F("Current: "));
      Serial.print(amps, 3);
      Serial.println(F(" A"));
    }

    evaluateCurrent(amps);
  }

  updateLEDs();
}

void handleEstopEvent() {
  switch (currentState) {
    case SystemState::RUNNING:
    case SystemState::LOW_CURRENT:
      motor.brake();
      currentState = SystemState::STOPPED;
      Serial.println(F("E-STOP pressed. Motor braked -> STOPPED."));
      break;

    case SystemState::STOPPED:
    case SystemState::HIGH_CURRENT:
      motor.forward(Config::DEFAULT_RUN_SPEED);
      motorStartTime = millis();
      currentState = SystemState::RUNNING;
      Serial.println(F("Start/Reset requested. Motor running -> RUNNING."));
      break;
  }
}

void evaluateCurrent(float amps) {

  if (currentState != SystemState::RUNNING && currentState != SystemState::LOW_CURRENT) {
    return;
  }

  bool pastStartupSurge = (millis() - motorStartTime) > Config::STARTUP_SURGE_IGNORE_MS;

  if (pastStartupSurge && CurrentSensor_IsOvercurrent(Config::CURRENT_LIMIT_A)) {
    enterFault(amps);
    return;
  }


  if (fabs(amps) < (Config::LOW_CURRENT_THRESHOLD_A - Config::CURRENT_HYSTERESIS_A)) {
    currentState = SystemState::LOW_CURRENT;
  } else if (fabs(amps) > (Config::LOW_CURRENT_THRESHOLD_A + Config::CURRENT_HYSTERESIS_A)) {
    currentState = SystemState::RUNNING;
  }
}

void enterFault(float amps) {
  motor.brake(); 
  currentState = SystemState::HIGH_CURRENT;
  Serial.print(F("OVERCURRENT TRIP: "));
  Serial.print(amps, 3);
  Serial.println(F(" A. Motor braked. Press E-Stop button to reset."));
}

void updateLEDs() {
  digitalWrite(Config::PIN_LED_RUNNING,      currentState == SystemState::RUNNING      ? HIGH : LOW);
  digitalWrite(Config::PIN_LED_STOPPED,      currentState == SystemState::STOPPED      ? HIGH : LOW);
  digitalWrite(Config::PIN_LED_HIGH_CURRENT, currentState == SystemState::HIGH_CURRENT ? HIGH : LOW);
  digitalWrite(Config::PIN_LED_LOW_CURRENT,  currentState == SystemState::LOW_CURRENT  ? HIGH : LOW);
}