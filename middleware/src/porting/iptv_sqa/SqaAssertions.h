#ifndef _SQAASSERTIONS_H_
#define _SQAASSERTIONS_H_

#include "Assertions.h"

extern int gSqaModuleLevel;
/* 
telnet 24:
log_level_set sqa 4
logout 1
*/
#define LOG_SQA_ERROR(args...)   LOG_ERROR(HLG_RUNNING, gSqaModuleLevel, args)
#define LOG_SQA_WARN(args...)    LOG_WARNING(HLG_RUNNING, gSqaModuleLevel, args)
#define LOG_SQA_PRINTF(args...)  LOG(HLG_RUNNING, gSqaModuleLevel, args)

#define LOG_SQA_ERROUT(args...)  \
do { \
    LOG_ERROR(HLG_RUNNING, gSqaModuleLevel, args); \
    goto Err; \
}while(0)

#define LOG_SQA_WARNOUT(args...)  \
do { \
    LOG_WARNING(HLG_RUNNING, gSqaModuleLevel, args); \
    goto Err; \
}while(0)
#endif //_STREAMASSERTIONS_H_

