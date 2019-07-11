#ifndef _JseGlueASSERTIONS_H_
#define _JseGlueASSERTIONS_H_

#include "Assertions.h"

extern int gJseGlueModuleLevel;
#define LogJseGlueDebug(args...)   LOG(HLG_RUNNING, gJseGlueModuleLevel, args)
#define LogJseGlueWarn(args...)    LOG_WARNING(HLG_RUNNING, gJseGlueModuleLevel, args)
#define LogJseGlueError(args...)   LOG_ERROR(HLG_RUNNING, gJseGlueModuleLevel, args)
#define LogJseGlueTest(args...)    LOG(HLG_RUNNING, gJseGlueModuleLevel, args)

#endif //_JseGlueASSERTIONS_H_
