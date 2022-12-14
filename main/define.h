
#ifndef __DEFINE_SOURCE_H__
#define __DEFINE_SOURCE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "factory_setting.h"


#define DEVICE_NAME_SNAP_AIR_UNIT              "SnapAirUnit"

#define SAND_BOX_ALIVE_CHARACTER               ('.')
#define SAND_BOX_REBOOT_PROMOTES               (10)

#define TASK_SIMPLE_BUFFER                     (1024)
#define TASK_MIDDLE_BUFFER                     (2048)
#define TASK_LARGE_BUFFER                      (3072)
#define TASK_EXLARGE_BUFFER                    (4096)
#define TASK_XXLARGE_BUFFER                    (5120)

#define TIME_5_MS                              (5)
#define TIME_10_MS                             (10)
#define TIME_20_MS                             (20)
#define TIME_50_MS                             (50)
#define TIME_100_MS                            (100)
#define TIME_200_MS                            (200)
#define TIME_500_MS                            (500)
#define TIME_ONE_SECOND_IN_MS                  (1000)
#define TIME_TWO_SECOND_IN_MS                  (2000)

#define STR_BUFFER_LEN                         (128)
#define STR_IP_LEN                             (16)
#define STR_VERSION_LEN                        (32)


#define TIME_DIFF_IN_MS(prev, curr)            ((curr - prev)/1000)

#define UNUSED(x)                              (void)(x)


#define STRCAT(param1, param2)                 (param1##param2)
#define TOSTR(param)                           #param

#ifdef __cplusplus
}
#endif

#endif // #ifndef __DEFINE_SOURCE_H__

