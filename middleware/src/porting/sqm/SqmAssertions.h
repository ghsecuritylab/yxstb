#ifndef _SQMASSERTIONS_H_
#define _SQMASSERTIONS_H_

#include "Assertions.h"

extern int gSqmModuleLevel;
/*
telnet 24:
log_level_set sqm 4
logout 1
*/
#define SQM_LOG_ERROR(args...)   LOG_ERROR(HLG_RUNNING, gSqmModuleLevel, args)
#define SQM_LOG_WARN(args...)    LOG_WARNING(HLG_RUNNING, gSqmModuleLevel, args)
#define SQM_LOG(args...)  LOG(HLG_RUNNING, gSqmModuleLevel, args)

#endif // _SQMASSERTIONS_H_
