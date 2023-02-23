
#ifndef __UDP_SERVER_SOURCE_H__
#define __UDP_SERVER_SOURCE_H__

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t udp_srv_start(void);
int udp_control_send(uint8_t * buf, int len);

#ifdef __cplusplus
}
#endif

#endif // #ifndef __UDP_SERVER_SOURCE_H__

