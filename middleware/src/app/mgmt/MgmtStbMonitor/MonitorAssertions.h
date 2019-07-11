#ifndef _MonitorAssertions_H_
#define _MonitorAssertions_H_

#include "Assertions.h"

extern int g_monitorModuleLevel;

#ifdef __cplusplus
extern "C" {
#endif


#define MONITOR_LOG_ERROR(args...) LOG_ERROR(HLG_OPERATION, g_monitorModuleLevel, args)
#define MONITOR_LOG_WARNING(args...) LOG_WARNING(HLG_OPERATION, g_monitorModuleLevel, args)
#define MONITOR_LOG(args...) LOG(HLG_OPERATION, g_monitorModuleLevel, args)
#define MONITOR_LOG_VERBOSE(args...) LOG_VERBOSE(HLG_OPERATION, g_monitorModuleLevel, args)


#ifdef __cplusplus
};
#endif //__cplusplus


#endif //_MonitorAssertions_H_

