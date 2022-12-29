/// @file softap.c

/*
 * idf header files
 */
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_spp_api.h"
#include "esp_check.h"
#include "esp_bt.h"
#include "lwip/err.h"
#include "lwip/sys.h"

/*
 * basic header files
 */
#include "config.h"
#include "define.h"

/*
 * module header files
 */
#include "module.h"
#include "factory_setting.h"
#include "mode.h"

/*
 * service header files
 */
#include "softap.h"
#include "rest_server.h"

static esp_event_handler_instance_t g_softap_instance_any_id;
static esp_netif_t*               g_softap_instance_netif = NULL;

static void wifi_sofap_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(MODULE_WIFI_AP, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(MODULE_WIFI_AP, "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}

static void wifi_softap_task(void* args)
{
    //ESP_ERROR_CHECK(esp_netif_init());
    //ESP_ERROR_CHECK(esp_event_loop_create_default());
    g_softap_instance_netif = esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_sofap_event_handler,
                                                        NULL,
                                                        &g_softap_instance_any_id));

    wifi_config_t wifi_config = {
        .ap = {
            //.ssid = FACTORY_ESP_WIFI_AP_SSID,
            .ssid_len = strlen(get_ap_ssid()),
            .channel = FACTORY_ESP_WIFI_CHANNEL,
            //.password = FACTORY_ESP_WIFI_AP_PASS,
            .max_connection = FACTORY_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
            .pmf_cfg = {
                    .required = false,
            },
        },
    };
    
    memset(wifi_config.ap.ssid, 0, WIFI_SSID_LENGTH);
    strcpy((char *)wifi_config.ap.ssid, get_ap_ssid());
    memset(wifi_config.ap.password, 0, WIFI_SSID_LENGTH);
    strcpy((char *)wifi_config.ap.password, get_ap_pass());

    if (strlen(get_ap_pass()) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(MODULE_WIFI_AP, "wifi_start_softap done. SSID:%s password:%s channel:%d",
             get_ap_ssid(), get_ap_pass(), FACTORY_ESP_WIFI_CHANNEL);

    set_str_ip(192, 168, 4, 1);

    //vTaskDelete(NULL);
}


void wifi_ap_start(void)
{
    ESP_ERROR_CHECK(snap_sw_module_start(wifi_softap_task, false, 0, MODULE_WIFI_AP));
    ESP_ERROR_CHECK(rest_srv_start(CONFIG_RESTFUL_WEB_MOUNT_POINT));
    protocol_state_set(SW_STATE_IDLE);
}

void wifi_ap_stop(void)
{
    //esp_err_t ret;
    //wifi_mode_t wifi_mode;
    ESP_ERROR_CHECK(rest_srv_stop());

    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &g_softap_instance_any_id));
    ESP_ERROR_CHECK(esp_wifi_deinit());
    esp_netif_destroy_default_wifi(g_softap_instance_netif);
    //ESP_ERROR_CHECK(esp_netif_deinit());
    g_softap_instance_netif = NULL;
}


