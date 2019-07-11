#ifndef _MonitorLogPostUDP_H_
#define _MonitorLogPostUDP_H_

#include "LogFilter.h"

#include <netinet/ip.h>

#ifdef __cplusplus

namespace android {

class MonitorLogPostUDP : public LogFilter {
public:
    MonitorLogPostUDP(const char* host, int port, int logLevel, int logType);
    ~MonitorLogPostUDP();

    virtual bool pushBlock(uint8_t* blockHead, uint32_t blockLength);
	int checkLog(const char *input, const int input_len, int log_type, int log_level);

public:
	int stopFlag;
private:
    int m_fd;
    struct sockaddr_in m_serverAddr;
	int m_logLevel;
	int m_logType;
};

} // namespace android

#endif // __cplusplus

#endif // _MonitorLogPostUDP_H_
