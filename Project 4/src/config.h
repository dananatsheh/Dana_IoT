#ifndef CONFIG_H
#define CONFIG_H

#define WIFI_SSID       "Dana"
#define WIFI_PASSWORD   "password44"

#define AP_SSID         "ESP32-EnviroMonitor"
#define AP_PASSWORD     "monitor123"

#define WIFI_CONNECT_TIMEOUT_MS  15000

#define I2C_SDA_PIN     21
#define I2C_SCL_PIN     22
#define BME280_ADDR_1   0x76
#define BME280_ADDR_2   0x77

#define SENSOR_READ_INTERVAL_MS   2000
#define CSV_LOG_INTERVAL_MS       10000

#define TEMP_GREEN_MIN   18.0
#define TEMP_GREEN_MAX   28.0
#define TEMP_YELLOW_MIN  15.0
#define TEMP_YELLOW_MAX  32.0
#define TEMP_RED_MIN     10.0
#define TEMP_RED_MAX     35.0

#define HUM_GREEN_MIN    30.0
#define HUM_GREEN_MAX    60.0
#define HUM_YELLOW_MIN   20.0
#define HUM_YELLOW_MAX   70.0
#define HUM_RED_MIN      10.0
#define HUM_RED_MAX      80.0

// Rebased around a ~895 hPa local baseline (Amman altitude) instead of
// sea-level ~1013 hPa - adjust if you relocate or calibrate further.
#define PRES_GREEN_MIN   880.0
#define PRES_GREEN_MAX   910.0
#define PRES_YELLOW_MIN  860.0
#define PRES_YELLOW_MAX  930.0
#define PRES_RED_MIN     850.0
#define PRES_RED_MAX     940.0

#define CSV_FILE_PATH      "/data.csv"
#define CSV_HEADER         "timestamp,temperature,humidity,pressure"
#define CSV_MAX_SIZE_BYTES (256 * 1024)

#endif