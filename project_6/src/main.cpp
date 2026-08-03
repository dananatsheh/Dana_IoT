#include <Arduino.h>
#include "config.h"
#include "IRSensor.h"
#include "MotorController.h"
#include "StatusIndicator.h"
#include "LinkMonitor.h"
#include "MqttInterlockClient.h"



IRSensor            irSensor(IR_PIN, IR_DEBOUNCE_MS);
MotorController      motor(MOTOR_IN1, MOTOR_IN2, MOTOR_ENA);
StatusIndicator      statusLeds(TX_LED, MOTOR_LED, IR_LED, RX_LED);
LinkMonitor          linkMonitor(LINK_TIMEOUT_MS);
MqttInterlockClient  mqttClient(linkMonitor);

unsigned long lastPublish = 0;

void setup() {
  Serial.begin(115200);
  delay(500);

  irSensor.begin();
  motor.begin();
  statusLeds.begin();
  mqttClient.begin();

  Serial.println("Task 2.4 - MQTT Interlock ready.");
  Serial.print("  My ID:         "); Serial.println(MY_ID);
  Serial.print("  Partner ID:    "); Serial.println(PARTNER_ID);
  Serial.print("  Publishing to: "); Serial.println(MY_TOPIC);
  Serial.print("  Listening on:  "); Serial.println(PARTNER_TOPIC);
}

void loop() {
  mqttClient.loop();

  irSensor.update();

  bool linkLost       = linkMonitor.isLinkLost();
  bool motorShouldRun  = (!linkLost) && mqttClient.receivedRunMotor();

  motor.setRunning(motorShouldRun);

  statusLeds.setMotorState(motorShouldRun);
  statusLeds.setObstacleState(irSensor.obstacleDetected());
  statusLeds.setLinkState(!linkLost);


  if (millis() - lastPublish > PUBLISH_INTERVAL_MS) {
    lastPublish = millis();
    mqttClient.publishState(irSensor.stableRawState());
    statusLeds.toggleHeartbeat();
  }

  Serial.print("linkLost: ");        Serial.print(linkLost);
  Serial.print(" | receivedRunMotor: "); Serial.print(mqttClient.receivedRunMotor());
  Serial.print(" | motorShouldRun: ");   Serial.println(motorShouldRun);

  delay(20);
}
