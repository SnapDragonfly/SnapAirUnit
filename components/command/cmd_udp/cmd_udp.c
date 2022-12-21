/// @file cmd_udp.c

/*
 * idf header files
 */
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "esp_err.h"
#include "esp_log.h"

/*
 * basic header files
 */
#include "config.h"
#include "define.h"


/*
 * module header files
 */
#include "module.h"
#include "mode.h"
#include "cmd_udp.h"
#include "util.h"
#include "factory_setting.h"
#include "msp_protocol.h"
#include "factory_setting.h"

/*
 * service header files
 */
#include "udp_server.h"

esp_err_t udp_bluetooth(struct udp_data * data)
{
    UNUSED(data);
    return snap_sw_mode_switch(SW_MODE_BT_SPP);
}

esp_err_t udp_ap(struct udp_data * data)
{
    esp_err_t err;

    if(NULL == data){
        return ESP_FAIL;
    }

    if(data->counts != 2){
        return ESP_FAIL;
    }
    
    err = set_sta_settings(data->param[0], data->param[1]);
    if (err != ESP_OK) {
        ESP_LOGE(MODULE_CMD_UDP, "%s", esp_err_to_name(err));
        return err;
    }

    err = snap_sw_mode_switch(SW_MODE_WIFI_STA);
    if (err != ESP_OK) {
        ESP_LOGE(MODULE_CMD_UDP, "%s", esp_err_to_name(err));
        return err;
    }

    return err;
}

esp_err_t udp_wifi(struct udp_data * data)
{
    esp_err_t err;
    
    if(NULL == data){
        return ESP_FAIL;
    }

    if(data->counts != 2){
        return ESP_FAIL;
    }

    err = set_ap_settings(data->param[0], data->param[1]);
    if (err != ESP_OK) {
        ESP_LOGE(MODULE_CMD_UDP, "%s", esp_err_to_name(err));
        return err;
    }

    err = snap_sw_mode_switch(SW_MODE_WIFI_AP);
    if (err != ESP_OK) {
        ESP_LOGE(MODULE_CMD_UDP, "%s", esp_err_to_name(err));
        return err;
    }

    return err;
}

esp_err_t udp_sdk(struct udp_data * data)
{
    UNUSED(data);

    udp_send_msg((uint8_t *)get_app_versions(), strlen(get_idf_versions()));
    return ESP_ERR_INVALID_RESPONSE;
}

esp_err_t udp_arm(struct udp_data * data)
{
    UNUSED(data);

    mspSetChannel(4, 1900);
    mspUpdateChannels();

    return ESP_OK;
}

esp_err_t udp_emergency(struct udp_data * data)
{
    UNUSED(data);

    mspSetChannel(4, 1200);
    mspUpdateChannels();

    return ESP_OK;
}

esp_err_t udp_debug(struct udp_data * data)
{
    if(NULL == data){
        return ESP_FAIL;
    }

    if(data->counts != 1){
        return ESP_FAIL;
    }

    return snap_sw_command_set(atoi(data->param[0]));
}

esp_err_t udp_reboot(struct udp_data * data)
{
    snap_reboot(3);
    return ESP_OK;
}

esp_err_t udp_command(struct udp_data * data)
{
    snap_sw_command_set(true);
    return ESP_OK;
}

esp_err_t udp_exit(struct udp_data * data)
{
    snap_sw_command_set(false);
    return ESP_OK;
}




