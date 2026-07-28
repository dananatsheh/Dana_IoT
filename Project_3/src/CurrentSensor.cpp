#include "CurrentSensor.h"
#include "config.h"

namespace {
  int   s_pin           = -1;
  float s_sensitivity   = Config::CURRENT_SENSITIVITY_V_PER_A;
  float s_zeroVoltage   = Config::ADC_MAX_VOLTAGE / 2.0f; // sane default until calibrated
  float s_filteredAmps  = 0.0f;
  bool  s_filterPrimed  = false;
}

void CurrentSensor_Init(int pin, float sensitivityVPerA) {
  s_pin         = pin;
  s_sensitivity = sensitivityVPerA;
  pinMode(s_pin, INPUT);
  s_filterPrimed = false;
}

void CurrentSensor_Calibrate(int numSamples) {
  if (s_pin < 0) return; // not initialized

  uint32_t sum = 0;
  for (int i = 0; i < numSamples; i++) {
    sum += analogRead(s_pin);
    delay(2);
  }

  float avgRaw = sum / (float)numSamples;
  s_zeroVoltage = (avgRaw / Config::ADC_RESOLUTION) * Config::ADC_MAX_VOLTAGE;


  s_filterPrimed = false;
}

float CurrentSensor_ReadAmps() {
  if (s_pin < 0) return 0.0f;


  long rawSum = 0;
  for (int i = 0; i < Config::CURRENT_OVERSAMPLE_COUNT; i++) {
    rawSum += analogRead(s_pin);
  }
  float rawAvg  = rawSum / (float)Config::CURRENT_OVERSAMPLE_COUNT;
  float voltage = (rawAvg / Config::ADC_RESOLUTION) * Config::ADC_MAX_VOLTAGE;
  float instantAmps = (voltage - s_zeroVoltage) / s_sensitivity;


  if (!s_filterPrimed) {
    s_filteredAmps = instantAmps;
    s_filterPrimed = true;
  } else {
    s_filteredAmps = Config::CURRENT_EMA_ALPHA * instantAmps +
                     (1.0f - Config::CURRENT_EMA_ALPHA) * s_filteredAmps;
  }

  return s_filteredAmps;
}

bool CurrentSensor_IsOvercurrent(float thresholdA) {
  // Evaluates the last filtered reading rather than sampling again,
  // so it stays consistent with whatever value was just logged/used
  // by the caller after their ReadAmps() call.
  return fabs(s_filteredAmps) > thresholdA;
}
