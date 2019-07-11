#ifndef _JseAssertions_H_
#define _JseAssertions_H_

#include "Assertions.h"

#ifdef __cplusplus

extern int gJseModuleLevel;

#define LogJseError(args...)   LOG_ERROR(HLG_RUNNING, gJseModuleLevel, args)
#define LogJseWarning(args...) LOG_WARNING(HLG_RUNNING, gJseModuleLevel, args)
#define LogJseInfo(args...)    LOG(HLG_RUNNING, gJseModuleLevel, args)
#define LogJseDebug(args...)   LOG(HLG_RUNNING, gJseModuleLevel, args)

#endif //__cplusplus

#endif //_JseAssertions_H_

