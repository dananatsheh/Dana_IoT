#include <Arduino.h>
#include <WiFi.h>
#include "BME280Sensor.h"

#define SSID       "CYBER_EXT"
#define PASS       "cyberap2025"

#define SENSOR_READ_INTERVAL_MS   2500

BME280Sensor bme280;

void setup() {
  Serial.begin(115200);
  delay(500);
  WiFi.begin(SSID, PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("WiFi connected, IP: ");
  Serial.println(WiFi.localIP());

  WiFiClient ob;
  bool tcpOk = ob.connect("192.168.1.33", 1883);

  if (tcpOk) {
    Serial.println("TCP connect OK");
  } else {
    Serial.println("TCP connect FAILED");
  }

  Wire.begin();
  bool bmeOk = bme280.begin();
  if (bmeOk) {
    Serial.println("Initialized BME280");
  } else {
    Serial.println("BME280 failed to initialize");
  }
}

void loop() {
  static unsigned long lastRead = 0;

  if (millis() - lastRead > SENSOR_READ_INTERVAL_MS) {
    lastRead = millis();
    bme280.readNow();

    bool validity = bme280.isDataValid();
    if (validity) {
      Serial.print("Temperature: ");
      Serial.println(bme280.getTemperature());
      Serial.print("Humidity: ");
      Serial.println(bme280.getHumidity());
      Serial.print("Pressure: ");
      Serial.println(bme280.getPressure());
    } else {
      Serial.println("Readings are invalid");
    }
  }
}