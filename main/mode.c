/// @file mode.c

/*
 * idf header files
 */
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
#include "esp_spp_api.h"
#include "esp_check.h"
#include "esp_bt.h"
#include "sdkconfig.h"

/*
 * basic header files
 */
#include "config.h"
#include "define.h"
#include "handle.h"

/*
 * module header files
 */
#include "module.h"
#include "mode.h"
#include "key.h"
#include "cmd_nvs.h"

/*
 * service header files
 */
#include "softap.h"
#include "station.h"
#include "btspp.h"

static uint16_t     g_sw_mode              = SW_MODE_WIFI_AP;
static enum_protocol_state_t g_sw_state    = SW_STATE_INVALID;
static bool         g_command_mode         = false;

esp_err_t snap_sw_command_set(int mode)
{
    if (1 == mode){
        g_command_mode = true;
    } else if(0 == mode){
        g_command_mode = false;
    } else{
        return ESP_FAIL;
    }

    return ESP_OK;
}

bool snap_sw_command_get(void)
{
    return g_command_mode;
}

void protocol_state_set(enum_protocol_state_t state)
{
    g_sw_state = state;
}

void protocol_state_degrade(enum_protocol_state_t state)
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

void protocol_state_upgrade(enum_protocol_state_t state)
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


bool protocol_state_active(enum_wireless_mode_t mode)
{
    return g_sw_state > SW_STATE_HALF_DUPLEX && g_sw_mode == mode;
}

enum_protocol_state_t protocol_state_get(void)
{
    return g_sw_state;
}

void wireless_mode_set(enum_wireless_mode_t mode)
{
    g_sw_mode = mode;
}

enum_wireless_mode_t wireless_mode_next(void)
{
#if (WIRELESS_SIMPLIFIED)
    uint16_t mode;
    switch(g_sw_mode){
        case SW_MODE_WIFI_AP:
            mode = SW_MODE_BT_SPP;
            break;
            
        case SW_MODE_BT_SPP:
            mode = SW_MODE_WIFI_AP;
            break;
            
        case SW_MODE_WIFI_STA:
        default:
            mode = SW_MODE_WIFI_AP;
            break;
    }
    return mode;
#else
    return (g_sw_mode + 1) % SW_MODE_NULL;
#endif
}

enum_wireless_mode_t wireless_mode_get(void)
{
    return g_sw_mode;
}

esp_err_t wireless_mode_switch(enum_wireless_mode_t mode)
{
    /* Make sure it's NOT overflow! */
    mode = mode % SW_MODE_NULL;

    /* Check if it's necessary to change mode */
    if (mode == g_sw_mode)
        return ESP_OK;

    /* Stop previous modes */
    switch(g_sw_mode){
        case SW_MODE_WIFI_AP:
            protocol_state_set(SW_STATE_INVALID);
            wifi_ap_stop();
            break;

        case SW_MODE_WIFI_STA:
            protocol_state_set(SW_STATE_INVALID);
            wifi_sta_stop();
            break;

        case SW_MODE_BT_SPP:
            protocol_state_set(SW_STATE_INVALID);
            bt_spp_stop();
            break;

        default:
            break;
        }

    /* Start new mode */
#if ( defined(PASS_THROUGH_UART) && (ESP_RF_MODE == 2) ) // SW_MODE_BT_SPP
    switch(mode){
        case SW_MODE_WIFI_AP:
            wifi_ap_start();
            break;

        case SW_MODE_WIFI_STA:
            mode = SW_MODE_BT_SPP;
            /* fall through */

        case SW_MODE_BT_SPP:
            bt_spp_start();
            break;

        default:
            break;
        }
#else
    switch(mode){
        case SW_MODE_WIFI_AP:
            wifi_ap_start();
            break;

        case SW_MODE_WIFI_STA:
            wifi_sta_start();
            break;

        case SW_MODE_BT_SPP:
            bt_spp_start();
            break;

        default:
            break;
        }
#endif

#if (DEBUG_MODE)
    ESP_LOGI(MODULE_MODE, "current mode(%d), set to %d, actually %d", g_sw_state, mode, mode % SW_MODE_NULL);
#endif /* DEBUG_MODE */

    g_sw_mode = mode;
    (void)led_blink_set((struct blink_led *)g_mode_handle, g_sw_mode);
    (void)led_blink_set((struct blink_led *)g_status_handle, LED_STATUS_SLOW);
    (void)nvs_set_wireless_mode(g_sw_mode);

    return ESP_OK;
}


esp_err_t wireless_mode_init(void)
{
    /*
     * Application & service mode
     * ToDO:
     * Please set to AP by Spec, currently used for test(STA)
     */
    uint16_t mode;
    esp_err_t err = nvs_get_wireless_mode(&mode);
    if(ESP_OK != err){
        g_sw_mode = ESP_RF_MODE;
        (void)nvs_set_wireless_mode(g_sw_mode);
    } else {
        g_sw_mode = mode;
    }

    switch(g_sw_mode){
        case SW_MODE_WIFI_AP:
            wifi_ap_start();
            break;

        case SW_MODE_WIFI_STA:
            wifi_sta_start();
            break;

        case SW_MODE_BT_SPP:
            bt_spp_start();
            break;

        default:
            g_sw_mode = SW_MODE_WIFI_AP;
            (void)nvs_set_wireless_mode(g_sw_mode);
            wifi_ap_start();
            break;
    }

    (void)led_blink_set((struct blink_led *)g_mode_handle, g_sw_mode);
    (void)led_blink_set((struct blink_led *)g_status_handle, LED_STATUS_SLOW);

    ESP_LOGI(MODULE_MODE, "mode_init err = %d, mode = %d, g_sw_mode = %d", err, mode, g_sw_mode);
    return ESP_OK;
}


