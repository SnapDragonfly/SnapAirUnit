
#ifndef __DEFINE_SOURCE_H__
#define __DEFINE_SOURCE_H__

#ifdef __cplusplus
extern "C" {
#endif

#define DEVICE_NAME_SNAP_AIR_UNIT              ("Snap Air Unit")

#define SAND_BOX_ALIVE_CHARACTER               ('.')
#define SAND_BOX_REBOOT_PROMOTES               (10)

#define TASK_SIMPLE_BUFFER                     (1024)
#define TASK_MIDDLE_BUFFER                     (2048)
#define TASK_LARGE_BUFFER                      (3072)

#define TIME_ONE_SECOND_IN_MS                  (1000)


#define TIME_DIFF_IN_MS(prev, curr)            ((curr - prev)/1000)

#define UNUSED(x)                              (void)(x)


#define STRCAT(param1, param2)                 (param1##param2)
#define TOSTR(param)                           #param

#ifdef __cplusplus
}
#endif

#endif // #ifndef __DEFINE_SOURCE_H__

