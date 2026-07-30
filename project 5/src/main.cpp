#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include "config.h"
#include "Message.h"

uint8_t partnerMAC[6] = {0x9C, 0x13, 0x9E, 0xAB, 0x6C, 0xB4};

unsigned long arrive_time = 0;
bool receivedRunMotor = false;

bool PrevIRReading = 1;
unsigned long last_changed = 0;
bool stableObstacleState = 1;

void OnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int len) {
  message incoming;
  memcpy(&incoming, incomingData, sizeof(incoming));
  arrive_time = millis();
  receivedRunMotor = incoming.runMotor;
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
}

void updateIRSensor() {
  bool current_reading = digitalRead(IR);
  if (current_reading != PrevIRReading) {
    PrevIRReading = current_reading;
    last_changed = millis();
  }
  if (millis() - last_changed > 500) {
    stableObstacleState = PrevIRReading;
  }
}

void setup() {
  Serial.begin(115200); delay(500);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, partnerMAC, 6);
  peerInfo.channel = ESPNOW_CHANNEL;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  pinMode(IR, INPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(TransmittionLED, OUTPUT);
  pinMode(MotorLED, OUTPUT);
  pinMode(IRLED, OUTPUT);
  pinMode(RecieveLED, OUTPUT);

  analogWrite(ENA, 255);
}

void loop() {
  updateIRSensor();

  bool linkLost = (millis() - arrive_time > 2000);

  bool motorShouldRun = (!linkLost) && receivedRunMotor;

  if (motorShouldRun) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
  } else {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
  }

  Serial.print("linkLost: "); Serial.print(linkLost);
  Serial.print(" | receivedRunMotor: "); Serial.print(receivedRunMotor);
  Serial.print(" | motorShouldRun: "); Serial.println(motorShouldRun);

  digitalWrite(MotorLED, motorShouldRun);
  digitalWrite(IRLED, !stableObstacleState);
  digitalWrite(RecieveLED, !linkLost);

  message outgoing;
  outgoing.senderID = 0;
  outgoing.runMotor = stableObstacleState;
  outgoing.timestamp = millis();

  esp_now_send(partnerMAC, (uint8_t *)&outgoing, sizeof(outgoing));
  digitalWrite(TransmittionLED, !digitalRead(TransmittionLED));

  delay(20);
}