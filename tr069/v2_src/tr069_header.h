
#ifndef __TR069_HEADER_H__
#define __TR069_HEADER_H__


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tr069_os.h"

#if 0
#include "ind_mem.h"
#else
#define IND_MALLOC(size)			malloc(size)
#define IND_DALLOC(size)			malloc(size)
#define IND_CALLOC(size, n)			calloc(size, n)
#define IND_STRDUP(ptr)				strdup(ptr)
#define IND_REALLOC(ptr, size)		realloc(ptr, size)
#define IND_FREE(ptr)				free(ptr)
#define IND_MEMCPY(dest,src, n)		memcpy(dest,src, n)
#define IND_MEMSET(s, ch, n)		memset(s, ch, n)
#define IND_STRCPY(dest, src)		strcpy(dest, src)
#define IND_STRNCPY(dest, src, n)	strncpy(dest, src, n)
#endif

#include "tr069_define.h"
#include "tr069_stdex.h"

#include "tr069_struct.h"
#include "tr069_param.h"
#include "tr069_config.h"
#include "tr069_method.h"
#include "tr069_tr106.h"
#include "tr069_main.h"

#include "tr069_timer.h"

#include "tr069_http.h"
#include "tr069_soap.h"
#include "tr069_diag.h"
#include "tr069_port.h"

#include "tr069_debug.h"

#include "tr069_stun.h"
#include "tr069_hmac_sha1.h"

#endif
