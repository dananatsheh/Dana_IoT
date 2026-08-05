#ifndef CONFIG_H
#define CONFIG_H

#define WIFI_SSID       "Dana"
#define WIFI_PASSWORD   "meuisshit55"

#define MQTT_BROKER     "broker.emqx.io"
#define MQTT_PORT       1883
#define MQTT_USERNAME   ""   
#define MQTT_PASSWORD   ""

#define MY_ID           "studentB"
#define PARTNER_ID      "studentA"


#define MY_TOPIC        "sensor/control/cyber"   
#define PARTNER_TOPIC   "sensor/data/cyber"      

#define TX_LED          16   
#define MOTOR_LED       19   
#define IR_LED          18   
#define RX_LED          17   

#define IR_PIN          34   
#define MOTOR_IN1       26   
#define MOTOR_IN2       27    
#define MOTOR_ENA       25   

#define IR_DEBOUNCE_MS       500   
#define LINK_TIMEOUT_MS      2000  
#define PUBLISH_INTERVAL_MS  200   
#define MQTT_RECONNECT_MS    3000  

#endif