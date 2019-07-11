#include "MonitorToolLog.h"

#if defined(ANDROID)
#include "cutils/log.h"
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "IPTVMiddleware"
#endif

#include <stdio.h>
#include <string.h>

extern android::MonitorLog* g_logMonitor;

namespace android {
MonitorLog::MonitorLog()
{
    m_startupLogFd   = NULL;
    m_debugInfoLogFd = NULL;
}

bool MonitorLog::pushBlock(uint8_t* blockHead, uint32_t blockLength)
{       
	if (m_startupLogFd) {
        printf("=====get startup log======\n");
        fwrite(blockHead, 1, blockLength, m_startupLogFd);
        fflush(m_startupLogFd);
    }

    
    if (m_debugInfoLogFd) {
        printf("=====get debug log======\n");
        fwrite(blockHead, 1, blockLength, m_debugInfoLogFd);
        fflush(m_debugInfoLogFd);
    }
    
    return false;
}

int MonitorLog::writeStartupLog(FILE *logFd)
{
    if (!logFd)
        return -1;
		
    m_startupLogFd = logFd;
	
	return 0;
}

int MonitorLog::stopStartupLog()
{
	if (m_startupLogFd) {
		fclose(m_startupLogFd);
		m_startupLogFd = NULL;
	}

    return 0;
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

} // namespace Hippo

extern "C"
{
int startupLog(FILE *logFd)
{
	if (g_logMonitor)
		g_logMonitor->writeStartupLog(logFd);
	return 0;
}

int stopStartupLog()
{
	if (g_logMonitor)
		g_logMonitor->stopStartupLog();
    return 0;
}

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
    
}
