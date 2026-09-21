#ifndef CONFIG_H
#define CONFIG_H

#define WIFI_SSID       "Dana"
#define WIFI_PASSWORD   "password"

#define MQTT_BROKER          "192.168.1.33"
#define MQTT_PORT            1883
#define MQTT_USERNAME        "dana"
#define MQTT_PASSWORD        "eng008"
#define MQTT_CLIENT_ID_PREFIX "esp32-task27-"
#define MQTT_RECONNECT_MS    5000

#define TOPIC_SENSOR_TEMPERATURE   "sensor/bme280/temperature"
#define TOPIC_SENSOR_HUMIDITY      "sensor/bme280/humidity"
#define TOPIC_SENSOR_PRESSURE      "sensor/bme280/pressure"

#define TOPIC_MOTOR_STATUS_STATE      "motor/status/state"
#define TOPIC_MOTOR_STATUS_DIRECTION  "motor/status/direction"
#define TOPIC_MOTOR_STATUS_SPEED      "motor/status/speed"

#define TOPIC_MOTOR_COMMAND_STATE      "motor/command/state"
#define TOPIC_MOTOR_COMMAND_DIRECTION  "motor/command/direction"
#define TOPIC_MOTOR_COMMAND_SPEED      "motor/command/speed"

#define SENSOR_READ_INTERVAL_MS   2500
#define SENSOR_STALE_MS           10000

#define MOTOR_STATUS_PUBLISH_INTERVAL_MS 1000

#define MOTOR_PIN_IN1   26
#define MOTOR_PIN_IN2   27
#define MOTOR_PIN_PWM   25
#define MOTOR_PWM_CHANNEL         0
#define MOTOR_PWM_FREQ_HZ         20000
#define MOTOR_PWM_RESOLUTION_BITS 8

#endif
