#ifndef __HIPPO_LINUX_HH_
#define __HIPPO_LINUX_HH_

#include <stdlib.h>
#include <strings.h>

#include <time.h>
#include <sys/select.h>
#include <pthread.h>
#include <semaphore.h>

#include "Hippo_HEventBaseLinux.h"

#ifdef __cplusplus
extern "C" {
#endif
inline unsigned long a_Ticker_get_millis( void )
{
	struct timespec tp;
    clock_gettime( CLOCK_MONOTONIC, &tp );

    return tp.tv_sec*1000 + tp.tv_nsec / 1000000 ;
}
#ifdef __cplusplus
}
#endif

#endif

