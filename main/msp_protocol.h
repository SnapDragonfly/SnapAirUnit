
#ifndef __MSP_PROTOCOL_SOURCE_H__
#define __MSP_PROTOCOL_SOURCE_H__

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t udp_handle_msp_protocol(uint8_t * buf, int len);
esp_err_t ttl_handle_msp_protocol(uint8_t * buf, int len);


#ifdef __cplusplus
}
#endif

#endif // #ifndef __MSP_PROTOCOL_SOURCE_H__

