#pragma once
#include <Arduino.h>

void  CurrentSensor_Init(int pin, float sensitivityVPerA);
void  CurrentSensor_Calibrate(int numSamples = 300);
float CurrentSensor_ReadAmps();
bool  CurrentSensor_IsOvercurrent(float thresholdA);
