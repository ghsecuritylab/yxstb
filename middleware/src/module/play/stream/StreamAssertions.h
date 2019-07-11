#ifndef _STREAMASSERTIONS_H_
#define _STREAMASSERTIONS_H_

#include "Assertions.h"

extern int gStreamModuleLevel;
/* 
telnet 24:
log_level_set rtsp 4
logout 1
*/
#define LOG_STRM_ERROR(args...)   LOG_ERROR(HLG_RUNNING, gStreamModuleLevel, args)
#define LOG_STRM_WARN(args...)    LOG_WARNING(HLG_RUNNING, gStreamModuleLevel, args)
#define LOG_STRM_PRINTF(args...)  LOG(HLG_RUNNING, gStreamModuleLevel, args)

#define LOG_STRM_DEBUG(args...) \
do { \
    if (gStreamModuleLevel >= LOG_LEVEL_VERBOSE) \
        logVerboseCStyle(__FILE__, __LINE__, __FUNCTION__, HLG_RUNNING, LOG_LEVEL_VERBOSE, args); \
} while(0)

#define LOG_STRM_ERROUT(args...)  \
do { \
    LOG_ERROR(HLG_RUNNING, gStreamModuleLevel, args); \
    goto Err; \
}while(0)

#define LOG_STRM_WARNOUT(args...)  \
do { \
    LOG_WARNING(HLG_RUNNING, gStreamModuleLevel, args); \
    goto Warn; \
}while(0)

#endif //_STREAMASSERTIONS_H_

