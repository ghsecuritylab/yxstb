#ifndef __NetworkAssertions__H_
#define __NetworkAssertions__H_

#include "Assertions.h"

#ifdef __cplusplus

extern int gNetworkModuleLevel;

#define NETWORK_LOG_ERR(args...)    LOG_ERROR(HLG_RUNNING, gNetworkModuleLevel, args)
#define NETWORK_LOG_WARN(args...)   LOG_WARNING(HLG_RUNNING, gNetworkModuleLevel, args)
#define NETWORK_LOG_INFO(args...)   LOG(HLG_RUNNING, gNetworkModuleLevel, args)

#endif //__cplusplus

#endif
