#ifndef _MonitorLog_H_
#define _MonitorLog_H_

#include "LogFilter.h"
#include <iostream>

#ifdef __cplusplus

namespace android {

class MonitorLog : public LogFilter {
public:
    MonitorLog();
    virtual bool pushBlock(uint8_t* blockHead, uint32_t blockLength);
    int writeDebugInfoLog(FILE *logFd);
    int stopDebugInfoLog();
    int closeStartupLogFd();
	int writeStartupLog(FILE *logFd);
	int stopStartupLog();
    
private:
    std::FILE* m_startupLogFd;
    std::FILE* m_debugInfoLogFd;
};

} // namespace Hippo

#endif // __cplusplus
#endif // _MonitorLog_H_

