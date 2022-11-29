
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "define.h"
#include "module.h"
#include "mode.h"
#include "station.h"
#include "rest_server.h"

#if CONFIG_ESP_WIFI_AUTH_OPEN
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_OPEN
#elif CONFIG_ESP_WIFI_AUTH_WEP
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WEP
#elif CONFIG_ESP_WIFI_AUTH_WPA_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WAPI_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WAPI_PSK
#endif

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static int  s_retry_num       = 0;
bool g_wifi_sta_start  = true;

static esp_event_handler_instance_t sta_instance_any_id;
static esp_event_handler_instance_t sta_instance_got_ip;
static esp_netif_t* sta_instance_netif = NULL;

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < FACTORY_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(MODULE_WIFI_STA, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(MODULE_WIFI_STA,"connect to the AP fail");
        snap_sw_state_set(SW_STATE_INVALID);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(MODULE_WIFI_STA, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        snap_sw_state_set(SW_STATE_IDLE);
    }
}

static void task_wifi_start_sta(void* args)
{
    snap_sw_state_set(SW_STATE_INVALID);

    s_wifi_event_group = xEventGroupCreate();

    //ESP_ERROR_CHECK(esp_netif_init());
    //ESP_ERROR_CHECK(esp_event_loop_create_default());
    sta_instance_netif = esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &sta_instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &sta_instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            //.ssid = FACTORY_ESP_WIFI_STA_SSID,
            //.password = FACTORY_ESP_WIFI_STA_PASS,
            /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (pasword len => 8).
             * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
             * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
	     * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
             */
            .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        },
    };

    memset(wifi_config.sta.ssid, 0, SSID_LENGTH);
    strcpy((char *)wifi_config.sta.ssid, get_sta_ssid());
    memset(wifi_config.sta.password, 0, SSID_LENGTH);
    strcpy((char *)wifi_config.sta.password, get_sta_pass());

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(MODULE_WIFI_STA, "wifi_init_sta done.");

    int i = 10;
    do{
        i--;
        
        /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
         * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
        EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                pdFALSE,
                pdFALSE,
                TIME_ONE_SECOND_IN_MS / portTICK_PERIOD_MS);
                //portMAX_DELAY);

        /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
         * happened. */
        if (bits & WIFI_CONNECTED_BIT) {
            ESP_LOGI(MODULE_WIFI_STA, "connected to ap SSID:%s password:%s",
                     get_sta_ssid(), get_sta_pass());
            snap_sw_state_set(SW_STATE_IDLE);
            g_wifi_sta_start = true;
            break;
        } else if (bits & WIFI_FAIL_BIT) {
            ESP_LOGI(MODULE_WIFI_STA, "Failed to connect to SSID:%s, password:%s",
                     get_sta_ssid(), get_sta_pass());
            g_wifi_sta_start = false;
            //snap_sw_mode_switch(SW_MODE_WIFI_AP);
        } else {
            ESP_LOGW(MODULE_WIFI_STA, "UNEXPECTED EVENT %08X", bits);
            if(i <= 0){
                g_wifi_sta_start = false;
                //snap_sw_mode_switch(SW_MODE_WIFI_AP);
            }
        }

        ESP_LOGI(MODULE_WIFI_STA, "Rechecking in %d seconds...\n", i);
        vTaskDelay(TIME_ONE_SECOND_IN_MS / portTICK_PERIOD_MS);
    }while(i > 0);

    if (g_wifi_sta_start){
        ESP_ERROR_CHECK(start_rest_server(CONFIG_RESTFUL_WEB_MOUNT_POINT));
    } else {
        extern void snap_sw_mode_set(enum_mode_t mode);
        snap_sw_mode_set(SW_MODE_NULL);
    }
    vTaskDelete(NULL);
}

void wifi_init_sta(void)
{
    s_retry_num = 0;
    ESP_ERROR_CHECK(snap_sw_module_start(task_wifi_start_sta, true, TASK_EXLARGE_BUFFER, MODULE_WIFI_STA));
}

void wifi_stop_sta(void)
{
    //esp_err_t ret;
    //wifi_mode_t wifi_mode;
    if(g_wifi_sta_start){
        ESP_ERROR_CHECK(stop_rest_server());
    }
    g_wifi_sta_start = true;

    snap_sw_state_set(SW_STATE_INVALID);
    ESP_ERROR_CHECK(esp_wifi_disconnect());
    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &sta_instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &sta_instance_any_id));
    ESP_ERROR_CHECK(esp_wifi_deinit());
    esp_netif_destroy_default_wifi(sta_instance_netif);
    //ESP_ERROR_CHECK(esp_netif_deinit());
    vEventGroupDelete(s_wifi_event_group);
    sta_instance_netif = NULL;
}


