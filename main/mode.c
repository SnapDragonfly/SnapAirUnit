
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

static uint16_t g_sw_mode              = SW_MODE_WIFI_AP;
static enum_state_t g_sw_state         = SW_STATE_INVALID;
static bool g_debug_mode               = false;

esp_err_t snap_sw_debug_set(int mode)
{
    if (1 == mode){
        g_debug_mode = true;
    } else if(0 == mode){
        g_debug_mode = false;
    } else{
        return ESP_FAIL;
    }

    return ESP_OK;
}

bool snap_sw_debug_get(void)
{
    return g_debug_mode;
}

void snap_sw_state_set(enum_state_t state)
{
    g_sw_state = state;
}

void snap_sw_state_degrade(enum_state_t state)
{
    if (g_sw_state > state){
#if (DEBUG_MODE)
        ESP_LOGI(MODULE_MODE, "degrade from %d to %d", g_sw_state, state);
#endif /* DEBUG_MODE */
        g_sw_state = state;
    } 
#if (DEBUG_MODE)
    else {
        ESP_LOGI(MODULE_MODE, "degrade ignore %d, hold %d", state, g_sw_state);
    }
#endif /* DEBUG_MODE */
}

void snap_sw_state_upgrade(enum_state_t state)
{
    if (g_sw_state < state){
#if (DEBUG_MODE)
        ESP_LOGI(MODULE_MODE, "upgrade from %d to %d", g_sw_state, state);
#endif /* DEBUG_MODE */
        g_sw_state = state;
    } 
#if (DEBUG_MODE)
    else {
        ESP_LOGI(MODULE_MODE, "upgrade ignore %d, hold %d", state, g_sw_state);
    }
#endif /* DEBUG_MODE */
}


bool snap_sw_state_active(enum_mode_t mode)
{
    return g_sw_state > SW_STATE_HALF_DUPLEX && g_sw_mode == mode;
}

enum_state_t snap_sw_state_get(void)
{
    return g_sw_state;
}

void snap_sw_mode_set(enum_mode_t mode)
{
    g_sw_mode = mode;
}

enum_mode_t snap_sw_mode_next(void)
{
    return (g_sw_mode + 1) % SW_MODE_NULL;
}

enum_mode_t snap_sw_mode_get(void)
{
    return g_sw_mode;
}

esp_err_t snap_sw_mode_switch(enum_mode_t mode)
{
#if (DEBUG_MODE)
    ESP_LOGI(MODULE_MODE, "current mode(%d), set to %d, actually %d", g_sw_state, mode, mode % SW_MODE_NULL);
#endif /* DEBUG_MODE */

    /* Make sure it's NOT overflow! */
    mode = mode % SW_MODE_NULL;

    /* Check if it's necessary to change mode */
    if (mode == g_sw_mode)
        return ESP_OK;

    /* Stop previous modes */
    switch(g_sw_mode){
        case SW_MODE_WIFI_AP:
            snap_sw_state_set(SW_STATE_INVALID);
            wifi_stop_softap();
            break;

        case SW_MODE_WIFI_STA:
            snap_sw_state_set(SW_STATE_INVALID);
            wifi_stop_sta();
            break;

        case SW_MODE_BT_SPP:
            snap_sw_state_set(SW_STATE_INVALID);
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

void snap_sw_mode_init(void)
{
    /*
     * All service module need default event loop
     * ToDo:
     * Please check with ESP mannuals
     */
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /*
     * Application & service mode
     * ToDO:
     * Please set to AP by Spec, currently used for test(STA)
     */
    g_sw_mode = SW_MODE_WIFI_STA;
    ESP_ERROR_CHECK(esp_netif_init());
    //esp_netif_create_default_wifi_ap();

    ESP_LOGI(MODULE_MODE, "mode_init");
}

