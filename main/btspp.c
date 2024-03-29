/// @file btspp.c

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
#include "nvs.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "esp_bt_device.h"
#include "esp_spp_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_system.h"
#include "time.h"
#include "sys/time.h"
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
#include "msp_protocol.h"
#include "tello_protocol.h"

/*
 * service header files
 */
#include "ttl.h"
#include "btspp.h"

static struct timeval g_time_new;
static struct timeval g_time_old;

static long  g_data_num                    = 0;
uint32_t     g_esp_ssp_handle              = 0;

static const esp_spp_mode_t g_esp_spp_mode   = ESP_SPP_MODE_CB;
static const esp_spp_sec_t g_sec_mask      = ESP_SPP_SEC_AUTHENTICATE;
static const esp_spp_role_t g_role_slave     = ESP_SPP_ROLE_SLAVE;

static char *bda2str(uint8_t * bda, char *str, size_t size)
{
    if (bda == NULL || str == NULL || size < 18) {
        return NULL;
    }

    uint8_t *p = bda;
    sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x",
            p[0], p[1], p[2], p[3], p[4], p[5]);
    return str;
}

static void print_speed(void)
{
    float time_old_s = g_time_old.tv_sec + g_time_old.tv_usec / 1000000.0;
    float time_new_s = g_time_new.tv_sec + g_time_new.tv_usec / 1000000.0;
    float time_interval = time_new_s - time_old_s;
    float speed = g_data_num * 8 / time_interval / 1000.0;
    ESP_LOGI(MODULE_BT_SPP, "speed(%fs ~ %fs): %f kbit/s" , time_old_s, time_new_s, speed);
    g_data_num = 0;
    g_time_old.tv_sec = g_time_new.tv_sec;
    g_time_old.tv_usec = g_time_new.tv_usec;
}

