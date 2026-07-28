#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <LittleFS.h>
#include <Preferences.h>
#include <ESPAsyncWebServer.h>
#include <time.h>

#include "config.h"
#include "dashboard_html.h"

Adafruit_BME280 bme;
bool bmeOk = false;

AsyncWebServer server(80);
Preferences prefs;


String dashboardResolved;

String buildDashboardHtml() {
  String html((const char *)DASHBOARD_HTML);

  html.replace("__TEMP_GREEN_MIN__", String(TEMP_GREEN_MIN, 1));
  html.replace("__TEMP_GREEN_MAX__", String(TEMP_GREEN_MAX, 1));
  html.replace("__TEMP_YELLOW_MIN__", String(TEMP_YELLOW_MIN, 1));
  html.replace("__TEMP_YELLOW_MAX__", String(TEMP_YELLOW_MAX, 1));
  html.replace("__TEMP_RED_MIN__", String(TEMP_RED_MIN, 1));
  html.replace("__TEMP_RED_MAX__", String(TEMP_RED_MAX, 1));

  html.replace("__HUM_GREEN_MIN__", String(HUM_GREEN_MIN, 1));
  html.replace("__HUM_GREEN_MAX__", String(HUM_GREEN_MAX, 1));
  html.replace("__HUM_YELLOW_MIN__", String(HUM_YELLOW_MIN, 1));
  html.replace("__HUM_YELLOW_MAX__", String(HUM_YELLOW_MAX, 1));
  html.replace("__HUM_RED_MIN__", String(HUM_RED_MIN, 1));
  html.replace("__HUM_RED_MAX__", String(HUM_RED_MAX, 1));

  html.replace("__PRES_GREEN_MIN__", String(PRES_GREEN_MIN, 1));
  html.replace("__PRES_GREEN_MAX__", String(PRES_GREEN_MAX, 1));
  html.replace("__PRES_YELLOW_MIN__", String(PRES_YELLOW_MIN, 1));
  html.replace("__PRES_YELLOW_MAX__", String(PRES_YELLOW_MAX, 1));
  html.replace("__PRES_RED_MIN__", String(PRES_RED_MIN, 1));
  html.replace("__PRES_RED_MAX__", String(PRES_RED_MAX, 1));

  return html;
}

float temp = NAN;
float hum = NAN;
float pres = NAN;
unsigned long lastRead = 0;
bool hasReading = false;


bool dataIsStale = false;
unsigned long savedEpoch = 0;   
String savedTimestampStr = "";  

unsigned long lastPollMs = 0;
unsigned long lastLogMs = 0;

bool timeSynced();
String currentTimestamp();
void connectWifi();
bool initBME();
void initCsv();
void appendCsv(const String &ts, float t, float h, float p);
void loadPersistedReading();
void savePersistedReading();
void pollSensor();
void logCsv();
void setupRoutes();

bool timeSynced() {
  time_t now = time(nullptr);
  return now > 8 * 3600 * 2;
}

String currentTimestamp() {
  if (timeSynced()) {
    time_t now = time(nullptr);
    struct tm ti;
    localtime_r(&now, &ti);
    char buf[25];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &ti);
    return String(buf);
  }
  unsigned long s = millis() / 1000;
  char buf[16];
  snprintf(buf, sizeof(buf), "boot+%lus", s);
  return String(buf);
}

void connectWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.printf("Connecting to %s ", WIFI_SSID);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < WIFI_CONNECT_TIMEOUT_MS) {
    delay(300);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("Connected. Dashboard: http://%s\n", WiFi.localIP().toString().c_str());
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  } else {
    Serial.println("WiFi failed, starting AP");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    Serial.printf("AP started. Dashboard: http://%s\n", WiFi.softAPIP().toString().c_str());
  }
}

bool initBME() {
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  if (bme.begin(BME280_ADDR_1, &Wire)) return true;
  if (bme.begin(BME280_ADDR_2, &Wire)) return true;
  return false;
}

void initCsv() {
  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS mount failed");
    return;
  }
  if (!LittleFS.exists(CSV_FILE_PATH)) {
    File f = LittleFS.open(CSV_FILE_PATH, "w");
    if (f) {
      f.println(CSV_HEADER);
      f.close();
    }
  }
}

void appendCsv(const String &ts, float t, float h, float p) {
  File check = LittleFS.open(CSV_FILE_PATH, "r");
  if (check) {
    size_t sz = check.size();
    check.close();
    if (sz > CSV_MAX_SIZE_BYTES) {
      LittleFS.remove(CSV_FILE_PATH);
      File f = LittleFS.open(CSV_FILE_PATH, "w");
      if (f) { f.println(CSV_HEADER); f.close(); }
    }
  }

  File f = LittleFS.open(CSV_FILE_PATH, "a");
  if (!f) return;
  f.printf("%s,%.2f,%.2f,%.2f\n", ts.c_str(), t, h, p);
  f.close();
}

