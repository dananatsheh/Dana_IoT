#include "MqttInterlockClient.h"

MqttInterlockClient* MqttInterlockClient::_instance = nullptr;

MqttInterlockClient::MqttInterlockClient(LinkMonitor& linkMonitor)
  : _mqttClient(_wifiClient), _linkMonitor(linkMonitor) {}

void MqttInterlockClient::begin() {
  _instance = this;

  connectWiFi();

  _mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  _mqttClient.setCallback(MqttInterlockClient::staticCallback);

  connectBroker();
}

void MqttInterlockClient::loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }
  if (!_mqttClient.connected()) {
    connectBroker();
  }
  _mqttClient.loop();
}

void MqttInterlockClient::publishState(bool runMotor) {
  if (!_mqttClient.connected()) {
    return;
  }


  const char* payload = runMotor ? "1" : "0";
  _mqttClient.publish(MY_TOPIC, payload);
}

bool MqttInterlockClient::isConnected() {
  return _mqttClient.connected();
}

void MqttInterlockClient::connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  Serial.print("Connecting to WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(300);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("WiFi connected, IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi connection failed, will retry.");
  }
}

void MqttInterlockClient::connectBroker() {
  if (_mqttClient.connected()) {
    return;
  }
  if (millis() - _lastReconnectAttempt < MQTT_RECONNECT_MS) {
    return;
  }
  _lastReconnectAttempt = millis();

  String clientId = String("esp32-") + MY_ID + "-" + String((uint32_t)ESP.getEfuseMac(), HEX);

  Serial.print("Connecting to MQTT broker ");
  Serial.print(MQTT_BROKER);
  Serial.print(" ...");

  bool connected;
  if (strlen(MQTT_USERNAME) > 0) {
    connected = _mqttClient.connect(clientId.c_str(), MQTT_USERNAME, MQTT_PASSWORD);
  } else {
    connected = _mqttClient.connect(clientId.c_str());
  }

  if (connected) {
    Serial.println(" connected.");
    _mqttClient.subscribe(PARTNER_TOPIC);
    Serial.print("Subscribed to: ");
    Serial.println(PARTNER_TOPIC);
  } else {
    Serial.print(" failed, rc=");
    Serial.println(_mqttClient.state());
  }
}

void MqttInterlockClient::handleMessage(char* topic, uint8_t* payload, unsigned int length) {
  Serial.print("[recv] topic=");
  Serial.print(topic);
  Serial.print(" len=");
  Serial.print(length);
  Serial.print(" payload=");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if (length < 1) {
    return;
  }


  constexpr bool obstacleIsOne = false;
  bool obstacleDetected = obstacleIsOne ? (payload[1] == '1') : (payload[0] == '0');
  _receivedRunMotor = !obstacleDetected;
  _linkMonitor.notifyPacketReceived();
}

void MqttInterlockClient::staticCallback(char* topic, uint8_t* payload, unsigned int length) {
  if (_instance != nullptr) {
    _instance->handleMessage(topic, payload, length);
  }
}