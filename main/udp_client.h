
#ifndef __UDP_CLIENT_SOURCE_H__
#define __UDP_CLIENT_SOURCE_H__

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t udp_clt_start(void);
int udp_status_send(uint8_t * buf, int len);

#ifdef __cplusplus
}
#endif

#endif // #ifndef __UDP_CLIENT_SOURCE_H__

