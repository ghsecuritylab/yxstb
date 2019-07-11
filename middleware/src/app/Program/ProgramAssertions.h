#ifndef _ProgramAssertions_H_
#define _ProgramAssertions_H_

#include "Assertions.h"

extern int g_programModuleLevel;

#ifdef __cplusplus

namespace Hippo {

#define PROGRAM_LOG_ERROR(args...) LOG_ERROR(HLG_OPERATION, g_programModuleLevel, args)
#define PROGRAM_LOG_WARNING(args...) LOG_WARNING(HLG_OPERATION, g_programModuleLevel, args)
#define PROGRAM_LOG(args...) LOG(HLG_OPERATION, g_programModuleLevel, args)
#define PROGRAM_LOG_VERBOSE(args...) LOG_VERBOSE(HLG_OPERATION, g_programModuleLevel, args)

} // namespace Hippo

#endif //__cplusplus

#endif //_ProgramAssertions_H_
