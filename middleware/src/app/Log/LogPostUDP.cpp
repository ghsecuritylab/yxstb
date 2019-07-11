
#include "LogPostUDP.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>


namespace Hippo {

LogPostUDP::LogPostUDP(const char* host, int port)
	: m_fd(-1)
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

bool 
LogPostUDP::pushBlock(uint8_t* blockHead, uint32_t blockLength)
{
    if (m_fd != -1) {
        sendto(m_fd, blockHead, blockLength, 0, (struct sockaddr*)&m_serverAddr, sizeof(m_serverAddr));
    }
    return false;
}

} // namespace Hippo
