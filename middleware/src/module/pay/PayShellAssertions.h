#ifndef _PAYSHELLASSERTIONS_H_
#define _PAYSHELLASSERTIONS_H_

#include "Assertions.h"

extern int gPayShellModuleLevel;
    
#define LogPayShellError(args...)   LOG_ERROR(HLG_RUNNING, gPayShellModuleLevel, args)
#define LogPayShellWarning(args...) LOG_WARNING(HLG_RUNNING, gPayShellModuleLevel, args)
#define LogPayShellInfo(args...)    LOG(HLG_RUNNING, gPayShellModuleLevel, args)
#define LogPayShellDebug(args...)   LOG(HLG_RUNNING, gPayShellModuleLevel, args)

#define PayShellErrOut(args...) \
do{                                     \
    LOG_ERROR(HLG_RUNNING, gPayShellModuleLevel, args);  \
    goto Err;                           \
} while(0)

#endif // ifndef _PAYSHELLASSERTIONS_H_

