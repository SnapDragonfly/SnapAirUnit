
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "nvs.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
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
#include "define.h"
#include "ttl.h"
#include "module.h"
#include "mode.h"
#include "btspp.h"

#define SPP_SERVER_NAME     DEVICE_NAME_SNAP_AIR_UNIT
#define SPP_DEVICE_NAME     DEVICE_NAME_SNAP_AIR_UNIT

static struct timeval time_new, time_old;
static long data_num = 0;
static const esp_spp_mode_t esp_spp_mode = ESP_SPP_MODE_CB;
static const esp_spp_sec_t sec_mask = ESP_SPP_SEC_AUTHENTICATE;
static const esp_spp_role_t role_slave = ESP_SPP_ROLE_SLAVE;

uint32_t esp_ssp_handle = 0;
bool bt_is_connected = false;

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
    float time_old_s = time_old.tv_sec + time_old.tv_usec / 1000000.0;
    float time_new_s = time_new.tv_sec + time_new.tv_usec / 1000000.0;
    float time_interval = time_new_s - time_old_s;
    float speed = data_num * 8 / time_interval / 1000.0;
    ESP_LOGI(MODULE_BT_SPP, "speed(%fs ~ %fs): %f kbit/s" , time_old_s, time_new_s, speed);
    data_num = 0;
    time_old.tv_sec = time_new.tv_sec;
    time_old.tv_usec = time_new.tv_usec;
}

