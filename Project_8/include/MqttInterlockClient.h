#ifndef MQTT_INTERLOCK_CLIENT_H
#define MQTT_INTERLOCK_CLIENT_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

enum class CommandSlot {
  STATE,
  DIRECTION,
  SPEED
};

class MqttInterlockClient {
public:
  MqttInterlockClient();

  void begin();
  void loop();

  void publish(const char* topic, const char* payload, bool retained = false);
  void subscribe(const char* topic, CommandSlot slot);

  bool isConnected();

  bool getNewStateCommand(String &outValue);
  bool getNewDirectionCommand(String &outValue);
  bool getNewSpeedCommand(String &outValue);

  bool consumeReconnectFlag();

private:
  struct CommandInbox {
    String value;
    bool hasNew = false;
  };

  struct TopicRoute {
    String topic;
    CommandSlot slot;
  };

  static const uint8_t MAX_ROUTES = 8;

  WiFiClient _wifiClient;
  PubSubClient _mqttClient;

  TopicRoute _routes[MAX_ROUTES];
  uint8_t _routeCount = 0;

  CommandInbox _stateInbox;
  CommandInbox _directionInbox;
  CommandInbox _speedInbox;

  bool _reconnectFlag = false;
  unsigned long _lastReconnectAttempt = 0;

  void connectWiFi();
  void connectBroker();
  void resubscribeAll();

  void handleMessage(char* topic, uint8_t* payload, unsigned int length);
  static void staticCallback(char* topic, uint8_t* payload, unsigned int length);
  static MqttInterlockClient* _instance;
};

#endif
