#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define BLINK_GPIO 18

static const char *TAG = "BlinkExample";

void app_main(void)
{
    gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

    bool led_state = false;
    ESP_LOGI(TAG, "ESP32 Blink on Pin 18 Started!");

    while (1) {
        led_state = !led_state;
        gpio_set_level(BLINK_GPIO, led_state);

        if (led_state) {
            ESP_LOGI(TAG, "Pin 18 is HIGH (LED ON)");
        } else {
            ESP_LOGI(TAG, "Pin 18 is LOW (LED OFF)");
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
