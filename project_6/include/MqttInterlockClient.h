#ifndef MQTT_INTERLOCK_CLIENT_H
#define MQTT_INTERLOCK_CLIENT_H

#include <WiFi.h>
#include <PubSubClient.h>
#include "config.h"
#include "InterlockMessage.h"
#include "LinkMonitor.h"

class MqttInterlockClient {
public:
  explicit MqttInterlockClient(LinkMonitor& linkMonitor);

  void begin();
  void loop();

  void publishState(bool runMotor);


  bool receivedRunMotor() const { return _receivedRunMotor; }

  bool isConnected();

private:
  void connectWiFi();
  void connectBroker();
  void handleMessage(char* topic, uint8_t* payload, unsigned int length);

  static void staticCallback(char* topic, uint8_t* payload, unsigned int length);
  static MqttInterlockClient* _instance;

  WiFiClient   _wifiClient;
  PubSubClient _mqttClient;
  LinkMonitor& _linkMonitor;

  bool          _receivedRunMotor    = false;
  unsigned long _lastReconnectAttempt = 0;
};

#endif