static void esp_spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
    char bda_str[18] = {0};

    switch (event) {
    case ESP_SPP_INIT_EVT:
        if (param->init.status == ESP_SPP_SUCCESS) {
#if (DEBUG_BT_SPP)
            ESP_LOGI(MODULE_BT_SPP, "ESP_SPP_INIT_EVT");
#endif /* DEBUG_BT_SPP */
            esp_spp_start_srv(g_sec_mask, g_role_slave, 0, SPP_SERVER_NAME);
        } 
#if (DEBUG_BT_SPP)
        else {
            ESP_LOGE(MODULE_BT_SPP, "ESP_SPP_INIT_EVT status:%d", param->init.status);
        }
#endif /* DEBUG_BT_SPP */
        break;
        
    case ESP_SPP_DISCOVERY_COMP_EVT:
#if (DEBUG_BT_SPP)
        ESP_LOGI(MODULE_BT_SPP, "ESP_SPP_DISCOVERY_COMP_EVT");
#endif /* DEBUG_BT_SPP */
        break;
    
    case ESP_SPP_OPEN_EVT:
#if (DEBUG_BT_SPP)
        ESP_LOGI(MODULE_BT_SPP, "ESP_SPP_OPEN_EVT");
#endif /* DEBUG_BT_SPP */
        protocol_state_upgrade(SW_STATE_FULL_DUPLEX);
        (void)led_blink_set((struct blink_led *)g_status_handle, LED_STATUS_ON);
        break;
    
    case ESP_SPP_CLOSE_EVT:
        protocol_state_set(SW_STATE_INVALID);
#if (DEBUG_BT_SPP)
        ESP_LOGI(MODULE_BT_SPP, "ESP_SPP_CLOSE_EVT status:%d handle:%d close_by_remote:%d", param->close.status,
                 param->close.handle, param->close.async);
#endif /* DEBUG_BT_SPP */
        break;
        
    case ESP_SPP_START_EVT:
        if (param->start.status == ESP_SPP_SUCCESS) {
#if (DEBUG_BT_SPP)
            ESP_LOGI(MODULE_BT_SPP, "ESP_SPP_START_EVT handle:%d sec_id:%d scn:%d", param->start.handle, param->start.sec_id,
                     param->start.scn);
#endif /* DEBUG_BT_SPP */
            esp_bt_dev_set_device_name(SPP_DEVICE_NAME);
            esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
        } 
#if (DEBUG_BT_SPP)
        else {
            ESP_LOGE(MODULE_BT_SPP, "ESP_SPP_START_EVT status:%d", param->start.status);
        }
#endif /* DEBUG_BT_SPP */
        break;
        
    case ESP_SPP_CL_INIT_EVT:
#if (DEBUG_BT_SPP)
        ESP_LOGI(MODULE_BT_SPP, "ESP_SPP_CL_INIT_EVT");
#endif /* DEBUG_BT_SPP */
        break;
    
    case ESP_SPP_DATA_IND_EVT:
#if (DEBUG_BT_SPP)
        ESP_LOGI(MODULE_BT_SPP, "ESP_SPP_DATA_IND_EVT len:%d handle:%d",
                 param->data_ind.len, param->data_ind.handle);
        if (param->data_ind.len < STR_BUFFER_LEN) {
            esp_log_buffer_hex(MODULE_BT_SPP, param->data_ind.data, param->data_ind.len);
        }else{
            gettimeofday(&g_time_new, NULL);
            g_data_num += param->data_ind.len;
            if (g_time_new.tv_sec - g_time_old.tv_sec >= 3) {
                print_speed();
            }
        }
#else
        UNUSED(print_speed);
#endif /* DEBUG_BT_SPP */

        if(protocol_state_active(SW_MODE_BT_SPP)){

#if 1
#if (DEBUG_BT_SPP)
            ESP_LOGI(MODULE_BT_SPP, "ESP_SPP_DATA_IND_EVT ttl_msg_send");
#endif /* DEBUG_BT_SPP */
            ESP_ERROR_CHECK(ttl_msg_send(param->data_ind.data, param->data_ind.len));
#else
            protocol_state_upgrade(SW_STATE_FULL_DUPLEX);
            //wireless_handle_msp(param->data_ind.data, param->data_ind.len);
            esp_err_t ret;
            switch(protocol_state_get()){
                case SW_STATE_FULL_DUPLEX:
#if (DEBUG_BT_SPP)
                    ESP_LOGI(MODULE_BT_SPP, "spp received %d bytes", param->data_ind.len);
                    esp_log_buffer_hex(MODULE_BT_SPP, param->data_ind.data, param->data_ind.len);
#endif /* DEBUG_BT_SPP */

                    ret = wireless_handle_msp(param->data_ind.data, param->data_ind.len);
                    if(ESP_OK == ret){
                        break;
                    }

                    /* FALL THROUGH */

                case SW_STATE_TELLO:
                    protocol_state_upgrade(SW_STATE_TELLO);
#if (DEBUG_BT_SPP)
                    ESP_LOGI(MODULE_BT_SPP, "tello received %d bytes", param->data_ind.len);
                    ESP_LOGI(MODULE_BT_SPP, "%s", param->data_ind.data);
#endif /* DEBUG_BT_SPP */

                    ret = nomsp_handle_tello(param->data_ind.data, param->data_ind.len);
                    if(ESP_ERR_NOT_SUPPORTED == ret){
                        protocol_state_upgrade(SW_STATE_CLI);
                        //vTaskDelay(TIME_50_MS / portTICK_PERIOD_MS);
                        ESP_ERROR_CHECK(ttl_msg_send(param->data_ind.data, param->data_ind.len));
                    } else if(ESP_OK != ret){
                        protocol_state_degrade(SW_STATE_FULL_DUPLEX);
                    }

                    break;

                case SW_STATE_CLI:
                    ret = nomsp_handle_cli(param->data_ind.data, param->data_ind.len);
                    if(ESP_OK != ret){
                        protocol_state_set(SW_STATE_FULL_DUPLEX);
                    }

                    break;

                default:
                    ESP_LOGW(MODULE_BT_SPP, "Can't be HERE!!! Received %d bytes", param->data_ind.len);

                    break;
                }
#endif
        }
        break;
        
    case ESP_SPP_CONG_EVT:
#if (DEBUG_BT_SPP)
        ESP_LOGI(MODULE_BT_SPP, "ESP_SPP_CONG_EVT");
#endif /* DEBUG_BT_SPP */
        break;
    
    case ESP_SPP_WRITE_EVT:
#if (DEBUG_BT_SPP)
        ESP_LOGI(MODULE_BT_SPP, "ESP_SPP_WRITE_EVT");
#endif /* DEBUG_BT_SPP */
        break;
    
    case ESP_SPP_SRV_OPEN_EVT:
#if (DEBUG_BT_SPP)
        ESP_LOGI(MODULE_BT_SPP, "ESP_SPP_SRV_OPEN_EVT status:%d handle:%d, rem_bda:[%s]", param->srv_open.status,
                 param->srv_open.handle, bda2str(param->srv_open.rem_bda, bda_str, sizeof(bda_str)));
#else
        UNUSED(bda_str);
#endif /* DEBUG_BT_SPP */
        protocol_state_set(SW_STATE_FULL_DUPLEX);
        g_esp_ssp_handle = param->srv_open.handle;
        gettimeofday(&g_time_old, NULL);
        break;
        
    case ESP_SPP_SRV_STOP_EVT:
#if (DEBUG_BT_SPP)
        ESP_LOGI(MODULE_BT_SPP, "ESP_SPP_SRV_STOP_EVT");
#endif /* DEBUG_BT_SPP */
        g_esp_ssp_handle = 0;
        (void)led_blink_set((struct blink_led *)g_status_handle, LED_STATUS_SLOW);
        break;
        
    case ESP_SPP_UNINIT_EVT:
#if (DEBUG_BT_SPP)
        ESP_LOGI(MODULE_BT_SPP, "ESP_SPP_UNINIT_EVT");
#endif /* DEBUG_BT_SPP */
        break;
    
    default:
        break;
    }
}

