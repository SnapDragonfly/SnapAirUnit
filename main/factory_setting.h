

#ifndef __FACTORY_SETTINGS_SOURCE_H__
#define __FACTORY_SETTINGS_SOURCE_H__

#ifdef __cplusplus
extern "C" {
#endif

#define SSID_LENGTH                    16
#define PASS_LENGTH                    16

// for wifi AP mode
#define FACTORY_ESP_WIFI_AP_SSID       CONFIG_ESP_WIFI_AP_SSID
#define FACTORY_ESP_WIFI_AP_PASS       CONFIG_ESP_WIFI_AP_PASSWORD

#define FACTORY_ESP_WIFI_CHANNEL       CONFIG_ESP_WIFI_CHANNEL
#define FACTORY_MAX_STA_CONN           CONFIG_ESP_MAX_STA_CONN


// for wifi station mode
#define FACTORY_ESP_WIFI_STA_SSID      CONFIG_ESP_WIFI_STA_SSID
#define FACTORY_ESP_WIFI_STA_PASS      CONFIG_ESP_WIFI_STA_PASSWORD

#define FACTORY_ESP_MAXIMUM_RETRY      CONFIG_ESP_MAXIMUM_RETRY


esp_err_t start_factory_settings(void);
esp_err_t restore_factory_settings(void);
esp_err_t restore_ap_settings(void);
esp_err_t restore_sta_settings(void);
esp_err_t set_ap_settings(const char * ssid, const char * pass);
esp_err_t set_sta_settings(const char * ssid, const char * pass);
char * get_ap_ssid(void);
char * get_ap_pass(void);
char * get_sta_ssid(void);
char * get_sta_pass(void);

#ifdef __cplusplus
}
#endif

#endif // #ifndef __FACTORY_SETTINGS_SOURCE_H__


