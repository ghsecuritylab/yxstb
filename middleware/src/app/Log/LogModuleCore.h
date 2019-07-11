#ifndef _LogModuleCore_H_
#define _LogModuleCore_H_

#include "Assertions.h"

extern int g_systemModuleLevel;

#define SYSTEM_LOG_ERROR(args...)   LOG_ERROR(HLG_RUNNING, g_systemModuleLevel, args)
#define SYSTEM_LOG_WARNING(args...) LOG_WARNING(HLG_RUNNING, g_systemModuleLevel, args)
#define SYSTEM_LOG(args...)         LOG(HLG_RUNNING, g_systemModuleLevel, args)
#define SYSTEM_LOG_VERBOSE(args...) LOG_VERBOSE(HLG_RUNNING, g_systemModuleLevel, args)

extern int g_runningModuleLevel;

#define RUNNING_LOG_ERROR(args...)   LOG_ERROR(HLG_RUNNING, g_runningModuleLevel, args)
#define RUNNING_LOG_WARNING(args...) LOG_WARNING(HLG_RUNNING, g_runningModuleLevel, args)
#define RUNNING_LOG(args...)         LOG(HLG_RUNNING, g_runningModuleLevel, args)
#define RUNNING_LOG_VERBOSE(args...) LOG_VERBOSE(HLG_RUNNING, g_runningModuleLevel, args)

#endif // _LogModuleCore_H_
