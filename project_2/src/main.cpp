#include <Arduino.h>
#include <math.h>

// ---------------------- PIN ASSIGNMENTS ----------------------
const int PIN_RELAY          = 23;  
const int PIN_CURRENT_SENSOR = 34;  
const int PIN_BUTTON         = 21;  

const int PIN_LED_RUNNING      = 12; // Green
const int PIN_LED_STOPPED      = 19;  // Yellow
const int PIN_LED_HIGH_CURRENT = 33; // Red
const int PIN_LED_LOW_CURRENT  = 27; // Blue

unsigned long motorStartTime = 0;

// ---------------------- CURRENT SENSOR CALIBRATION ----------------------
const float ADC_MAX_VOLTAGE        = 3.3f;   
const int   ADC_RESOLUTION          = 4095;  
float ZERO_CURRENT_VOLTAGE = 0.0f;   
const float SENSITIVITY_V_PER_A    = 0.066f; 

const int CURRENT_SAMPLE_COUNT = 50;

// ---------------------- THRESHOLDS ----------------------
const float HIGH_CURRENT_THRESHOLD_A = 0.4f;   
const float LOW_CURRENT_THRESHOLD_A  = 0.15f;  
const float CURRENT_HYSTERESIS_A     = 0.1f;  

// ---------------------- RELAY POLARITY ----------------------
const bool RELAY_ACTIVE_LOW = true;

// ---------------------- TIMING ----------------------
const unsigned long BUTTON_DEBOUNCE_MS = 40;
const unsigned long SAMPLE_INTERVAL_MS = 50;   

// ---------------------- SYSTEM STATES ----------------------
enum SystemState {
  STATE_STOPPED,       
  STATE_RUNNING,       
  STATE_LOW_CURRENT,   
  STATE_FAULT_HIGH     
};

SystemState currentState = STATE_STOPPED;

// ---------------------- BUTTON DEBOUNCE STATE ----------------------
bool lastButtonReading   = HIGH; 
bool debouncedButtonState = HIGH;
unsigned long lastButtonChangeMs = 0;
bool buttonPressEvent = false; 

unsigned long lastSampleMs = 0;

// ---------------------- FORWARD DECLARATIONS ----------------------
void updateButton();
void handleButtonPress();
float readCurrentAmps();
void evaluateCurrent(float currentA);
void setRelay(bool energized);
void updateLEDs();
void calibrateCurrentSensor();

// ---------------------- AUTO CALIBRATION ----------------------
void calibrateCurrentSensor() {
  const int CALIBRATION_SAMPLES = 300;
  uint32_t adcSum = 0;

  Serial.println();
  Serial.println("=================================");
  Serial.println("Current sensor auto calibration");
  Serial.println("Motor must be OFF...");
  Serial.println("=================================");

  delay(1000);

  for (int i = 0; i < CALIBRATION_SAMPLES; i++) {
    adcSum += analogRead(PIN_CURRENT_SENSOR);
    delay(2);
  }

  float adcAverage = adcSum / (float)CALIBRATION_SAMPLES;
  ZERO_CURRENT_VOLTAGE = (adcAverage / ADC_RESOLUTION) * ADC_MAX_VOLTAGE;

  Serial.print("Zero voltage = ");
  Serial.print(ZERO_CURRENT_VOLTAGE, 4);
  Serial.println(" V");
  Serial.println("Calibration complete.");
  Serial.println();
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_RELAY, OUTPUT);
  pinMode(PIN_BUTTON, INPUT_PULLUP);

  pinMode(PIN_LED_RUNNING, OUTPUT);
  pinMode(PIN_LED_STOPPED, OUTPUT);
  pinMode(PIN_LED_HIGH_CURRENT, OUTPUT);
  pinMode(PIN_LED_LOW_CURRENT, OUTPUT);

  analogReadResolution(12);

  // Quick boot self-test flash: all 4 LEDs on for 2s, then off.
  digitalWrite(PIN_LED_RUNNING, HIGH);
  digitalWrite(PIN_LED_STOPPED, HIGH);
  digitalWrite(PIN_LED_HIGH_CURRENT, HIGH);
  digitalWrite(PIN_LED_LOW_CURRENT, HIGH);
  delay(2000);
  digitalWrite(PIN_LED_RUNNING, LOW);
  digitalWrite(PIN_LED_STOPPED, LOW);
  digitalWrite(PIN_LED_HIGH_CURRENT, LOW);
  digitalWrite(PIN_LED_LOW_CURRENT, LOW);

  // Motor OFF first (fail-safe), then calibrate current sensor while it's guaranteed at 0A
  setRelay(false);
  delay(1000); 

  calibrateCurrentSensor();

  currentState = STATE_STOPPED;
  updateLEDs();

  Serial.println("Motor protection system booted. State: STOPPED.");
}

