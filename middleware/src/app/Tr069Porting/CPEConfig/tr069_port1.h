
#ifndef _tr069_port__h_
#define _tr069_port__h_

#ifdef SQM_INCLUDED
#include "sqm_port.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

int app_TMS_aes_keys_set();
int app_TMS_aes_keys_get(unsigned char *key);

#ifdef __cplusplus
}
#endif

#endif//_tr069_port__h_
