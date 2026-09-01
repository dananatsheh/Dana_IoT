#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"

#define POT_ADC_UNIT        ADC_UNIT_1
#define POT_ADC_CHANNEL     ADC_CHANNEL_6
#define IR_SENSOR_GPIO      GPIO_NUM_5
#define IR_LED_GPIO         GPIO_NUM_4
#define PWM_LED_GPIO        GPIO_NUM_2

#define LEDC_TIMER          LEDC_TIMER_0
#define LEDC_MODE            LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL         LEDC_CHANNEL_0
#define LEDC_DUTY_RES        LEDC_TIMER_10_BIT
#define LEDC_FREQUENCY       5000

#define SENSOR_PERIOD_MS     200
#define OUTPUT_RECEIVE_TIMEOUT_MS  300  
#define QUEUE_LENGTH         1    
#define SIMULATE_SLOW_CONSUMER  0

static const char *TAG_SENSOR = "SENSOR_TASK";
static const char *TAG_OUTPUT = "OUTPUT_TASK";
static const char *TAG_MAIN   = "APP_MAIN";

static adc_oneshot_unit_handle_t adc_handle;
static QueueHandle_t sensor_queue;

typedef struct {
    int  pot_raw;
    bool ir_state;
} sensor_msg_t;

static void sensor_task(void *pvParameters)
{
    TickType_t last_wake = xTaskGetTickCount();

    while (1) {
        sensor_msg_t msg;

        int raw = 0;
        adc_oneshot_read(adc_handle, POT_ADC_CHANNEL, &raw);
        msg.pot_raw = raw;
        msg.ir_state = (gpio_get_level(IR_SENSOR_GPIO) == 0);
        BaseType_t sent = xQueueSend(sensor_queue, &msg, 0);

        if (sent == pdPASS) {
            ESP_LOGI(TAG_SENSOR, "SENT   pot_raw=%d  ir_state=%d  tick=%lu",
                     msg.pot_raw, msg.ir_state, (unsigned long)xTaskGetTickCount());
        } else {
            ESP_LOGW(TAG_SENSOR, "DROPPED (queue full, depth=%d)  pot_raw=%d  ir_state=%d  tick=%lu",
                     QUEUE_LENGTH, msg.pot_raw, msg.ir_state, (unsigned long)xTaskGetTickCount());
        }

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(SENSOR_PERIOD_MS));
    }
}

static void output_task(void *pvParameters)
{
    sensor_msg_t msg;
    sensor_msg_t last_msg = { .pot_raw = 0, .ir_state = false };
    bool have_data = false;

    while (1) {
        BaseType_t got = xQueueReceive(sensor_queue, &msg,
                                        pdMS_TO_TICKS(OUTPUT_RECEIVE_TIMEOUT_MS));

        if (got == pdPASS) {
            last_msg = msg;
            have_data = true;

            int duty = msg.pot_raw >> 2;
            ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty);
            ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
            gpio_set_level(IR_LED_GPIO, msg.ir_state ? 1 : 0);

            ESP_LOGI(TAG_OUTPUT, "RECV   duty=%d  ir_led=%d  tick=%lu",
                     duty, msg.ir_state, (unsigned long)xTaskGetTickCount());
        } else {
            ESP_LOGW(TAG_OUTPUT, "TIMEOUT no new data (have_data=%d) - reapplying last known duty=%d ir_led=%d  tick=%lu",
                     have_data, last_msg.pot_raw >> 2, last_msg.ir_state,
                     (unsigned long)xTaskGetTickCount());
        }

#if SIMULATE_SLOW_CONSUMER
        vTaskDelay(pdMS_TO_TICKS(400));
#endif
    }
}

static void init_peripherals(void)
{
    adc_oneshot_unit_init_cfg_t init_cfg = { .unit_id = POT_ADC_UNIT };
    adc_oneshot_new_unit(&init_cfg, &adc_handle);

    adc_oneshot_chan_cfg_t chan_cfg = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten    = ADC_ATTEN_DB_12,
    };
    adc_oneshot_config_channel(adc_handle, POT_ADC_CHANNEL, &chan_cfg);

    gpio_config_t ir_in_cfg = {
        .pin_bit_mask = 1ULL << IR_SENSOR_GPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    gpio_config(&ir_in_cfg);

    gpio_config_t ir_led_cfg = {
        .pin_bit_mask = 1ULL << IR_LED_GPIO,
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&ir_led_cfg);

    ledc_timer_config_t timer_cfg = {
        .speed_mode      = LEDC_MODE,
        .timer_num       = LEDC_TIMER,
        .duty_resolution = LEDC_DUTY_RES,
        .freq_hz         = LEDC_FREQUENCY,
        .clk_cfg         = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&timer_cfg);

    ledc_channel_config_t channel_cfg = {
        .gpio_num   = PWM_LED_GPIO,
        .speed_mode = LEDC_MODE,
        .channel    = LEDC_CHANNEL,
        .timer_sel  = LEDC_TIMER,
        .duty       = 0,
        .hpoint     = 0,
    };
    ledc_channel_config(&channel_cfg);
}

void app_main(void)
{
    ESP_LOGI(TAG_MAIN, "Task 4.2 starting - init peripherals + queue (depth=%d)", QUEUE_LENGTH);
    init_peripherals();

    sensor_queue = xQueueCreate(QUEUE_LENGTH, sizeof(sensor_msg_t));
    if (sensor_queue == NULL) {
        ESP_LOGE(TAG_MAIN, "Failed to create sensor_queue - halting");
        while (1) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }


    xTaskCreate(sensor_task, "SensorTask", 4096, NULL, 3, NULL);
    xTaskCreate(output_task, "OutputTask", 4096, NULL, 4, NULL);

    ESP_LOGI(TAG_MAIN, "Both tasks created - data now flows only through the queue");

    char stats_buf[1024];
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(5000));
#if CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS
        vTaskGetRunTimeStats(stats_buf);
        ESP_LOGI(TAG_MAIN, "\nTask\t\tAbs Time\t%% Time\n%s", stats_buf);
#endif
    }
}