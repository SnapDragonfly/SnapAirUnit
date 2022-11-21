
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "sdkconfig.h"

#include "key.h"
#include "module.h"
#include "define.h"
#include "process.h"
#include "handle.h"

#if (DEBUG_KEY_MODE)
static int64_t key_press_time = 0;
#endif /* DEBUG_KEY_MODE */

static bool   g_key_filter    = false;

void mode_key_lock(void)
{
    g_key_filter = true;
}

void mode_key_unlock(void)
{
    g_key_filter = false;
}

void mode_key_long_press(void* arg)
{
#if (DEBUG_KEY_MODE)
    int64_t curr_time;

    curr_time = esp_timer_get_time();
    ESP_LOGI(MODULE_KEY_MODE, "%s %lld", arg, TIME_DIFF_IN_MS(key_press_time, curr_time));
#endif /* DEBUG_KEY_MODE */

    if(g_key_filter){
        ESP_LOGI(MODULE_KEY_MODE, "%s filter", arg);
        return;
    }

    ESP_ERROR_CHECK(esp_event_post(EVT_PROCESS, MODE_KEY_LONG_PRESSED, NULL, 0, portMAX_DELAY));
}

void mode_key_pressed(void* arg)
{
#if (DEBUG_KEY_MODE)
    key_press_time = esp_timer_get_time();
    ESP_LOGI(MODULE_KEY_MODE, "%s %lld", arg, key_press_time);
#endif /* DEBUG_KEY_MODE */

    if(g_key_filter){
        ESP_LOGI(MODULE_KEY_MODE, "%s filter", arg);
        return;
    }

    ESP_ERROR_CHECK(esp_event_post_to(g_evt_handle, EVT_PROCESS, MODE_KEY_SHORT_PRESSED, NULL, 0, portMAX_DELAY));
    ESP_ERROR_CHECK(esp_event_post(EVT_PROCESS, MODE_KEY_SHORT_PRESSED, NULL, 0, portMAX_DELAY));
}

void mode_key_released(void* arg)
{
#if (DEBUG_KEY_MODE)
    int64_t curr_time;

    curr_time = esp_timer_get_time();
    ESP_LOGI(MODULE_KEY_MODE, "%s %lld", arg, TIME_DIFF_IN_MS(key_press_time, curr_time));
#endif /* DEBUG_KEY_MODE */

    if(g_key_filter){
        ESP_LOGI(MODULE_KEY_MODE, "%s filter", arg);
        return;
    }

    ESP_ERROR_CHECK(esp_event_post(EVT_PROCESS, MODE_KEY_RELEASED, NULL, 0, portMAX_DELAY));
}

button_handle_t module_key_start(uint8_t num)
{
    button_handle_t key;
    uint8_t io_level;

    io_level = gpio_get_level(num);
    key      = iot_button_create(num, CONFIG_KEY_ACTIVE);
    if (CONFIG_KEY_ACTIVE == io_level){
        ESP_LOGW(MODULE_KEY_MODE, "Mode Key is PRESSED!!!");
    }

    ESP_ERROR_CHECK(iot_button_add_custom_cb(key, CONFIG_KEY_LONG_PRESS, mode_key_long_press, "long"));
    ESP_ERROR_CHECK(iot_button_set_evt_cb(key, BUTTON_CB_PUSH, mode_key_pressed, "press"));
    ESP_ERROR_CHECK(iot_button_set_evt_cb(key, BUTTON_CB_RELEASE, mode_key_released, "release"));

    ESP_LOGI(MODULE_KEY_MODE, "OK!");

    return key;
}

