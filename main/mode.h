
#ifndef __MODE_SOURCE_H__
#define __MODE_SOURCE_H__

#ifdef __cplusplus
extern "C" {
#endif


enum {
    SW_MODE_WIFI_AP       = 0,
    SW_MODE_WIFI_STA      = 1,
    SW_MODE_BT_SPP        = 2,
    SW_MODE_NULL
};

esp_err_t snap_sw_mode_switch(uint16_t mode);
uint16_t snap_sw_mode_next(void);
uint16_t snap_sw_mode_get(void);

esp_err_t snap_sw_mode_start(TaskFunction_t pxTaskCode, bool task);
void snap_sw_mode_init(void);

#ifdef __cplusplus
}
#endif

#endif // #ifndef __MODE_SOURCE_H__


