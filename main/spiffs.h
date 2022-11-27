
#ifndef __SPIFFS_SOURCE_H__
#define __SPIFFS_SOURCE_H__

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t start_spiffs(void);
esp_err_t stop_spiffs(void);
esp_err_t spiffs_test(void);


#ifdef __cplusplus
}
#endif

#endif // #ifndef __SPIFFS_SOURCE_H__

