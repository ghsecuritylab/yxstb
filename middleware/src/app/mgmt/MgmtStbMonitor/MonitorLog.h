#ifndef _MonitorLog_H_
#define _MonitorLog_H_

#include "LogFilter.h"
#include <iostream>

#ifdef __cplusplus

namespace Hippo {

class MonitorLog : public LogFilter {
public:
    MonitorLog();
    virtual bool pushBlock(uint8_t* blockHead, uint32_t blockLength);
    int writeDebugInfoLog(FILE *logFd);
    int stopDebugInfoLog();
    int closeStartupLogFd();
    
private:
    FILE* m_startupLogFd;
    FILE* m_debugInfoLogFd;
};

} // namespace Hippo

#endif // __cplusplus
#endif // _MonitorLog_H_

