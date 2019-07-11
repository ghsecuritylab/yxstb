#ifndef _MonitorMonitorLogPostFTP_H_
#define _MonitorMonitorLogPostFTP_H_

#include "LogFilter.h"
#include <curl.h>

#include <pthread.h>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef __cplusplus

namespace android {

class MonitorLogPostFTP : public LogFilter {
public:
    MonitorLogPostFTP(const char* server, const char* userPwd, int logType, int logLevel);
    ~MonitorLogPostFTP();
    long fileSize() { if (mWriteFp) return ftell(mWriteFp); else return 0L; }
    void beginUpload();   
    void upload(void* param);
    virtual bool pushBlock(uint8_t* blockHead, uint32_t blockLength);
	int checkLog(const char *input, const int input_len, int log_type, int log_level);
private:
    pthread_mutex_t mMutex;
    FILE* mWriteFp;
    std::string mServer;
    std::string mUserPwd;
	int m_logLevel;
	int m_logType;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _MonitorLogPostFTP_H_
