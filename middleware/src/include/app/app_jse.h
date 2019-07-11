#include <string.h>
#ifndef __APP_JSE_H__
#define __APP_JSE_H__

#ifdef __cplusplus
extern "C" {
#endif

int jse_sys_sys_control(char* str);
int jse_sys_sys_read(char* str, char* buf, int length);

#ifdef __cplusplus
}
#endif

#endif

