#ifndef _BrowserAssertions_H_
#define _BrowserAssertions_H_

#include "Assertions.h"

extern int g_browserModuleLevel;

#ifdef __cplusplus

namespace Hippo {

#define BROWSER_LOG_ERROR(args...) LOG_ERROR(HLG_OPERATION, g_browserModuleLevel, args)
#define BROWSER_LOG_WARNING(args...) LOG_WARNING(HLG_OPERATION, g_browserModuleLevel, args)
#define BROWSER_LOG(args...) LOG(HLG_OPERATION, g_browserModuleLevel, args)
#define BROWSER_LOG_VERBOSE(args...) LOG_VERBOSE(HLG_OPERATION, g_browserModuleLevel, args)

} // namespace Hippo

#endif //__cplusplus

#endif //_BrowserAssertions_H_
