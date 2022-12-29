

#ifndef __TELLO_PROTOCOL_SOURCE_H__
#define __TELLO_PROTOCOL_SOURCE_H__

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t nomsp_handle_tello(uint8_t * buf, int len);
esp_err_t nomsp_handle_cli(uint8_t * buf, int len);

#ifdef __cplusplus
}
#endif

#endif // #ifndef __TELLO_PROTOCOL_SOURCE_H__


