
#ifndef __UDP_SERVER_SOURCE_H__
#define __UDP_SERVER_SOURCE_H__

#ifdef __cplusplus
extern "C" {
#endif

//void sanp_sw_udp_init(void);
//esp_err_t stop_udp_server(void);
esp_err_t start_udp_server(void);
esp_err_t udp_send_msg(uint8_t * buf, int len);


#ifdef __cplusplus
}
#endif

#endif // #ifndef __UDP_SERVER_SOURCE_H__

