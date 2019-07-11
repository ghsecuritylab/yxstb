
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <arpa/inet.h>

#include "app/Assertions.h"
#include "ind_mem.h"
#include "ind_net.h"

static IndRouteCall *gRouteCall = NULL;

#if ENABLE_IPV6

#define INT_ADDR_LEN	16//IPv4

uint32_t ind_net_index(ind_sin_t sin)
{
	uint32_t idx;

	if (sin->family == AF_INET6) {
		idx = sin->in6_addr.s6_addr[15];
	} else {
		u_char *up = (u_char *)&sin->in_addr.s_addr;
		idx =  (int)((uint32_t)up[3]);
	}

	return idx;
}

int ind_net_equal(ind_sin_t sin1, ind_sin_t sin2)
{
	if (sin1 == NULL || sin2 == NULL)
		return 0;
	if (sin1->family != sin2->family)
		return 0;
	if (sin1->family == AF_INET6) {
		if (memcmp(&sin1->in6_addr, &sin2->in6_addr, sizeof(sin1->in6_addr)))
			return 0;
	} else {
		if (sin1->in_addr.s_addr != sin2->in_addr.s_addr)
			return 0;
	}

	if (sin1->port == sin2->port)
		return 2;

	return 1;
}

int ind_net_ntop(ind_sin_t sin, char *buf, int len)
{
	int l;
	char *p;

	if (sin == NULL || buf == NULL)
		ERR_OUT("sin = %p, buf = %p\n", sin, buf);

	if (sin->family == AF_INET6) {
		if (len < IND_ADDR_LEN)
			ERR_OUT("len = %d\n", len);

		buf[0] = '[';
		p = (char*)inet_ntop(AF_INET6, &sin->in6_addr, buf + 1, IND_ADDR_LEN - 2);
		if (p == NULL)
			ERR_OUT("inet_ntop AF_INET6 %s\n", strerror(errno));
		l = strlen(buf);
		buf[l] = ']';
		l ++;
		buf[l] = 0;
	} else {
		if (len < INT_ADDR_LEN)
			ERR_OUT("len = %d\n", len);
		p = (char*)inet_ntop(AF_INET, &sin->in_addr, buf, IND_ADDR_LEN);
		if (p == NULL)
			ERR_OUT("inet_ntop AF_INET %s\n", strerror(errno));
	}

	return strlen(buf);
Err:
	return 0;
}

int ind_net_pton(char* url, ind_sin_t sin)
{
	int r, i;
	char *p;
	char buf[IND_ADDR_LEN];

	if (url == NULL || sin == NULL)
		ERR_OUT("url = %p, sin = %p\n", url, sin);

	p = url;
	i = 0;
	sin->port = 0;
	if ('[' == url[0]) {
		sin->family = AF_INET6;
		i ++;
		while(isxdigit(p[i]) || p[i] == ':')
			i ++;
	} else {
		sin->family = AF_INET;
		while(isdigit(p[i]) || p[i] == '.')
			i ++;
	}

	if (AF_INET6 == sin->family) {
		if (i > 40)
			ERR_OUT("addr6 len = %d\n", i);
		if (']' != p[i])
			ERR_OUT("addr6 end! len = %d\n", i);
		IND_MEMCPY(buf, p + 1, i - 1);
		buf[i - 1] = 0;
		i ++;

		r = inet_pton(AF_INET6, buf, &sin->in6_addr);
		if (r != 1)
			ERR_OUT("inet_pton AF_INET6 '%s' r = %d\n", buf, r);
		sin->in_addr.s_addr = 0;
	} else {
		if (i > 15)
			ERR_OUT("addr len = %d\n", i);
		IND_MEMCPY(buf, p, i);
		buf[i] = 0;

		r = inet_pton(AF_INET, buf, &sin->in_addr);
		if (r != 1)
			ERR_OUT("inet_pton AF_INET r = %d\n", r);
	}

	p += i;
	if (*p != ':') {
		sin->port = 0;
		return (int)(p - url);
	}

	p ++;
	i = 0;
	while(isdigit(p[i]))
		i ++;
	if (i == 0 || i > 5)
		ERR_OUT("port len = %d\n", i);
	IND_MEMCPY(buf, p, i);
	buf[i] = 0;
	sin->port = (unsigned short)atoi(buf);
	p += i;

	return (int)(p - url);
Err:
	return -1;
}

