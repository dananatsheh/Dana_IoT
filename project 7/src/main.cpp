#include <Arduino.h>
#include<WiFi.h>

#define SSID       "CYBER_EXT"
#define PASS       "cyberap2025"

void setup() {
 Serial.begin(115200);
 delay(500);
 WiFi.begin(SSID, PASS);

 while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("WiFi connected, IP: ");
  Serial.println(WiFi.localIP());

  WiFiClient ob;
  bool tcpOk = ob.connect("192.168.1.33", 1883);

  if (tcpOk) {
    Serial.println("TCP connect OK");
} else {
    Serial.println("TCP connect FAILED");
}
}

void loop() {
}