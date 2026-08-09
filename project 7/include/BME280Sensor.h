#ifndef BME280_SENSOR_H
#define BME280_SENSOR_H

#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

class BME280Sensor {
public:
  explicit BME280Sensor(uint8_t address = 0x76) : _address(address) {}

  bool begin();
  void readNow();

  float getTemperature() const;
  float getHumidity() const;
  float getPressure() const;

  bool isDataValid() const;

private:
  Adafruit_BME280 _bme;
  uint8_t _address;

  float _temperature;
  float _humidity;
  float _pressure;

  bool _lastReadSuccess;
  bool _sensorInitSuccess;
  unsigned long _lastReadMs;
};

#endif
