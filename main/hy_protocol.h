

#ifndef __HY_PROTOCOL_SOURCE_H__
#define __HY_PROTOCOL_SOURCE_H__

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t ttl_handle_wifi_hy(uint8_t * buf, int len);
esp_err_t udp_handle_hy(uint8_t * buf, int len);


#ifdef __cplusplus
}
#endif

#endif // #ifndef __HY_PROTOCOL_SOURCE_H__


