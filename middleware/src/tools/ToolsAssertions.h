#ifndef _TOOLSASSERTIONS_H_
#define _TOOLSASSERTIONS_H_

#include "Assertions.h"

#ifdef __cplusplus

extern int gToolsModuleLevel;

namespace Hippo {

#define LogToolsError(args...)   LOG_ERROR(HLG_USER, gToolsModuleLevel, args)
#define LogToolsWarn(args...) LOG_WARNING(HLG_USER, gToolsModuleLevel, args)
#define LogToolsDebug(args...)   LOG(HLG_USER, gToolsModuleLevel, args)

} // namespace Hippo

#endif //__cplusplus

#endif //_ToolsAssertions_H_

