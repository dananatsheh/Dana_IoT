#include "MqttInterlockClient.h"
#include "config.h"

MqttInterlockClient* MqttInterlockClient::_instance = nullptr;

MqttInterlockClient::MqttInterlockClient()
  : _mqttClient(_wifiClient) {}

void MqttInterlockClient::begin() {
  _instance = this;

  connectWiFi();

  _mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  _mqttClient.setCallback(MqttInterlockClient::staticCallback);
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

void MqttInterlockClient::publish(const char* topic, const char* payload) {
  if (!_mqttClient.connected()) {
    return;
  }
  _mqttClient.publish(topic, payload);
}

void MqttInterlockClient::subscribe(const char* topic, CommandSlot slot) {
  if (_routeCount < MAX_ROUTES) {
    _routes[_routeCount].topic = topic;
    _routes[_routeCount].slot = slot;
    _routeCount++;
  }

  if (_mqttClient.connected()) {
    _mqttClient.subscribe(topic);
  }
}

bool MqttInterlockClient::isConnected() {
  return _mqttClient.connected();
}

bool MqttInterlockClient::getNewStateCommand(String &outValue) {
  if (!_stateInbox.hasNew) {
    return false;
  }
  outValue = _stateInbox.value;
  _stateInbox.hasNew = false;
  return true;
}

bool MqttInterlockClient::getNewDirectionCommand(String &outValue) {
  if (!_directionInbox.hasNew) {
    return false;
  }
  outValue = _directionInbox.value;
  _directionInbox.hasNew = false;
  return true;
}

bool MqttInterlockClient::getNewSpeedCommand(String &outValue) {
  if (!_speedInbox.hasNew) {
    return false;
  }
  outValue = _speedInbox.value;
  _speedInbox.hasNew = false;
  return true;
}

bool MqttInterlockClient::consumeReconnectFlag() {
  if (!_reconnectFlag) {
    return false;
  }
  _reconnectFlag = false;
  return true;
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

  String clientId = String(MQTT_CLIENT_ID_PREFIX) + String((uint32_t)ESP.getEfuseMac(), HEX);

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
    resubscribeAll();
    _reconnectFlag = true;
  } else {
    Serial.print(" failed, rc=");
    Serial.println(_mqttClient.state());
  }
}

void MqttInterlockClient::resubscribeAll() {
  for (uint8_t i = 0; i < _routeCount; i++) {
    _mqttClient.subscribe(_routes[i].topic.c_str());
    Serial.print("Subscribed to: ");
    Serial.println(_routes[i].topic);
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

  String value;
  for (unsigned int i = 0; i < length; i++) {
    value += (char)payload[i];
  }

  for (uint8_t i = 0; i < _routeCount; i++) {
    if (_routes[i].topic == topic) {
      switch (_routes[i].slot) {
        case CommandSlot::STATE:
          _stateInbox.value = value;
          _stateInbox.hasNew = true;
          break;
        case CommandSlot::DIRECTION:
          _directionInbox.value = value;
          _directionInbox.hasNew = true;
          break;
        case CommandSlot::SPEED:
          _speedInbox.value = value;
          _speedInbox.hasNew = true;
          break;
      }
      return;
    }
  }
}

void MqttInterlockClient::staticCallback(char* topic, uint8_t* payload, unsigned int length) {
  if (_instance != nullptr) {
    _instance->handleMessage(topic, payload, length);
  }
}
