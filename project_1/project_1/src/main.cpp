#include <Arduino.h>

const int irPin = 25;       
const int potPin = 34;      
const int ledPin = 26;      

const int pwmFreq = 5000;       
const int pwmResolution = 10;   
const int pwmChannel = 0;       

void setup() {
  Serial.begin(115200);

  pinMode(irPin, INPUT);
  pinMode(potPin, INPUT);

  ledcSetup(pwmChannel, pwmFreq, pwmResolution);
  ledcAttachPin(ledPin, pwmChannel);
}

void loop() {
  int irState = digitalRead(irPin);
  int pwmValue = 0;
  float brightnessPercent = 0.0;
  
  if (irState == LOW) { 
    int potValue = analogRead(potPin); 
    pwmValue = map(potValue, 0, 4095, 0, 1023); 
    ledcWrite(pwmChannel, pwmValue);
    brightnessPercent = ((float)pwmValue / 1023.0) * 100.0;
  } else {
    ledcWrite(pwmChannel, 0); 
    pwmValue = 0;
    brightnessPercent = 0.0;
  }

  int currentPotRaw = analogRead(potPin);
  float voltage = (currentPotRaw / 4095.0) * 3.3;

  Serial.print("IR Activated: ");
  Serial.print(irState == LOW ? "YES" : "NO");
  Serial.print(" | Potentiometer Count: ");
  Serial.print(currentPotRaw);
  Serial.print(" | Voltage: ");
  Serial.print(voltage, 2);
  Serial.print("V | Effective LED Intensity: ");
  Serial.print(brightnessPercent, 1);
  Serial.println("%");

  delay(250); 
}