#else//!ENABLE_IPV6

uint32_t ind_net_index(ind_sin_t sin)
{
	u_char *up;
	uint32_t idx;

	up = (u_char *)&sin->in_addr.s_addr;
	idx =  (int)((uint32_t)up[3]);

	return idx;
}

int ind_net_equal(ind_sin_t sin1, ind_sin_t sin2)
{
	if (sin1 == NULL || sin2 == NULL)
		return 0;
	if (memcmp(&sin1->in_addr, &sin2->in_addr, sizeof(sin1->in_addr)) == 0) {
		if (sin1->port == sin2->port)
			return 2;
		return 1;
	}
	return 0;
}

int ind_net_ntop(ind_sin_t sin, char *buf, int len)
{
	unsigned char *p;

	if (buf == NULL || len < IND_ADDR_LEN)
		ERR_OUT("buf = %p, len = %d\n", buf, len);
	p = (unsigned char *)(&sin->in_addr.s_addr);
	return sprintf(buf, "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
Err:
	return -1;
}

int ind_net_pton(char* url, ind_sin_t sin)
{
	int i;
	char *p;
	char buf[16];
	unsigned int addr;

	sin->family = AF_INET;

	p = url;
	i = 0;
	while(isdigit(p[i]) || p[i] == '.')
		i ++;
	if (i == 0 || i > 15)
		ERR_OUT("addr len = %d\n", i);
	IND_MEMCPY(buf, p, i);
	buf[i] = 0;
	addr = inet_addr(buf);
	if (addr == INADDR_ANY || addr == INADDR_NONE)
		ERR_OUT("addr = 0x%x\n", addr);

	sin->in_addr.s_addr = addr;

	p += i;
	if (*p != ':') {
		sin->port = 0;
		return (int)(p - url);
	}

	p ++;
	i = 0;
	while(isdigit(p[i]))
		i ++;
	if (i == 0 || i > 5)
		ERR_OUT("port len = %d\n", i);
	IND_MEMCPY(buf, p, i);
	buf[i] = 0;
	sin->port = (unsigned short)atoi(buf);
	p += i;

	return (int)(p - url);
Err:
	return -1;
}
#endif//ENABLE_IPV6

int ind_net_ntop_ex(ind_sin_t sin, char *buf, int len)
{
	int l;

	if (sin == NULL || buf == NULL)
		ERR_OUT("sin = %p, buf = %p\n", sin, buf);

	l = ind_net_ntop(sin, buf, len);
	if (l <= 0)
		ERR_OUT("ind_net_ntop len = %d\n", len);

	if (l + 7 > len)
		ERR_OUT("l = %d, len = %d\n", l, len);
	l += sprintf(buf + l, ":%hu", sin->port);

	return l;
Err:
	return 0;
}

int ind_net_connect(int sock, struct ind_sin *sin)
{
	if (sock < 0 || sin == NULL)
		ERR_OUT("sock = %d, sin = %p\n", sock, sin);

#ifdef ENABLE_IPV6
	if (sin->family == AF_INET6) {
		struct sockaddr_in6 sa;

		memset(&sa, 0, sizeof(sa));
		sa.sin6_family = AF_INET6;
		sa.sin6_addr = sin->in6_addr;
		sa.sin6_port = htons(sin->port);

		connect(sock, (struct sockaddr*)&sa, sizeof(sa));
	} else
#endif
	{
		struct sockaddr_in sa;

		memset(&sa, 0, sizeof(sa));
		sa.sin_family = AF_INET;
		sa.sin_addr = sin->in_addr;
		sa.sin_port = htons(sin->port);
        if (gRouteCall)
            gRouteCall(sa.sin_addr.s_addr);
		if (connect(sock, (struct sockaddr*)&sa, sizeof(sa)) && EINPROGRESS != errno)
			ERR_OUT("errno = %d / %s\n", errno, strerror(errno));
	}

	return 0;
Err:
	return -1;
}

int ind_net_bind(int sock, struct ind_sin *sin)
{
	if (sock < 0)
		ERR_OUT("sock = %d\n", sock);

#ifdef ENABLE_IPV6
	if (sin->family == AF_INET6) {
		struct sockaddr_in6 sa;

		memset(&sa, 0, sizeof(sa));
		sa.sin6_family = AF_INET6;
		sa.sin6_addr = sin->in6_addr;
		sa.sin6_port = htons(sin->port);

		if (bind(sock, (struct sockaddr*)&sa, sizeof(sa)))
			ERR_OUT("bind %hd failed %d! %s\n", sin->port, errno, strerror(errno));
	} else
#endif
	{
		struct sockaddr_in sa;

		memset(&sa, 0, sizeof(sa));
		sa.sin_family = AF_INET;
		sa.sin_addr = sin->in_addr;
		sa.sin_port = htons(sin->port);
		if (bind(sock, (struct sockaddr*)&sa, sizeof(sa)))
			ERR_OUT("bind %hd failed %d! %s\n", sin->port, errno, strerror(errno));
	}

	return 0;
Err:
	return -1;
}

int ind_net_aton(char *cp, struct sockaddr_in* sin)
{
	unsigned int addr, value;
	int part, len;

	if (cp == NULL)
		return 0;

	len = 0;
	addr = 0;
	for (part = 1; part <= 4; part++) {

		if (!isdigit(*cp))
			return 0;

		value = 0;
		while (isdigit(*cp)) {
			value *= 10;
			value += *cp++ - '0';
			if (value > 255)
				return 0;
			len ++;
		}

		if (part < 4) {
			if (*cp++ != '.')
				return 0;
			len ++;
		}

		addr <<= 8;
		addr |= value;
	}

	/*  W. Richard Stevens in his book UNIX Network Programming,
	 *  Volume 1, second edition, on page 71 says:
	 *
	 *  An undocumented feature of inet_aton is that if addrptr is
	 *  a null pointer, the function still performs it validation
	 *  of the input string, but does not store the result.
	 */
	if (sin)
		sin->sin_addr.s_addr = htonl(addr);

	if (*cp++ == ':') {
		unsigned short svalue;

		len ++;
		svalue = 0;
		while (isdigit(*cp)) {
			svalue *= 10;
			svalue += *cp++ - '0';
			len ++;
		}
		if (sin)
			sin->sin_port = htons(svalue);
	} else {
		if (sin)
			sin->sin_port = 0;
	}

	return len;
}

int ind_net_ntoa(struct sockaddr_in* sin, char *buf)
{
	unsigned char *p;

	if (buf == NULL)
		return 0;
	p = (unsigned char *)(&sin->sin_addr.s_addr);
	return sprintf(buf, "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
}

int ind_net_stoa(unsigned s_addr, char *buf)
{
	unsigned char *p;

	if (buf == NULL)
		return 0;
	p = (unsigned char *)(&s_addr);
	return sprintf(buf, "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
}

int ind_net_ntoa_ex(struct sockaddr_in* sin, char *buf)
{
	unsigned char *p;

	if (buf == NULL)
		return 0;
	p = (unsigned char *)(&sin->sin_addr.s_addr);
	return sprintf(buf, "%d.%d.%d.%d:%d", p[0], p[1], p[2], p[3], ntohs(sin->sin_port));
}

void ind_net_noblock(int fd)
{
#if defined(_WIN32)
#else
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
#endif
}

void ind_net_setRouteCall(IndRouteCall *func)
{
    gRouteCall = func;
}

int ind_net_sendto(int s, const void *msg, int len, unsigned int flags, struct sockaddr_in *to)
{
    if (gRouteCall)
        gRouteCall(to->sin_addr.s_addr);
    return sendto(s, msg, len, flags, (struct sockaddr *)to, sizeof(struct sockaddr));
}
