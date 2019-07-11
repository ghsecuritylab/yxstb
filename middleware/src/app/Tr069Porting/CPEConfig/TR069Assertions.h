#ifndef _TR069ASSERTIONS_H_
#define _TR069ASSERTIONS_H_

#include "Assertions.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int gTr069ModuleLevel;

#define LogTr069Error(args...)   LOG_ERROR(HLG_OPERATION, gTr069ModuleLevel, args)
#define LogTr069Warning(args...) LOG_WARNING(HLG_OPERATION, gTr069ModuleLevel, args)
#define LogTr069Info(args...)    LOG(HLG_OPERATION, gTr069ModuleLevel, args)
#define LogTr069Debug(args...)   LOG(HLG_OPERATION, gTr069ModuleLevel, args)

#ifdef __cplusplus
};
#endif //__cplusplus

#endif //_TR069ASSERTIONS_H_

