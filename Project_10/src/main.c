#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "driver/ledc.h"
#include "esp_log.h"

static const char *TAG = "TASK3_4";

#define IR_LED_PIN          GPIO_NUM_4
#define IR_SENSOR_PIN       GPIO_NUM_5
#define IR_ACTIVE_LEVEL     0

#define PWM_LED_PIN         GPIO_NUM_2
#define LEDC_TIMER          LEDC_TIMER_0
#define LEDC_MODE            LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL         LEDC_CHANNEL_0
#define LEDC_DUTY_RES         LEDC_TIMER_13_BIT
#define LEDC_FREQUENCY        5000

#define POT_ADC_UNIT         ADC_UNIT_1
#define POT_ADC_CHANNEL      ADC_CHANNEL_6
#define POT_ADC_ATTEN        ADC_ATTEN_DB_12
#define POT_ADC_BITWIDTH     ADC_BITWIDTH_12
#define POT_SAMPLES          16

static void gpio_setup(void)
{
    gpio_set_direction(IR_LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(IR_LED_PIN, 0);

    gpio_set_direction(IR_SENSOR_PIN, GPIO_MODE_INPUT);
}

static adc_oneshot_unit_handle_t adc_setup(void)
{
    adc_oneshot_unit_handle_t adc1_handle;

    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = POT_ADC_UNIT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc1_handle));

    adc_oneshot_chan_cfg_t chan_config = {
        .bitwidth = POT_ADC_BITWIDTH,
        .atten    = POT_ADC_ATTEN,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, POT_ADC_CHANNEL, &chan_config));

    return adc1_handle;
}

static void pwm_setup(void)
{
    ledc_timer_config_t timer_conf = {
        .speed_mode      = LEDC_MODE,
        .timer_num       = LEDC_TIMER,
        .duty_resolution = LEDC_DUTY_RES,
        .freq_hz         = LEDC_FREQUENCY,
        .clk_cfg         = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer_conf));

    ledc_channel_config_t channel_conf = {
        .gpio_num   = PWM_LED_PIN,
        .speed_mode = LEDC_MODE,
        .channel    = LEDC_CHANNEL,
        .timer_sel  = LEDC_TIMER,
        .duty       = 0,
        .hpoint     = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&channel_conf));
}

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing peripherals...");

    gpio_setup();
    adc_oneshot_unit_handle_t adc1_handle = adc_setup();
    pwm_setup();

    ESP_LOGI(TAG, "Setup complete. Entering main loop.");

    while (1) {
        int sum = 0;
        for (int i = 0; i < POT_SAMPLES; i++) {
            int sample = 0;
            ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, POT_ADC_CHANNEL, &sample));
            sum += sample;
        }
        int raw = sum / POT_SAMPLES;

        uint32_t duty = (raw * 8191) / 4095;

        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty);
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);

        int ir_state = gpio_get_level(IR_SENSOR_PIN);
        bool object_detected = (ir_state == IR_ACTIVE_LEVEL);
        gpio_set_level(IR_LED_PIN, object_detected ? 1 : 0);

        ESP_LOGI(TAG, "Pot raw=%d duty=%lu | IR=%d -> LED=%s",
                 raw, (unsigned long)duty, ir_state, object_detected ? "ON" : "OFF");

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}