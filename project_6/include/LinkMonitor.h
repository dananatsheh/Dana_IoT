#ifndef LINK_MONITOR_H
#define LINK_MONITOR_H

#include <Arduino.h>


class LinkMonitor {
public:
  explicit LinkMonitor(unsigned long timeoutMs) : _timeoutMs(timeoutMs) {}

  void notifyPacketReceived() {
    _lastArrival = millis();
  }

  bool isLinkLost() const {
    return (millis() - _lastArrival) > _timeoutMs;
  }

private:
  unsigned long _timeoutMs;
  unsigned long _lastArrival = 0;
};

#endif
