
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

#define MSP_SET_RAW_RC                         200    //in message          8 rc chan

#define MAX_SUPPORTED_RC_CHANNEL_COUNT         18


esp_err_t wireless_handle_msp(uint8_t * buf, int len);
esp_err_t ttl_handle_wifi_msp(uint8_t * buf, int len);
esp_err_t ttl_handle_wifi_nomsp(uint8_t * buf, int len);
esp_err_t ttl_handle_bt_package(uint8_t * buf, int len);
esp_err_t auc_handle_msp(uint8_t * buf, int len);

esp_err_t auc_srv_start(void);
esp_err_t auc_set_channel(uint8_t index, uint16_t value);
esp_err_t auc_set_channels(uint8_t count, uint16_t *value);
esp_err_t auc_update_channels(void);
esp_err_t auc_set_type(messageVersion_e type);
messageVersion_e auc_get_type(void);

#ifdef __cplusplus
}
#endif

#endif // #ifndef __MSP_PROTOCOL_SOURCE_H__

