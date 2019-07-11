#ifndef _TvmsAssertions_H_
#define _TvmsAssertions_H_

#include "Assertions.h"

extern int g_tvmsModuleLevel;

#ifdef __cpluseplus
extern "C" {
#endif

#define TVMS_LOG_ERROR(args...) LOG_ERROR(HLG_OPERATION, g_tvmsModuleLevel, args)
#define TVMS_LOG_WARNING(args...) LOG_WARNING(HLG_OPERATION, g_tvmsModuleLevel, args)
#define TVMS_LOG(args...) LOG(HLG_OPERATION, g_tvmsModuleLevel, args)
#define TVMS_LOG_VERBOSE(args...) LOG_VERBOSE(HLG_OPERATION, g_tvmsModuleLevel, args)

#ifdef __cpluseplus
};
#endif

#endif