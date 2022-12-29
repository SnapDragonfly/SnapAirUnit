
#ifndef __MODE_SOURCE_H__
#define __MODE_SOURCE_H__

#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
    SW_MODE_WIFI_AP       = 0,
    SW_MODE_WIFI_STA      = 1,
    SW_MODE_BT_SPP        = 2,
    SW_MODE_NULL
} enum_wireless_mode_t;

typedef enum {
    SW_STATE_INVALID      = 0,
    SW_STATE_IDLE         = 1,
    SW_STATE_HALF_DUPLEX  = 2,
    SW_STATE_FULL_DUPLEX  = 3,
    SW_STATE_TELLO        = 4,
    SW_STATE_CLI          = 5,
    SW_STATE_NULL
} enum_protocol_state_t;

bool snap_sw_command_get(void);
esp_err_t snap_sw_command_set(int mode);


void protocol_state_set(enum_protocol_state_t state);
void protocol_state_degrade(enum_protocol_state_t state);
void protocol_state_upgrade(enum_protocol_state_t state);
bool protocol_state_active(enum_wireless_mode_t mode);
enum_protocol_state_t protocol_state_get(void);



esp_err_t wireless_mode_init(void);
esp_err_t wireless_mode_switch(enum_wireless_mode_t mode);
enum_wireless_mode_t wireless_mode_next(void);
enum_wireless_mode_t wireless_mode_get(void);
void wireless_mode_set(enum_wireless_mode_t mode);


#ifdef __cplusplus
}
#endif

#endif // #ifndef __MODE_SOURCE_H__


