
#include "MonitorLog.h"
#include "mgmtModuleParam.h"

#if defined(ANDROID)
#include "cutils/log.h"
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "IPTVMiddleware"
#endif

#include <stdio.h>
#include <string.h>

extern int g_startupCapFlag;
#ifdef ANDROID
extern char g_startupLogAddr[100];
#else
extern char g_StartupLogFile[100];
#endif
extern Hippo::MonitorLog* g_logMonitor;
static int noCreateLogFile = 1;

namespace Hippo {
MonitorLog::MonitorLog()
{
    m_startupLogFd   = NULL;
    m_debugInfoLogFd = NULL;
}

bool MonitorLog::pushBlock(uint8_t* blockHead, uint32_t blockLength)
{

    if (g_startupCapFlag) {
        #if defined(ANDROID)
            if ((strlen(g_startupLogAddr) != 0) && (noCreateLogFile == 1)) {
                m_startupLogFd = fopen(g_startupLogAddr, "w");
                if (m_startupLogFd)
                   printf("logfile create ok\n");
                noCreateLogFile = 0;
            }
            android_writeLog(ANDROID_LOG_INFO, LOG_TAG, (const char*)blockHead);
        #else
            if ((strlen(g_StartupLogFile) != 0) && (noCreateLogFile == 1)) {
                m_startupLogFd = fopen(g_StartupLogFile, "w");
                if (m_startupLogFd)
                   printf("logfile create ok\n");
                noCreateLogFile = 0;
            }
            if (m_startupLogFd) {
                fwrite(blockHead, 1, blockLength, m_startupLogFd);
                fflush(m_startupLogFd);
            }
        #endif

    } else if (m_startupLogFd) {
        fclose(m_startupLogFd);
        m_startupLogFd = NULL;
    }


    if (m_debugInfoLogFd) {
        printf("=====get log======\n");
        fwrite(blockHead, 1, blockLength, m_debugInfoLogFd);
        fflush(m_debugInfoLogFd);
    }

    return false;
}

int MonitorLog::writeDebugInfoLog(FILE *logFd)
{
    if (!logFd)
        return -1;

    m_debugInfoLogFd = logFd;
    return 0;
}

int MonitorLog::stopDebugInfoLog()
{
    printf("=====stop log======\n");
    if (m_debugInfoLogFd) {
        fclose(m_debugInfoLogFd);
        m_debugInfoLogFd = NULL;
    }

    return 0;
}

int MonitorLog::closeStartupLogFd()
{
    printf("stop StartupLogFd \n");
    if (m_startupLogFd) {
        fclose(m_startupLogFd);
        m_startupLogFd = NULL;
        printf("ok\n");
    }
    return 0;
}

} // namespace Hippo

extern "C"
{
int writeLog(FILE *logFd)
{
    if (g_logMonitor)
        g_logMonitor->writeDebugInfoLog(logFd);
    return 0;
}

int stopLog()
{
    if (g_logMonitor)
        g_logMonitor->stopDebugInfoLog();
    return 0;
}

int stopStartupLog()
{
    if (g_logMonitor)
        g_logMonitor->closeStartupLogFd();
    return 0;
}

}
