#ifndef _LogPostFTP_H_
#define _LogPostFTP_H_

#include "LogFilter.h"
#include "curl/curl.h"

#include <pthread.h>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef __cplusplus

namespace Hippo {

class LogPostFTP : public LogFilter {
public:
    LogPostFTP(const char* server, const char* userPwd);
    ~LogPostFTP();
    long fileSize() { if (mWriteFp) return ftell(mWriteFp); else return 0L; }
    void beginUpload();   
    static void upload(void* param);
    virtual bool pushBlock(uint8_t* blockHead, uint32_t blockLength);
private:
    pthread_mutex_t mMutex;
    FILE* mWriteFp;
    std::string mServer;
    std::string mUserPwd;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _LogPostFTP_H_
