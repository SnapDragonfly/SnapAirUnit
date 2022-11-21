
#ifndef __KEY_SOURCE_H__
#define __KEY_SOURCE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "iot_button.h"

#define KEY_MODE CONFIG_KEY_MODE

button_handle_t module_key_start(uint8_t num);
void mode_key_lock(void);
void mode_key_unlock(void);

#ifdef __cplusplus
}
#endif

#endif // #ifndef __KEY_SOURCE_H__

