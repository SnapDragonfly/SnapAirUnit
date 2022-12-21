

#ifndef __COMMON_SOURCE_H__
#define __COMMON_SOURCE_H__

#ifdef __cplusplus
extern "C" {
#endif

#define DEVICE_NAME_SNAP_AIR_UNIT              "SnapAirUnit"
    
#define SAND_BOX_ALIVE_CHARACTER               ('.')
#define SAND_BOX_REBOOT_PROMOTES               (10)

#define STACK_BUFFER_1K0                       (1024)
#define STACK_BUFFER_1K5                       (1536)
#define STACK_BUFFER_2K0                       (2048)
#define STACK_BUFFER_2K5                       (2560)
#define STACK_BUFFER_3K0                       (3072)
#define STACK_BUFFER_3K5                       (3584)
#define STACK_BUFFER_4K0                       (4096)
#define STACK_BUFFER_4K5                       (4608)
#define STACK_BUFFER_5K0                       (5120)
#define STACK_BUFFER_5K5                       (5632)


#define TASK_BUFFER_1K0                        (1024)
#define TASK_BUFFER_1K5                        (1536)
#define TASK_BUFFER_2K0                        (2048)
#define TASK_BUFFER_2K5                        (2560)
#define TASK_BUFFER_3K0                        (3072)
#define TASK_BUFFER_3K5                        (3584)
#define TASK_BUFFER_4K0                        (4096)
#define TASK_BUFFER_4K5                        (4608)
#define TASK_BUFFER_5K0                        (5120)
#define TASK_BUFFER_5K5                        (5632)

#define TIME_5_MS                              (5)
#define TIME_10_MS                             (10)
#define TIME_20_MS                             (20)
#define TIME_50_MS                             (50)
#define TIME_100_MS                            (100)
#define TIME_200_MS                            (200)
#define TIME_500_MS                            (500)
#define TIME_ONE_SECOND_IN_MS                  (1000)
#define TIME_TWO_SECOND_IN_MS                  (2000)

#define TTL_BUF_BASIC_SIZE                     (512)
#define STR_BUFFER_LEN                         (256)
#define STR_IP_LEN                             (16)
#define STR_VERSION_LEN                        (32)
    
    
#define TIME_DIFF_IN_MS(prev, curr)            ((curr - prev)/1000)
    
#define UNUSED(x)                              (void)(x)


#ifdef __cplusplus
}
#endif

#endif // #ifndef __COMMON_SOURCE_H__