void loadPersistedReading() {
  prefs.begin("envmon", true); 
  bool has = prefs.getBool("has", false);
  if (has) {
    temp = prefs.getFloat("temp", NAN);
    hum = prefs.getFloat("hum", NAN);
    pres = prefs.getFloat("pres", NAN);
    savedEpoch = prefs.getULong("epoch", 0);
    savedTimestampStr = prefs.getString("ts", "");
    hasReading = true;
    dataIsStale = true;
    Serial.printf("[NVS] Restored last reading: %.2f C, %.2f %%, %.2f hPa (saved %s)\n",
                  temp, hum, pres, savedTimestampStr.c_str());
  } else {
    Serial.println("[NVS] No persisted reading found");
  }
  prefs.end();
}

void savePersistedReading() {
  prefs.begin("envmon", false);
  prefs.putBool("has", true);
  prefs.putFloat("temp", temp);
  prefs.putFloat("hum", hum);
  prefs.putFloat("pres", pres);
  prefs.putULong("epoch", timeSynced() ? (unsigned long)time(nullptr) : 0);
  prefs.putString("ts", currentTimestamp());
  prefs.end();
}

void pollSensor() {
  unsigned long now = millis();
  if (now - lastPollMs < SENSOR_READ_INTERVAL_MS) return;
  lastPollMs = now;

  if (!bmeOk) return;

  float t = bme.readTemperature();
  float h = bme.readHumidity();
  float p = bme.readPressure() / 100.0F;

  if (isnan(t) || isnan(h) || isnan(p)) return;

  temp = t;
  hum = h;
  pres = p;
  lastRead = now;
  hasReading = true;
  dataIsStale = false; 
}

void logCsv() {
  unsigned long now = millis();
  if (now - lastLogMs < CSV_LOG_INTERVAL_MS) return;
  lastLogMs = now;

  if (!hasReading) return;

  String ts = currentTimestamp();
  appendCsv(ts, temp, hum, pres);
  Serial.printf("[CSV] %s, %.2f C, %.2f %%, %.2f hPa\n", ts.c_str(), temp, hum, pres);

  if (!dataIsStale) {
    savePersistedReading();
  }
}

void setupRoutes() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", dashboardResolved);
  });

  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request) {
    String json = "{";
    if (hasReading) {
      json += "\"ok\":true,";
      json += "\"temperature\":" + String(temp, 2) + ",";
      json += "\"humidity\":" + String(hum, 2) + ",";
      json += "\"pressure\":" + String(pres, 2) + ",";
      json += "\"stale\":" + String(dataIsStale ? "true" : "false") + ",";

      if (dataIsStale) {
        if (savedEpoch > 0 && timeSynced()) {
          long secondsAgo = (long)time(nullptr) - (long)savedEpoch;
          if (secondsAgo < 0) secondsAgo = 0;
          json += "\"secondsAgo\":" + String(secondsAgo) + ",";
        } else {
          json += "\"secondsAgo\":-1,";
        }
        json += "\"timestamp\":\"" + savedTimestampStr + "\"";
      } else {
        unsigned long secondsAgo = (millis() - lastRead) / 1000;
        json += "\"secondsAgo\":" + String(secondsAgo) + ",";
        json += "\"timestamp\":\"" + currentTimestamp() + "\"";
      }
    } else {
      json += "\"ok\":false";
    }
    json += "}";
    request->send(200, "application/json", json);
  });

  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    bool sta = (WiFi.getMode() == WIFI_STA) && (WiFi.status() == WL_CONNECTED);
    String json = "{";
    json += "\"mode\":\"" + String(sta ? "STA" : "AP") + "\",";
    json += "\"ssid\":\"" + String(sta ? WiFi.SSID() : String(AP_SSID)) + "\",";
    json += "\"ip\":\"" + String(sta ? WiFi.localIP().toString() : WiFi.softAPIP().toString()) + "\",";
    if (sta) {
      json += "\"rssi\":" + String(WiFi.RSSI()) + ",";
    } else {
      json += "\"rssi\":null,";
    }
    json += "\"uptimeSec\":" + String(millis() / 1000);
    json += "}";
    request->send(200, "application/json", json);
  });

  server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!LittleFS.exists(CSV_FILE_PATH)) {
      request->send(404, "text/plain", "No log file yet.");
      return;
    }
    AsyncWebServerResponse *response = request->beginResponse(LittleFS, CSV_FILE_PATH, "text/csv");
    response->addHeader("Content-Disposition", "attachment; filename=\"sensor_log.csv\"");
    request->send(response);
  });

  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
  });
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("=== Environmental Monitor ===");

  loadPersistedReading();

  bmeOk = initBME();
  Serial.println(bmeOk ? "BME280 OK" : "BME280 NOT FOUND");

  initCsv();
  connectWifi();
  dashboardResolved = buildDashboardHtml();
  setupRoutes();
  server.begin();
  Serial.println("Server started");
}

void loop() {
  pollSensor();
  logCsv();
}