
#ifndef __TR069_OS_LINUX_H__
#define __TR069_OS_LINUX_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/igmp.h>

#include <arpa/inet.h>

#include <netdb.h>

#include <errno.h>
#include <unistd.h>

#define SOCKET			int
#define INVALID_SOCKET	-1
#define SOCKET_ERROR	-1

typedef long long int64;

extern int (*log_printf)(const char *fmt, ...);

#define	stricmp		strcasecmp
#define	strnicmp	strncasecmp

#define	closesocket	close

#endif //__TR069_OS_LINUX_H__