void loop() {
  updateButton();

  if (buttonPressEvent) {
    handleButtonPress();
    buttonPressEvent = false;
  }

  if (millis() - lastSampleMs >= SAMPLE_INTERVAL_MS) {
    lastSampleMs = millis();
    float currentA = readCurrentAmps();
    Serial.print("Current: ");
    Serial.print(currentA, 3);
    Serial.println(" A");
    evaluateCurrent(currentA);
  }

  updateLEDs();
}

// ---------------------- BUTTON HANDLING ----------------------
void updateButton() {
  bool reading = digitalRead(PIN_BUTTON);

  if (reading != lastButtonReading) {
    lastButtonChangeMs = millis();
  }

  if ((millis() - lastButtonChangeMs) > BUTTON_DEBOUNCE_MS) {
    if (reading != debouncedButtonState) {
      debouncedButtonState = reading;
      if (debouncedButtonState == LOW) {
        buttonPressEvent = true;
      }
    }
  }

  lastButtonReading = reading;
}

void handleButtonPress() {
  switch (currentState) {
    case STATE_RUNNING:
    case STATE_LOW_CURRENT:
      setRelay(false);
      currentState = STATE_STOPPED;
      Serial.println("E-stop pressed. Motor stopped.");
      break;

    case STATE_STOPPED:
    case STATE_FAULT_HIGH:
      setRelay(true);
      motorStartTime = millis();
      currentState = STATE_RUNNING;
      Serial.println("Restart requested. Re-enabling motor and re-checking current.");
      break;
  }
}

// ---------------------- CURRENT SENSING ----------------------
float readCurrentAmps() {
  long rawSum = 0;
  for (int i = 0; i < CURRENT_SAMPLE_COUNT; i++) {
    rawSum += analogRead(PIN_CURRENT_SENSOR);
  }
  float rawAvg = rawSum / (float)CURRENT_SAMPLE_COUNT;
  float voltage = (rawAvg / ADC_RESOLUTION) * ADC_MAX_VOLTAGE;
  float current = -(voltage - ZERO_CURRENT_VOLTAGE) / SENSITIVITY_V_PER_A;
  return current;
}

void evaluateCurrent(float currentA) {
  if (currentState == STATE_STOPPED) {
    return; 
  }

  if (currentState == STATE_FAULT_HIGH) {
    return; 
  }

  // Ignore startup surge for the first 500 ms after (re)starting
  if (millis() - motorStartTime > 500 && fabs(currentA) > HIGH_CURRENT_THRESHOLD_A) {
    setRelay(false);
    currentState = STATE_FAULT_HIGH;
    Serial.print("OVERCURRENT TRIP: ");
    Serial.print(currentA);
    Serial.println(" A. Relay opened. Press button to reset.");
    return;
  }

  
  if (fabs(currentA) < (LOW_CURRENT_THRESHOLD_A - CURRENT_HYSTERESIS_A)) {
    currentState = STATE_LOW_CURRENT;
  } else if (fabs(currentA) > (LOW_CURRENT_THRESHOLD_A + CURRENT_HYSTERESIS_A)) {
    currentState = STATE_RUNNING;
  }
}

// ---------------------- OUTPUTS ----------------------
void setRelay(bool energized) {
  bool pinState = RELAY_ACTIVE_LOW ? !energized : energized;
  digitalWrite(PIN_RELAY, pinState ? HIGH : LOW);
}

void updateLEDs() {
  digitalWrite(PIN_LED_RUNNING,      currentState == STATE_RUNNING      ? HIGH : LOW);
  digitalWrite(PIN_LED_STOPPED,      currentState == STATE_STOPPED      ? HIGH : LOW);
  digitalWrite(PIN_LED_HIGH_CURRENT, currentState == STATE_FAULT_HIGH   ? HIGH : LOW);
  digitalWrite(PIN_LED_LOW_CURRENT,  currentState == STATE_LOW_CURRENT  ? HIGH : LOW);
}
