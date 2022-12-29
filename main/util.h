
#ifndef __UTIL_SOURCE_H__
#define __UTIL_SOURCE_H__

#ifdef __cplusplus
extern "C" {
#endif

void UTIL_reboot(int in_seconds);
void UTIL_alive(char c);
void UTIL_idle(void);

#ifdef __cplusplus
}
#endif

#endif // #ifndef __UTIL_SOURCE_H__

