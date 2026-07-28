#pragma once
#include <Arduino.h>



namespace Config {

constexpr int PIN_MOTOR_IN1 = 25;   
constexpr int PIN_MOTOR_IN2 = 26;   
constexpr int PIN_MOTOR_ENA = 27;  

constexpr int      PWM_CHANNEL          = 0;      
constexpr uint32_t PWM_FREQ_HZ          = 20000;  
constexpr uint8_t  PWM_RESOLUTION_BITS  = 8;       

constexpr uint8_t DEFAULT_RUN_SPEED = 255;        

constexpr int   PIN_CURRENT_SENSOR = 34;

constexpr float ADC_MAX_VOLTAGE       = 3.3f;
constexpr int   ADC_RESOLUTION_BITS   = 12;
constexpr int   ADC_RESOLUTION        = 4095;

constexpr float CURRENT_SENSITIVITY_V_PER_A = 0.066f; 
constexpr int   CURRENT_OVERSAMPLE_COUNT    = 32;      
constexpr float CURRENT_EMA_ALPHA           = 0.2f;    
constexpr int   CURRENT_CALIBRATION_SAMPLES = 300;

constexpr float CURRENT_LIMIT_A             = 0.4f;   
constexpr unsigned long STARTUP_SURGE_IGNORE_MS = 500; 

constexpr float LOW_CURRENT_THRESHOLD_A = 0.15f;  
constexpr float CURRENT_HYSTERESIS_A    = 0.05f;  

constexpr int PIN_ESTOP = 21;
constexpr unsigned long ESTOP_DEBOUNCE_MS = 40;

constexpr int PIN_LED_RUNNING      = 12;  
constexpr int PIN_LED_STOPPED      = 19;  
constexpr int PIN_LED_HIGH_CURRENT = 33;  
constexpr int PIN_LED_LOW_CURRENT  = 2;  

constexpr unsigned long CURRENT_SAMPLE_INTERVAL_MS = 20;   
constexpr unsigned long SERIAL_REPORT_INTERVAL_MS  = 250;  

} 