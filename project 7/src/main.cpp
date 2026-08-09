#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include "config.h"
#include "BME280Sensor.h"
#include "MotorController.h"
#include "MqttInterlockClient.h"

BME280Sensor bme280;
MotorController motor;
MqttInterlockClient mqttClient;

void publishSensorReadings() {
  char buf[16];

  dtostrf(bme280.getTemperature(), 0, 2, buf);
  mqttClient.publish(TOPIC_SENSOR_TEMPERATURE, buf);

  dtostrf(bme280.getHumidity(), 0, 2, buf);
  mqttClient.publish(TOPIC_SENSOR_HUMIDITY, buf);

  dtostrf(bme280.getPressure(), 0, 2, buf);
  mqttClient.publish(TOPIC_SENSOR_PRESSURE, buf);
}

void publishMotorStatus() {
  mqttClient.publish(TOPIC_MOTOR_STATUS_STATE, motor.isRunning() ? "1" : "0");
  mqttClient.publish(TOPIC_MOTOR_STATUS_DIRECTION, motor.isForward() ? "forward" : "reverse");

  char buf[8];
  itoa(motor.getSpeedPercent(), buf, 10);
  mqttClient.publish(TOPIC_MOTOR_STATUS_SPEED, buf);
}

void handleIncomingCommands() {
  String value;

  if (mqttClient.getNewStateCommand(value)) {
    if (value == "1") {
      motor.start();
    } else {
      motor.stop();
    }
  }

  if (mqttClient.getNewDirectionCommand(value)) {
    motor.setForward(value == "forward");
  }

  if (mqttClient.getNewSpeedCommand(value)) {
    motor.setSpeedPercent(value.toInt());
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);

  Wire.begin();
  bool bmeOk = bme280.begin();
  if (bmeOk) {
    Serial.println("Initialized BME280");
  } else {
    Serial.println("BME280 failed to initialize");
  }

  motor.begin();

  mqttClient.begin();
  mqttClient.subscribe(TOPIC_MOTOR_COMMAND_STATE, CommandSlot::STATE);
  mqttClient.subscribe(TOPIC_MOTOR_COMMAND_DIRECTION, CommandSlot::DIRECTION);
  mqttClient.subscribe(TOPIC_MOTOR_COMMAND_SPEED, CommandSlot::SPEED);
}

void loop() {
  mqttClient.loop();

  if (!mqttClient.isConnected()) {
    motor.stop();
  }

  if (mqttClient.consumeReconnectFlag()) {
    publishMotorStatus();
  }

  handleIncomingCommands();

  static unsigned long lastSensorRead = 0;
  if (millis() - lastSensorRead > SENSOR_READ_INTERVAL_MS) {
    lastSensorRead = millis();
    bme280.readNow();

    if (bme280.isDataValid()) {
      publishSensorReadings();
    } else {
      Serial.println("Readings are invalid");
    }
  }

  static unsigned long lastStatusPublish = 0;
  if (millis() - lastStatusPublish > MOTOR_STATUS_PUBLISH_INTERVAL_MS) {
    lastStatusPublish = millis();
    publishMotorStatus();
  }
}
