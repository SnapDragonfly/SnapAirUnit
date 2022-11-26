
#ifndef __RESTFUL_SOURCE_H__
#define __RESTFUL_SOURCE_H__

#ifdef __cplusplus
extern "C" {
#endif

void sanp_sw_rest_init(void);
esp_err_t stop_rest_server(void);
esp_err_t start_rest_server(const char *base_path);


#ifdef __cplusplus
}
#endif

#endif // #ifndef __RESTFUL_SOURCE_H__