void esp_bt_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
{
    char bda_str[18] = {0};

    switch (event) {
    case ESP_BT_GAP_AUTH_CMPL_EVT:{
#if (DEBUG_BT_SPP)
        if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS) {
            ESP_LOGI(MODULE_BT_SPP, "authentication success: %s bda:[%s]", param->auth_cmpl.device_name,
                     bda2str(param->auth_cmpl.bda, bda_str, sizeof(bda_str)));
        } else {
            ESP_LOGE(MODULE_BT_SPP, "authentication failed, status:%d", param->auth_cmpl.stat);
        }
#else
        UNUSED(bda_str);
#endif /* DEBUG_BT_SPP */
        break;
    }
    case ESP_BT_GAP_PIN_REQ_EVT:{
#if (DEBUG_BT_SPP)
        ESP_LOGI(MODULE_BT_SPP, "ESP_BT_GAP_PIN_REQ_EVT min_16_digit:%d", param->pin_req.min_16_digit);
#endif /* DEBUG_BT_SPP */
        if (param->pin_req.min_16_digit) {
#if (DEBUG_BT_SPP)
            ESP_LOGI(MODULE_BT_SPP, "Input pin code: 0000 0000 0000 0000");
#endif /* DEBUG_BT_SPP */
            esp_bt_pin_code_t pin_code = {0};
            esp_bt_gap_pin_reply(param->pin_req.bda, true, 16, pin_code);
        } else {
#if (DEBUG_BT_SPP)
            ESP_LOGI(MODULE_BT_SPP, "Input pin code: 1234");
#endif /* DEBUG_BT_SPP */
            esp_bt_pin_code_t pin_code;
            pin_code[0] = '1';
            pin_code[1] = '2';
            pin_code[2] = '3';
            pin_code[3] = '4';
            esp_bt_gap_pin_reply(param->pin_req.bda, true, 4, pin_code);
        }
        break;
    }

#if (CONFIG_BT_SSP_ENABLED == true)
    case ESP_BT_GAP_CFM_REQ_EVT:
#if (DEBUG_BT_SPP)
        ESP_LOGI(MODULE_BT_SPP, "ESP_BT_GAP_CFM_REQ_EVT Please compare the numeric value: %d", param->cfm_req.num_val);
#endif /* DEBUG_BT_SPP */
        esp_bt_gap_ssp_confirm_reply(param->cfm_req.bda, true);
        break;
    case ESP_BT_GAP_KEY_NOTIF_EVT:
#if (DEBUG_BT_SPP)
        ESP_LOGI(MODULE_BT_SPP, "ESP_BT_GAP_KEY_NOTIF_EVT passkey:%d", param->key_notif.passkey);
#endif /* DEBUG_BT_SPP */
        break;
    case ESP_BT_GAP_KEY_REQ_EVT:
#if (DEBUG_BT_SPP)
        ESP_LOGI(MODULE_BT_SPP, "ESP_BT_GAP_KEY_REQ_EVT Please enter passkey!");
#endif /* DEBUG_BT_SPP */
        break;
#endif

    case ESP_BT_GAP_MODE_CHG_EVT:
#if (DEBUG_BT_SPP)
        ESP_LOGI(MODULE_BT_SPP, "ESP_BT_GAP_MODE_CHG_EVT mode:%d bda:[%s]", param->mode_chg.mode,
                 bda2str(param->mode_chg.bda, bda_str, sizeof(bda_str)));
#endif /* DEBUG_BT_SPP */
        break;

    default: {
#if (DEBUG_BT_SPP)
        ESP_LOGI(MODULE_BT_SPP, "event: %d", event);
#endif /* DEBUG_BT_SPP */
        break;
    }
    }
    return;
}


