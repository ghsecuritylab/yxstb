#ifndef _LogPostUDP_H_
#define _LogPostUDP_H_

#include "LogFilter.h"

#include <netinet/ip.h>

#ifdef __cplusplus

namespace Hippo {

class LogPostUDP : public LogFilter {
public:
    LogPostUDP(const char* host, int port);
    ~LogPostUDP();

    virtual bool pushBlock(uint8_t* blockHead, uint32_t blockLength);

private:
    int m_fd;
    struct sockaddr_in m_serverAddr;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _LogPostUDP_H_
