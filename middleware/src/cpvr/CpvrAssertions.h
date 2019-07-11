#ifndef _CPVRASSERTIONS_H_
#define _CPVRASSERTIONS_H_

#include "Assertions.h"

#ifdef __cplusplus

extern int gCpvrModuleLevel;

namespace Hippo {

#define LogCpvrError(args...)   LOG_ERROR(HLG_OPERATION, gCpvrModuleLevel, args)
#define LogCpvrWarn(args...)    LOG_WARNING(HLG_OPERATION, gCpvrModuleLevel, args)
#define LogCpvrDebug(args...)   LOG(HLG_OPERATION, gCpvrModuleLevel, args)

} // namespace Hippo

#endif //__cplusplus

#endif //_CPVRAssertions_H_

