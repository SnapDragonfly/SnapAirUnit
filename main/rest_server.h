
#ifndef __RESTFUL_SOURCE_H__
#define __RESTFUL_SOURCE_H__

#ifdef __cplusplus
extern "C" {
#endif

void rest_srv_init(void* args);
esp_err_t rest_srv_stop(void);
esp_err_t rest_srv_start(const char *base_path);


#ifdef __cplusplus
}
#endif

#endif // #ifndef __RESTFUL_SOURCE_H__

