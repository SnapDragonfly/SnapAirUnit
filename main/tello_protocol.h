

#ifndef __TELLO_PROTOCOL_SOURCE_H__
#define __TELLO_PROTOCOL_SOURCE_H__

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t udp_handle_tello_protocol(uint8_t * buf, int len);
esp_err_t ttl_handle_tello_protocol(uint8_t * buf, int len);



#ifdef __cplusplus
}
#endif

#endif // #ifndef __TELLO_PROTOCOL_SOURCE_H__


