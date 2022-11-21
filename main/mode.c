
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "time.h"
#include "sys/time.h"
#include "sdkconfig.h"
#include "esp_wifi.h"
#include "esp_spp_api.h"
#include "esp_check.h"
#include "esp_bt.h"

#include "module.h"
#include "define.h"

#include "softap.h"
#include "station.h"
#include "btspp.h"
#include "mode.h"
#include "key.h"

uint16_t      g_sw_mode           = SW_MODE_WIFI_AP;

uint16_t snap_sw_mode_next(void)
{
    return (g_sw_mode + 1) % SW_MODE_NULL;
}

uint16_t snap_sw_mode_get(void)
{
    return g_sw_mode;
}

esp_err_t snap_sw_mode_switch(uint16_t mode)
{
    /* Make sure it's NOT overflow! */
    mode = mode % SW_MODE_NULL;

    /* Check if it's necessary to change mode */
    if (mode == g_sw_mode)
        return ESP_OK;

    /* Stop previous modes */
    switch(g_sw_mode){
        case SW_MODE_WIFI_AP:
            wifi_stop_softap();
            break;

        case SW_MODE_WIFI_STA:
            wifi_stop_sta();
            break;

        case SW_MODE_BT_SPP:
            bt_deinit_spp();
            break;

        default:
            break;
        }

    /* Start new mode */
    switch(mode){
        case SW_MODE_WIFI_AP:
            wifi_init_softap();
            break;

        case SW_MODE_WIFI_STA:
            wifi_init_sta();
            break;

        case SW_MODE_BT_SPP:
            bt_init_spp();
            break;

        default:
            break;
        }

    g_sw_mode = mode;

    return ESP_OK;
}

esp_err_t snap_sw_mode_start(TaskFunction_t pxTaskCode, bool task)
{
    if(!pxTaskCode){
        return ESP_FAIL;
    }
    
    mode_key_lock();
    
    if (task){
        xTaskCreate(pxTaskCode, MODULE_MODE, TASK_LARGE_BUFFER, NULL, uxTaskPriorityGet(NULL), NULL);
    }else{
        pxTaskCode(NULL);
    }

    mode_key_unlock();

    return ESP_OK;
}

void snap_sw_mode_init(void)
{
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ttl_uart_init();
    g_sw_mode = SW_MODE_WIFI_STA;
    ESP_ERROR_CHECK(esp_netif_init());
    //esp_netif_create_default_wifi_ap();
    wifi_init_sta();

    ESP_LOGI(MODULE_MODE, "mode_init");
}




