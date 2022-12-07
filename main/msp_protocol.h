
#ifndef __MSP_PROTOCOL_SOURCE_H__
#define __MSP_PROTOCOL_SOURCE_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MESSAGE_UNKNOW  = 0,
    MESSAGE_CENTER  = 1,
    MESSAGE_MSP      = 2,
    MESSAGE_VERSION_COUNT
} messageVersion_e;

#define MSP_SET_RAW_RC           200    //in message          8 rc chan


esp_err_t handle_msp_protocol(uint8_t * buf, int len);
esp_err_t ttl_handle_msp_protocol(uint8_t * buf, int len);
esp_err_t center_handle_msp_protocol(uint8_t * buf, int len);

esp_err_t start_message_center(void);


esp_err_t mspSetChannel(uint8_t index, uint16_t value);
esp_err_t mspSetChannels(uint8_t count, uint16_t *value);
esp_err_t mspUpdateChannels(void);

esp_err_t mspSetMessage(messageVersion_e type);
messageVersion_e mspGetMessage(void);

#ifdef __cplusplus
}
#endif

#endif // #ifndef __MSP_PROTOCOL_SOURCE_H__

