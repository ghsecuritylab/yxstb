#ifndef _UltraPlayerAssertions_H_
#define _UltraPlayerAssertions_H_

#include "Assertions.h"

extern int g_ultraPlayerModuleLevel;

#ifdef __cplusplus

namespace Hippo {

#define PLAYER_LOG_ERROR(args...) LOG_ERROR(HLG_OPERATION, g_ultraPlayerModuleLevel, args)
#define PLAYER_LOG_WARNING(args...) LOG_WARNING(HLG_OPERATION, g_ultraPlayerModuleLevel, args)
#define PLAYER_LOG(args...) LOG(HLG_OPERATION, g_ultraPlayerModuleLevel, args)
#define PLAYER_LOG_VERBOSE(args...) LOG_VERBOSE(HLG_OPERATION, g_ultraPlayerModuleLevel, args)

} // namespace Hippo

#endif //__cplusplus

#endif //_UltraPlayerAssertions_H_

