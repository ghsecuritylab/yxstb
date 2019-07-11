#include "Log.h"
#include "Assertions.h"

#include "RingBuffer.h"
#include "LogPool.h"
#include "LogPrinter.h"
#include "LogModule.h"
#include "SysTime.h"
#include "MonitorLog.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define LOG_BUFFER_SIZE	128 * 1024


/* huawei: operation log */
static int s_OperModuleFlag = 0;
int gOperModuleLevel = LOG_LEVEL_WARNING;
static LogModule LogSysOperModule("hwOper", s_OperModuleFlag, gOperModuleLevel);

/* huawei: running log */
static int s_RunModuleFlag = 0;
int gRunModuleLevel = LOG_LEVEL_WARNING;
static LogModule LogRunOperModule("hwRun", s_RunModuleFlag, gRunModuleLevel);

/* huawei: security log */
static int s_SafeModuleFlag = 0;
int gSafeModuleLevel = LOG_LEVEL_WARNING;
static LogModule LogSafeOperModule("hwSafe", s_SafeModuleFlag, gSafeModuleLevel);

/* huawei: user log */
static int s_UserModuleFlag = 0;
int gUserModuleLevel = LOG_LEVEL_WARNING;
static LogModule LogUserOperModule("hwUser", s_UserModuleFlag, gUserModuleLevel);


static uint8_t* g_logBuffer = NULL;
static Hippo::RingBuffer* g_ringBuffer = NULL;

static Hippo::Log g_log;

Hippo::LogPool* g_logPool = NULL;
Hippo::LogPrinter* g_logPrinter = NULL;
Hippo::MonitorLog* g_logMonitor = NULL;

extern "C" 
void logInit()
{
    g_logBuffer = (uint8_t*)malloc(LOG_BUFFER_SIZE);
    g_ringBuffer = new Hippo::RingBuffer(g_logBuffer, LOG_BUFFER_SIZE);

    g_logPool = new Hippo::LogPool();
    g_logPool->setBuffer(g_ringBuffer);

    g_logPrinter = new Hippo::LogPrinter();
    g_logPool->attachFilter(g_logPrinter, 0);

    g_log.setBuffer(g_ringBuffer);
    g_log.attachSink(g_logPool);

    g_logMonitor = new Hippo::MonitorLog();
    g_logPool->attachFilter(g_logMonitor, 0);
    
    logModuleInit();
}

extern "C" 
void logSetExtensionStyle(int value)
{
    g_log.setExtensionStyle(value);
}

extern "C" 
void logCStyle(int level, const char* fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    g_log.log(level, fmt, args);
    va_end(args);
}

extern "C" 
void logVerboseCStyle(const char* file, int line, const char* function, LogType pType, int level, const char* fmt, ...)
{
    va_list args;

#if defined(DEBUG_BUILD)
    static const char* textLevel[] = {"Assert : ", "Error! : ", "Warning: ", "Normal : ", "Verbose: "};
    if (2 == g_log.getExtensionStyle()) {
        static Hippo::SysTime::DateTime sDTime;
        static struct timespec sTimeSpec;
        static char sLogBuffer[4096] = { 0 };
        Hippo::SysTime::GetDateTime(&sDTime);
        clock_gettime(CLOCK_MONOTONIC, &sTimeSpec);
        va_start(args, fmt);
        vsnprintf(sLogBuffer, 4088, fmt, args);
        va_end(args);
        printf("%02d:%02d:%02d.%03d | %s:%d | %s %s %s", sDTime.mHour, sDTime.mMinute, sDTime.mSecond, sTimeSpec.tv_nsec / 1000000, strrchr(file, '/') + 1, line, function, textLevel[level], sLogBuffer);
        return;
    }
#endif

    va_start(args, fmt);
    g_log.logVerbose(file, line, function, pType, level, fmt, args);
    va_end(args);
}
