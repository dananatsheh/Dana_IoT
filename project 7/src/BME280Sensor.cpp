#include "BME280Sensor.h"
#include "config.h"

bool BME280Sensor::begin() {
  bool ok = _bme.begin(_address);
  _sensorInitSuccess = ok;
  return _sensorInitSuccess;
}

void BME280Sensor::readNow() {
  if (!_sensorInitSuccess) {
    return;
  }

  float t = _bme.readTemperature();
  float h = _bme.readHumidity();
  float p = _bme.readPressure() / 100.0F;

  if (isnan(t) || isnan(h) || isnan(p)) {
    _lastReadSuccess = false;
    return;
  }

  _temperature = t;
  _humidity    = h;
  _pressure    = p;
  _lastReadSuccess = true;
  _lastReadMs = millis();
}

float BME280Sensor::getTemperature() const {
  return _temperature;
}

float BME280Sensor::getHumidity() const {
  return _humidity;
}

float BME280Sensor::getPressure() const {
  return _pressure;
}

bool BME280Sensor::isDataValid() const {
  bool freshEnough = (millis() - _lastReadMs) < SENSOR_STALE_MS;
  return _sensorInitSuccess && _lastReadSuccess && freshEnough;
}
