#ifndef _LogPostUDP_H_
#define _LogPostUDP_H_

#include "LogFilter.h"

#include <netinet/ip.h>

#ifdef __cplusplus

namespace android {

class LogPostUDP : public LogFilter {
public:
	LogPostUDP(const char* host, int port);
	~LogPostUDP();

	virtual bool pushBlock(uint8_t* blockHead, uint32_t blockLength);
	int log_output_check(const char *input, const int input_len, int log_type, int log_level);
	void logTypeSet(int log_type);
	void logLevelSet(int log_level);

private:
	int m_fd;
	struct sockaddr_in m_serverAddr;
	int log_type;
	int log_level;
};

} // namespace android

#endif // __cplusplus

#endif // _LogPostUDP_H_
