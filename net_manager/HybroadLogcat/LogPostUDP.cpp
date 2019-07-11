
#include "LogPostUDP.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>

#include "nm_dbg.h"


namespace android {

LogPostUDP::LogPostUDP(const char* host, int port)
	: m_fd(-1),log_type(0),log_level(0)
{
    struct hostent* he;
    he = gethostbyname(host);
    if (he) {
        memset((void*)&m_serverAddr, 0, sizeof(m_serverAddr));
        m_serverAddr.sin_family = AF_INET;
        m_serverAddr.sin_port = htons(port);
        m_serverAddr.sin_addr = *((struct in_addr *)he->h_addr);

        m_fd = socket(AF_INET, SOCK_DGRAM, 0);
    }
}

LogPostUDP::~LogPostUDP()
{
    close(m_fd);
}

int LogPostUDP::log_output_check(const char *input, const int input_len, int log_type, int log_level)
{
	int log_info = 0;
	int error_code = 0;
	int input_type = 0;
	
	if(input_len > 5){
		if(input[0] == '<' && input[4] == '>' ){
			log_info = 100 *(input[1] - '0') + 10 * (input[2] - '0') + input[3] - '0';
			error_code = log_info % 8;
			switch(error_code){
				case 3: //error
				case 6: //informational
				case 7://debug
				if(log_level < error_code) return 0;
				break;
				default: //unknown
				return 0;
				break;
			}
			if(log_type != 0){
				input_type = log_info - error_code;
				if(input_type  != log_type * 8) return 0;
			}
			return 1;
		}
	}
	return 0;
}


bool LogPostUDP::pushBlock(uint8_t* blockHead, uint32_t blockLength)
{
	int ret = 0;
	if (m_fd != -1) {
		#if 1
		if(ret = log_output_check((const char *)blockHead, (const unsigned int)blockLength, log_type, log_level)){
			//printf("send data:%s\n", (char *)cache_buf);
			sendto(m_fd, blockHead, blockLength, 0, (struct sockaddr*)&m_serverAddr, sizeof(m_serverAddr));
		}  else {
			//printf("invalid data:%s\n", (char *)blockHead);
		}
		#else
		sendto(m_fd, blockHead, blockLength, 0, (struct sockaddr*)&m_serverAddr, sizeof(m_serverAddr));
		#endif
	}
	return false;
}

void LogPostUDP::logTypeSet(int type)
{
	log_type = type;
}
void LogPostUDP::logLevelSet(int level)
{
	log_level = level;
}

} // namespace android
