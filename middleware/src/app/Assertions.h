#ifndef Assertions_h
#define Assertions_h

#include <stdio.h>
#include <stdint.h>


#define NDEBUG

#define CRASH() do { \
    *(int *)(uintptr_t)0xbbadbeef = 0; \
    ((void(*)())0)(); /* More reliable, but doesn't say BBADBEEF */ \
} while(false)

#ifdef NDEBUG
#define ASSERT(assertion) ((void)0)
#else
#define ASSERT(assertion) do \
    if (!(assertion)) { \
        logVerboseCStyle(__FILE__, __LINE__, __FUNCTION__, HLG_RUNNING, 0, "%s\n", #assertion); \
        CRASH(); \
    } \
while (0)
#endif

enum LogLevel {
    LOG_LEVEL_ERROR   = 0x01,
    LOG_LEVEL_WARNING = 0x02,
    LOG_LEVEL_NORMAL  = 0x03,
    LOG_LEVEL_VERBOSE = 0x04
};

typedef enum _LogType {
	HLG_OPERATION  = 0x01,
	HLG_RUNNING    = 0x02,
	HLG_SECURITY   = 0x04,
	HLG_USER       = 0x08,
	HLG_ALL        = HLG_OPERATION | HLG_RUNNING | HLG_SECURITY | HLG_USER
}LogType;

#define LOG_ERROR(pModule, flag, args...) \
do { \
    if (flag >= LOG_LEVEL_ERROR) \
        logVerboseCStyle(__FILE__, __LINE__, __FUNCTION__, pModule, LOG_LEVEL_ERROR, args); \
} while(0)

#define LOG_WARNING(pModule, flag, args...) \
do { \
    if (flag >= LOG_LEVEL_WARNING) \
        logVerboseCStyle(__FILE__, __LINE__, __FUNCTION__, pModule, LOG_LEVEL_WARNING, args); \
} while(0)

#define LOG(pModule, flag, args...) \
do { \
    if (flag >= LOG_LEVEL_NORMAL) \
        logVerboseCStyle(__FILE__, __LINE__, __FUNCTION__, pModule, LOG_LEVEL_NORMAL, args); \
} while(0)

#ifdef NDEBUG
#define LOG_VERBOSE(pModule, flag, args...) ((void)0)
#else
#define LOG_VERBOSE(pModule, flag, args...) \
do { \
    if (flag >= LOG_LEVEL_VERBOSE) \
        logVerboseCStyle(__FILE__, __LINE__, __FUNCTION__, pModule, LOG_LEVEL_VERBOSE, args); \
} while(0)
#endif

/* huawei: operation log */
extern int gOperModuleLevel;
#define LogSysOperError(args...) LOG_ERROR(HLG_OPERATION, gOperModuleLevel, args)
#define LogSysOperWarn(args...) LOG_WARNING(HLG_OPERATION, gOperModuleLevel, args)
#define LogSysOperDebug(args...) LOG(HLG_OPERATION, gOperModuleLevel, args)

/* huawei: running log */
extern int gRunModuleLevel;
#define LogRunOperError(args...) LOG_ERROR(HLG_RUNNING, gRunModuleLevel, args)
#define LogRunOperWarn(args...) LOG_WARNING(HLG_RUNNING, gRunModuleLevel, args)
#define LogRunOperDebug(args...) LOG(HLG_RUNNING, gRunModuleLevel, args)

/* huawei: security log */
extern int gSafeModuleLevel;
#define LogSafeOperError(args...) LOG_ERROR(HLG_SECURITY, gSafeModuleLevel, args)
#define LogSafeOperWarn(args...) LOG_WARNING(HLG_SECURITY, gSafeModuleLevel, args)
#define LogSafeOperDebug(args...) LOG(HLG_SECURITY, gSafeModuleLevel, args)

/* huawei: user log */
extern int gUserModuleLevel;
#define LogUserOperError(args...) LOG_ERROR(HLG_USER, gUserModuleLevel, args)
#define LogUserOperWarn(args...) LOG_WARNING(HLG_USER, gUserModuleLevel, args)
#define LogUserOperDebug(args...) LOG(HLG_USER, gUserModuleLevel, args)

#if defined (DEBUG_BUILD)

#define LOG_PRINTF	log_printf
#define PRINTF(args...)      LOG(HLG_RUNNING, gUserModuleLevel, args)
#define DBG_PRN(args...)     LOG(HLG_RUNNING, gUserModuleLevel, args)
#define WARN_PRN(args...)    LOG_WARNING(HLG_RUNNING, gUserModuleLevel, args)
#define ERR_PRN(args...)     LOG_ERROR(HLG_RUNNING, gUserModuleLevel, args)
#define WARN_OUT(args...) \
do{                                     \
    LOG_WARNING(HLG_RUNNING, gUserModuleLevel, args);  \
    goto Warn;                           \
} while(0)
#define ERR_OUT(args...) \
do{                                     \
    LOG_ERROR(HLG_RUNNING, gUserModuleLevel, args);  \
    goto Err;                           \
} while(0)
#define END_OUT(args...) \
do{                                 \
    LOG(HLG_RUNNING, gUserModuleLevel, args);    \
    goto End;                       \
}while(0)

#else 
#define PRINTF(args...)     
#define PRINTF_ONLY(args...)
#define DBG_PRN(args...)    
#define WARN_PRN(args...)   
#define ERR_PRN(args...)    
#define WARN_OUT(args...) \
    goto Warn;
#define ERR_OUT(args...) \
    goto Err;
#define END_OUT(args...) \
    goto End;
#endif

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void logInit();

void logSetExtensionStyle(int value);
void logCStyle(int level, const char* fmt, ...);
void logVerboseCStyle(const char* file, int line, const char* function, LogType pType, int level, const char* fmt, ...);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* Assertions_h */
