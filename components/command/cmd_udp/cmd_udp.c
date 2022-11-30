

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "esp_err.h"
#include "esp_log.h"
#include "cmd_udp.h"

#include "module.h"
#include "mode.h"
#include "define.h"

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