static void esp_spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
    char bda_str[18] = {0};

    switch (event) {
    case ESP_SPP_INIT_EVT:
        if (param->init.status == ESP_SPP_SUCCESS) {
            ESP_LOGI(MODULE_BT_SPP, "ESP_SPP_INIT_EVT");
            esp_spp_start_srv(sec_mask, role_slave, 0, SPP_SERVER_NAME);
        } else {
            ESP_LOGE(MODULE_BT_SPP, "ESP_SPP_INIT_EVT status:%d", param->init.status);
        }
        break;
        
    case ESP_SPP_DISCOVERY_COMP_EVT:
        ESP_LOGI(MODULE_BT_SPP, "ESP_SPP_DISCOVERY_COMP_EVT");
        break;
    
    case ESP_SPP_OPEN_EVT:
        ESP_LOGI(MODULE_BT_SPP, "ESP_SPP_OPEN_EVT");
        break;
    
    case ESP_SPP_CLOSE_EVT:
        bt_is_connected = false;
        ESP_LOGI(MODULE_BT_SPP, "ESP_SPP_CLOSE_EVT status:%d handle:%d close_by_remote:%d", param->close.status,
                 param->close.handle, param->close.async);
        break;
        
    case ESP_SPP_START_EVT:
        if (param->start.status == ESP_SPP_SUCCESS) {
            ESP_LOGI(MODULE_BT_SPP, "ESP_SPP_START_EVT handle:%d sec_id:%d scn:%d", param->start.handle, param->start.sec_id,
                     param->start.scn);
            esp_bt_dev_set_device_name(SPP_DEVICE_NAME);
            esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
        } else {
            ESP_LOGE(MODULE_BT_SPP, "ESP_SPP_START_EVT status:%d", param->start.status);
        }
        break;
        
    case ESP_SPP_CL_INIT_EVT:
        ESP_LOGI(MODULE_BT_SPP, "ESP_SPP_CL_INIT_EVT");
        break;
    
    case ESP_SPP_DATA_IND_EVT:
        ESP_LOGI(MODULE_BT_SPP, "ESP_SPP_DATA_IND_EVT len:%d handle:%d",
                 param->data_ind.len, param->data_ind.handle);
#if (DEBUG_BT_SPP)
        if (param->data_ind.len < 128) {
            esp_log_buffer_hex(MODULE_BT_SPP, param->data_ind.data, param->data_ind.len);
        }else{
            gettimeofday(&time_new, NULL);
            data_num += param->data_ind.len;
            if (time_new.tv_sec - time_old.tv_sec >= 3) {
                print_speed();
            }
        }
#else
        UNUSED(print_speed);
#endif /* DEBUG_BT_SPP */

        uart_write_bytes(TTL_UART_NUM, param->data_ind.data, param->data_ind.len);
        break;
        
    case ESP_SPP_CONG_EVT:
        ESP_LOGI(MODULE_BT_SPP, "ESP_SPP_CONG_EVT");
        break;
    
    case ESP_SPP_WRITE_EVT:
        ESP_LOGI(MODULE_BT_SPP, "ESP_SPP_WRITE_EVT");
        break;
    
    case ESP_SPP_SRV_OPEN_EVT:
        ESP_LOGI(MODULE_BT_SPP, "ESP_SPP_SRV_OPEN_EVT status:%d handle:%d, rem_bda:[%s]", param->srv_open.status,
                 param->srv_open.handle, bda2str(param->srv_open.rem_bda, bda_str, sizeof(bda_str)));
        bt_is_connected = true;
        esp_ssp_handle = param->srv_open.handle;
        gettimeofday(&time_old, NULL);
        break;
        
    case ESP_SPP_SRV_STOP_EVT:
        ESP_LOGI(MODULE_BT_SPP, "ESP_SPP_SRV_STOP_EVT");
        esp_ssp_handle = 0;
        break;
        
    case ESP_SPP_UNINIT_EVT:
        ESP_LOGI(MODULE_BT_SPP, "ESP_SPP_UNINIT_EVT");
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
        if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS) {
            ESP_LOGI(MODULE_BT_SPP, "authentication success: %s bda:[%s]", param->auth_cmpl.device_name,
                     bda2str(param->auth_cmpl.bda, bda_str, sizeof(bda_str)));
        } else {
            ESP_LOGE(MODULE_BT_SPP, "authentication failed, status:%d", param->auth_cmpl.stat);
        }
        break;
    }
    case ESP_BT_GAP_PIN_REQ_EVT:{
        ESP_LOGI(MODULE_BT_SPP, "ESP_BT_GAP_PIN_REQ_EVT min_16_digit:%d", param->pin_req.min_16_digit);
        if (param->pin_req.min_16_digit) {
            ESP_LOGI(MODULE_BT_SPP, "Input pin code: 0000 0000 0000 0000");
            esp_bt_pin_code_t pin_code = {0};
            esp_bt_gap_pin_reply(param->pin_req.bda, true, 16, pin_code);
        } else {
            ESP_LOGI(MODULE_BT_SPP, "Input pin code: 1234");
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
        ESP_LOGI(MODULE_BT_SPP, "ESP_BT_GAP_CFM_REQ_EVT Please compare the numeric value: %d", param->cfm_req.num_val);
        esp_bt_gap_ssp_confirm_reply(param->cfm_req.bda, true);
        break;
    case ESP_BT_GAP_KEY_NOTIF_EVT:
        ESP_LOGI(MODULE_BT_SPP, "ESP_BT_GAP_KEY_NOTIF_EVT passkey:%d", param->key_notif.passkey);
        break;
    case ESP_BT_GAP_KEY_REQ_EVT:
        ESP_LOGI(MODULE_BT_SPP, "ESP_BT_GAP_KEY_REQ_EVT Please enter passkey!");
        break;
#endif

    case ESP_BT_GAP_MODE_CHG_EVT:
        ESP_LOGI(MODULE_BT_SPP, "ESP_BT_GAP_MODE_CHG_EVT mode:%d bda:[%s]", param->mode_chg.mode,
                 bda2str(param->mode_chg.bda, bda_str, sizeof(bda_str)));
        break;

    default: {
        ESP_LOGI(MODULE_BT_SPP, "event: %d", event);
        break;
    }
    }
    return;
}


void bt_deinit_spp(void)
{
    esp_bt_controller_status_t bt_status;

    bt_is_connected = false;

    ESP_ERROR_CHECK(esp_spp_deinit());
    ESP_ERROR_CHECK(esp_bluedroid_disable());
    ESP_ERROR_CHECK(esp_bluedroid_deinit());
    ESP_ERROR_CHECK(esp_bt_controller_disable());
    ESP_ERROR_CHECK(esp_bt_controller_deinit());
    do{
        ESP_LOGI(MODULE_BT_SPP, "BT SPP wait stop");
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

    if ((ret = esp_spp_init(esp_spp_mode)) != ESP_OK) {
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

void bt_init_spp(void)
{
    ESP_ERROR_CHECK(snap_sw_module_start(task_bt_start_spp, false, 0, MODULE_BT_SPP));
}

