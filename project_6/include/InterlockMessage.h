#ifndef INTERLOCK_MESSAGE_H
#define INTERLOCK_MESSAGE_H

#include <Arduino.h>
#include <ArduinoJson.h>

struct InterlockMessage {
  String   senderID;
  bool     runMotor;
  uint32_t timestamp;

  String toJson() const {
    JsonDocument doc;
    doc["senderID"]  = senderID;
    doc["runMotor"]  = runMotor;
    doc["timestamp"] = timestamp;

    String out;
    serializeJson(doc, out);
    return out;
  }

  static bool fromJson(const uint8_t* payload, unsigned int length, InterlockMessage& out) {
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, payload, length);
    if (err) {
      return false;
    }
    out.senderID  = doc["senderID"]  | "";
    out.runMotor  = doc["runMotor"]  | false;
    out.timestamp = doc["timestamp"] | 0;
    return true;
  }
};

#endif
