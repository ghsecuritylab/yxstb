#ifndef __HIPPO_OS_H__
#define __HIPPO_OS_H__

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#if defined(OS_WIN32)
#include "OS/win32/Hippo_win32.h"

#elif defined(OS_LINUX)
#include "OS/linux/Hippo_linux.h"

#elif defined(__WIN32__)

#define OS_WIN32
#include "OS/win32/Hippo_win32.h"
#else
//#param error;
#define OS_LINUX
#include "OS/linux/Hippo_linux.h"
#endif


#endif

