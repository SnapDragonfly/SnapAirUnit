

#ifndef __FACTORY_SETTINGS_SOURCE_H__
#define __FACTORY_SETTINGS_SOURCE_H__

#ifdef __cplusplus
extern "C" {
#endif

void factory_settings_init(void* args);


esp_err_t restore_factory_settings(void);
esp_err_t restore_ap_settings(void);
esp_err_t restore_sta_settings(void);
esp_err_t set_ap_settings(const char * ssid, const char * pass);
esp_err_t set_sta_settings(const char * ssid, const char * pass);
esp_err_t set_str_ip(uint16_t a, uint16_t b, uint16_t c, uint16_t d);
char * get_ap_ssid(void);
char * get_ap_pass(void);
char * get_sta_ssid(void);
char * get_sta_pass(void);
char * get_str_ip(void);

char * get_idf_versions(void);
char * get_app_versions(void);


#ifdef __cplusplus
}
#endif

#endif // #ifndef __FACTORY_SETTINGS_SOURCE_H__