void bt_spp_stop(void)
{
    esp_bt_controller_status_t bt_status;

    protocol_state_set(SW_STATE_INVALID);

    ESP_ERROR_CHECK(esp_spp_deinit());
    ESP_ERROR_CHECK(esp_bluedroid_disable());
    ESP_ERROR_CHECK(esp_bluedroid_deinit());
    ESP_ERROR_CHECK(esp_bt_controller_disable());
    ESP_ERROR_CHECK(esp_bt_controller_deinit());
    do{
#if (DEBUG_BT_SPP)
        ESP_LOGI(MODULE_BT_SPP, "BT SPP wait stop");
#endif /* DEBUG_BT_SPP */
        vTaskDelay(100 / portTICK_PERIOD_MS);
        bt_status = esp_bt_controller_get_status();
    }while(ESP_BT_CONTROLLER_STATUS_IDLE != bt_status);
}

static void task_bt_start_spp(void* args)
{
    char bda_str[18] = {0};
    esp_err_t ret;

    //ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));
    int retry = 3;
    do{
        ret = esp_bt_controller_mem_release(ESP_BT_MODE_BLE);
        if (ESP_OK != ret){
            ESP_LOGW(MODULE_BT_SPP, "esp_bt_controller_mem_release ret = %08x retry %d", ret, retry);
            retry--;
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }while (ESP_OK != ret && retry > 0);

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if ((ret = esp_bt_controller_init(&bt_cfg)) != ESP_OK) {
        ESP_LOGE(MODULE_BT_SPP, "%s initialize controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT)) != ESP_OK) {
        ESP_LOGE(MODULE_BT_SPP, "%s enable controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bluedroid_init()) != ESP_OK) {
        ESP_LOGE(MODULE_BT_SPP, "%s initialize bluedroid failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bluedroid_enable()) != ESP_OK) {
        ESP_LOGE(MODULE_BT_SPP, "%s enable bluedroid failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bt_gap_register_callback(esp_bt_gap_cb)) != ESP_OK) {
        ESP_LOGE(MODULE_BT_SPP, "%s gap register failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_spp_register_callback(esp_spp_cb)) != ESP_OK) {
        ESP_LOGE(MODULE_BT_SPP, "%s spp register failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_spp_init(g_esp_spp_mode)) != ESP_OK) {
        ESP_LOGE(MODULE_BT_SPP, "%s spp init failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

#if (CONFIG_BT_SSP_ENABLED == true)
    /* Set default parameters for Secure Simple Pairing */
    esp_bt_sp_param_t param_type = ESP_BT_SP_IOCAP_MODE;
    esp_bt_io_cap_t iocap = ESP_BT_IO_CAP_IO;
    esp_bt_gap_set_security_param(param_type, &iocap, sizeof(uint8_t));
#endif

    /*
     * Set default parameters for Legacy Pairing
     * Use variable pin, input pin code when pairing
     */
    esp_bt_pin_type_t pin_type = ESP_BT_PIN_TYPE_VARIABLE;
    esp_bt_pin_code_t pin_code;
    esp_bt_gap_set_pin(pin_type, 0, pin_code);

    ESP_LOGI(MODULE_BT_SPP, "Own address:[%s]", bda2str((uint8_t *)esp_bt_dev_get_address(), bda_str, sizeof(bda_str)));

    //vTaskDelete(NULL);
}

void bt_spp_start(void)
{
    ESP_ERROR_CHECK(snap_sw_module_start(task_bt_start_spp, false, 0, MODULE_BT_SPP));
    protocol_state_set(SW_STATE_IDLE);
}

