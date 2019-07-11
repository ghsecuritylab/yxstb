
#ifndef __TR069_DEBUG_H__
#define __TR069_DEBUG_H__

extern int g_tr069LogLevel;

#ifdef ANDROID_LOGCAT_OUTPUT

#include "nm_dbg.h"
#define TR069Debug(X, ...)      do { if (g_tr069LogLevel >= 4) { nm_dbg(LOG_DEBUG, __FILE__, __FUNCTION__, __LINE__,           X, ##__VA_ARGS__); } } while(0)
#define TR069Printf(X, ...)     do { if (g_tr069LogLevel >= 3) { nm_dbg(LOG_DEBUG, __FILE__, __FUNCTION__, __LINE__,           X, ##__VA_ARGS__); } } while(0)
#define TR069Warn(X, ...)       do { if (g_tr069LogLevel >= 2) { nm_dbg(LOG_DEBUG, __FILE__, __FUNCTION__, __LINE__, "WARN! "  X, ##__VA_ARGS__); } } while(0)
#define TR069Error(X, ...)      do { if (g_tr069LogLevel >= 1) { nm_dbg(LOG_DEBUG, __FILE__, __FUNCTION__, __LINE__, "ERROR! " X, ##__VA_ARGS__); } } while(0)
#define TR069WarnOut(X, ...)    do { if (g_tr069LogLevel >= 2) { nm_dbg(LOG_DEBUG, __FILE__, __FUNCTION__, __LINE__, "WARN! "  X, ##__VA_ARGS__); } goto Warn;} while(0)
#define TR069ErrorOut(X, ...)   do { if (g_tr069LogLevel >= 1) { nm_dbg(LOG_DEBUG, __FILE__, __FUNCTION__, __LINE__, "ERROR! " X, ##__VA_ARGS__); } goto Err; } while(0)

#else

#ifdef TR069_INDEPENDS
#define TR069Debug(X, ...)      do { if (g_tr069LogLevel >= 4) { tr069_logPrefix(__FILE__, __LINE__, __FUNCTION__); printf(X, ##__VA_ARGS__); } } while(0)
#define TR069Printf(X, ...)     do { if (g_tr069LogLevel >= 3) { tr069_logPrefix(__FILE__, __LINE__, __FUNCTION__); printf(X, ##__VA_ARGS__); } } while(0)
#define TR069Warn(X, ...)       do { if (g_tr069LogLevel >= 2) { tr069_logPrefix(__FILE__, __LINE__, __FUNCTION__); printf("WARN! ");  printf(X, ##__VA_ARGS__); } } while(0)
#define TR069Error(X, ...)      do { if (g_tr069LogLevel >= 1) { tr069_logPrefix(__FILE__, __LINE__, __FUNCTION__); printf("ERROR! "); printf(X, ##__VA_ARGS__); } } while(0)
#define TR069WarnOut(X, ...)    do { if (g_tr069LogLevel >= 2) { tr069_logPrefix(__FILE__, __LINE__, __FUNCTION__); printf("WARN! ");  printf(X, ##__VA_ARGS__); } goto Warn;} while(0)
#define TR069ErrorOut(X, ...)   do { if (g_tr069LogLevel >= 1) { tr069_logPrefix(__FILE__, __LINE__, __FUNCTION__); printf("ERROR! "); printf(X, ##__VA_ARGS__); } goto Err; } while(0)
#else
#include "../../src/app/Tr069Porting/CPEConfig/TR069Assertions.h"
#define TR069Debug      LogTr069Debug
#define TR069Printf     LogTr069Info
#define TR069Warn       LogTr069Warning
#define TR069Error      LogTr069Error
#define TR069WarnOut(args...)   do { LOG_WARNING(HLG_OPERATION, gTr069ModuleLevel, args); goto Warn;} while(0)
#define TR069ErrorOut(args...)  do { LOG_ERROR(HLG_OPERATION, gTr069ModuleLevel, args);   goto Err; } while(0)
#endif//TR069_INDEPENDS

#endif//ANDROID_LOGCAT_OUTPUT

#endif//__TR069_DEBUG_H__
