#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include "MonitorTraceroute.h"
#include "MonitorDef.h"
#include "MonitorManager.h"


static char hostname[16];
static u_int ident;         /* process id to identify our packets */
static u_short seq;
static int s;
static u_int timeout;
static u_int g_clock;
static struct sockaddr whereto; /* Who to try to reach */
static int datalen;         /* How much data */
static u_char inpacket[MAX_PACKET_LEN];
static u_char outpacket[MAX_PACKET_LEN];
static TRACEROUTE_STATUS traceroute_flag = TRACEROUTE_STOP;



void monitorTracerouteFunc(moni_buf_t buf, char *string)
{
    sprintf(buf->buf, "200connect^");
    buf->extend = (char *)malloc(strlen(string) + strlen("traceroute^") + 1);
    memset(buf->extend, 0, strlen(string) + strlen("traceroute^") + 1);
    sprintf(buf->extend, "traceroute^%s", string);
	printf("%s\n", buf->extend);
    MonitorManager::GetInstance()->monitor_cmd_response(buf);
    return;
}

int monitorTracerouteConnect(moni_buf_t buf, int len)
{
    char *p = NULL, *p1 = NULL;
    DIAG_MSG msg;

    printf("buf is %s\n", buf->buf);
    memset(&msg, 0, sizeof(msg));
    p = strstr(buf->buf, "null");
    if(p) {
        msg.cmd = TRACEROUTE_START;
        msg.length = -1;

        p += strlen("null");
        p1 = strchr(p, '^');

        if(!p1) {
            printf("Error: Not url, buf is %s\n", p);
            return -1;
        }

        p1 += 1;
        printf("url is %s\n", p1);

        strcpy(msg.url, p1);
        printf("msg.url is %s\n", msg.url);

        memcpy(&(msg.buf), buf, sizeof(struct moni_buf));
        printf("msg buf is %s\n", msg.buf.buf);

        msg.func = (void *)monitorTracerouteFunc;

        if(MonitorManager::GetInstance()->monitorDiagMsgPut(&msg) == -1) {
            printf("Error: monitor_diagMsg_put return -1\n");
            return -1;
        }

        return 0;
    }

    p = strstr(buf->buf, "/stop");
    if(p) {
        printf("receive stop request, stop traceroute!\n");
        monitorSetTraceRouteStop();
        return 0;
    }

    printf("Error: string illegal\n");
    return -1;
}



TRACEROUTE_STATUS monitorGetTraceRouteFlag( void )
{
	return traceroute_flag;
}

void monitorSetTraceRouteRun( void )
{
	traceroute_flag = TRACEROUTE_RUN;
	printf( "RUN traceroute, traceroute_flag is %d\n", traceroute_flag );
}

void monitorSetTraceRouteStop( void )
{
	traceroute_flag = TRACEROUTE_STOP;
	printf( "STOP traceroute, traceroute_flag is %d\n", traceroute_flag );
}


/*
 * in_cksum --
 *      Checksum routine for Internet Protocol family headers (C Version)
 */
u_short monitorInCksum(u_short *addr, int len)
{
	int nleft = len;
	u_short *w = addr;
	u_int sum = 0;
	u_short answer = 0;

	/*
	 * Our algorithm is simple, using a 32 bit accumulator (sum), we add
	 * sequential 16 bit words to it, and at the end, fold back all the
	 * carry bits from the top 16 bits into the lower 16 bits.
	 */
	while (nleft > 1)
	{
		sum += *w++;
		nleft -= 2;
	}

	/* mop up an odd byte, if necessary */
	if (nleft == 1)
	{
		*(u_char *)(&answer) = *(u_char *)w ;
		sum += answer;
	}

	/* add back carry outs from top 16 bits to low 16 bits */
	sum = (sum >> 16) + (sum & 0xffff);     /* add hi 16 to low 16 */
	sum += (sum >> 16);                     /* add carry */
	answer = (u_short)(~sum);                /* truncate to 16 bits */
	return(answer);
}

void monitorPinger(void)
{
	struct icmphdr *icp;
	int ret;

	seq ++;

	icp = (struct icmphdr *)outpacket;
	icp->type = ICMP_ECHO;
	icp->code = 0;
	icp->checksum = 0;
	icp->un.echo.sequence = seq;
	icp->un.echo.id = (u_short)ident;          /* ID */

	/* compute ICMP checksum here */
	icp->checksum = monitorInCksum((u_short *)icp, datalen);

	ret = sendto(s, (char *)outpacket, datalen, MSG_NOSIGNAL, &whereto, sizeof(struct sockaddr));
	if (ret != datalen)
		printf("ping: wrote %s %d chars, ret=%d, errno = %d\n", hostname, datalen, ret, errno);
}

int monitorPrPack(unsigned char *buf, int cc, int len, char *printf_string)
{
	u_char type, code;
	struct icmphdr *icp;
	struct monitorIphdr *ip;
	int hlen;
	char temp[256]= {0};

	/* Check the IP header */
	ip = (struct monitorIphdr *)buf;
	hlen = (ip->ip_v_hl & 0x0f) << 2;
	if (cc < hlen + ICMP_MINLEN)
		printf("packet too short (%d bytes) from %s\n", cc, inet_ntoa(ip->ip_src));

	/* Now the ICMP part */
	cc -= hlen;
	buf += hlen;
	icp = (struct icmphdr *)buf;

	type = icp->type;
	code = icp->code;

	if (type == ICMP_ECHOREPLY)
	{
		if (icp->un.echo.id != ident || icp->un.echo.sequence != seq)
		{
			// printf("icmp_id %d/%d icmp_seq = %d/%d\n", icp->icmp_id, ident, icp->icmp_seq, seq);
			snprintf( temp, sizeof(temp), "icmp_id %d/%d icmp_seq = %d/%d", icp->un.echo.id, ident, icp->un.echo.sequence, seq );
			strcat( printf_string, temp );
			return 0;
		}

		cc -= ICMP_MINLEN;
		buf += ICMP_MINLEN;
		// printf("%d bytes from %s: icmp_seq=%u ttl=%d", cc, inet_ntoa(ip->ip_src), icp->icmp_seq, ip->ip_ttl);
		snprintf( temp, sizeof(temp), "%d bytes from %s: icmp_seq=%u ttl=%d", cc, inet_ntoa(ip->ip_src), icp->un.echo.sequence, ip->ip_ttl );
		strcat( printf_string, temp );
		if (cc < len)
			printf("cc %d len = %d\n", cc, len);

		return ICMP_ECHOREPLY;
	}
	else if (type == ICMP_TIMXCEED && code == ICMP_TIMXCEED_INTRANS)
	{
		// printf("%d bytes from %s: icmp type %d code %d", cc, inet_ntoa(ip->ip_src), type, icp->icmp_code);
		snprintf( temp, sizeof(temp), "%d bytes from %s: icmp type %d code %d", cc, inet_ntoa(ip->ip_src), type, icp->code );
		strcat( printf_string, temp );
		return ICMP_TIMXCEED;
	}
	else
	{
		// printf("type = %d, code = %d", type, code);
		snprintf( temp, sizeof(temp), "type = %d, code = %d", type, code );
		strcat( printf_string, temp );
	}

	return -1;
}

int monitorDiagInit(char *host, int hl)
{
	struct hostent *hp;
	struct sockaddr_in *to;

	seq = 0;

	if (datalen < 0)
		datalen = 1;
	else  if (datalen > MAX_PACKET_LEN - hl)
		datalen = MAX_PACKET_LEN - hl;

	memset(&whereto, 0, sizeof(struct sockaddr));
	to = (struct sockaddr_in *)&whereto;
	to->sin_family = AF_INET;
	to->sin_addr.s_addr = inet_addr(host);
	if (strlen(host) <= 15 && to->sin_addr.s_addr != INADDR_NONE)
	{
		strcpy(hostname, host);
		printf("host addr = %s\n", hostname);
	}
	else
	{
		hp = gethostbyname(host);
		if (!hp)
		{
			printf("unknown host %s\n", host);
			return DIAG_ERROR_RESOLVE;
		}
		to->sin_family = hp->h_addrtype;
		memcpy(&to->sin_addr, hp->h_addr, (u_int)hp->h_length);
		strcpy(hostname, (char *)inet_ntoa(to->sin_addr));
		printf("host name = %s, addr = %s\n", host, hostname);
	}

    long long msec;
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    msec = tp.tv_sec;
    msec = msec * 1000 + tp.tv_nsec / 1000000;
	ident = (u_int)(msec / 1000) & 0xFFFF;


	s = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (s == INVALID_SOCKET)
		printf("socket");
	/*
	   {
	   int ret = connect(s, (struct sockaddr *)to, sizeof(struct sockaddr_in));
	   printf("connect ret = %d\n", ret);
	   }
	   */
	return 0;
//Err:
//	return DIAG_ERROR_OTHER;
}

int monitorWaitForReply(SOCKET sock, struct sockaddr_in *from)
{
	fd_set fds;
	struct timeval wait;
	int nfds;
	int cc = 0;
	socklen_t fromlen;

	FD_ZERO(&fds);
	FD_SET((u_int)sock, &fds);
	wait.tv_sec = (long)(timeout / 1000);
	wait.tv_usec = (timeout % 1000) * 1000;

	nfds = (int)(sock + 1);
	if (select(nfds, &fds, (fd_set *)0, (fd_set *)0, &wait) <= 0)
		return 0;
	fromlen = sizeof(struct sockaddr_in);
	cc=recvfrom(s, (char *)inpacket, MAX_PACKET_LEN, 0,
		(struct sockaddr *)from, &fromlen);

	return cc;
}

int monitorSendProbe(int ttl)
{
#ifndef WIN32
	if (setsockopt(s, IPPROTO_IP, IP_TTL, (char *)&ttl, sizeof(int)) < 0)
		printf("setsockopt (IP_TTL)\n");
#endif
	monitorPinger();
	return 0;
}

int monitorTraceroute( char *url, moni_buf_t buf, void* stb_func )
{
	int len, ret, type = 0;
	int i, ttl, max_ttl, dlen;
	//u_int clk;
	struct sockaddr_in from, *to = (struct sockaddr_in *)&whereto;

	max_ttl = 64;
	timeout = 2000;
	dlen = datalen = 28;

	char pstring[512] = {0};
	char temp[512] = {0};

	PTR_CALLBACK func = (PTR_CALLBACK)stb_func;

	printf("Trace: hopmax = %d, timeout = %d\n", max_ttl, timeout);

	//clk = a_Ticker_get_10millis();

	s = INVALID_SOCKET;
	ret = monitorDiagInit(url, sizeof(struct pinghdr));
	if (ret)
	{
		printf("ERROR! diag_init = %d\n", ret);
		return ret;
	}
	datalen += ICMP_MINLEN;

	//  printf("traceroute to %s (%s)\n", hostname, inet_ntoa(to->sin_addr));
	snprintf( pstring, sizeof(pstring), "traceroute to %s (%s)\n", hostname, inet_ntoa(to->sin_addr) );
	func(buf,pstring);

	for (ttl = 1; ttl <= max_ttl; ++ttl)
	{
		if( monitorGetTraceRouteFlag() == TRACEROUTE_STOP )
			break;
		//      printf("%2d ", ttl);
		snprintf( pstring, sizeof(pstring), "the %2d, ", ttl );

        unsigned int clk;
        struct timespec tp;
        clock_gettime(CLOCK_MONOTONIC, &tp);
        clk = (unsigned int)tp.tv_sec * 100 + (unsigned int)tp.tv_nsec / 10000000;
		g_clock = clk;

		seq ++;
		monitorSendProbe(ttl);
		type = 0;
		for (i = 0; i < 3; i ++)
		{
			len = monitorWaitForReply(s, &from);
			if (len < 0)
				printf("len = %d\n", len);
			if (len == 0)
			{
				strcat( pstring, " #" );

                //超过三次失败就结束
                if (i == 2) {
                    traceroute_flag = TRACEROUTE_STOP;
                    memset(pstring, 0, 512);
                    sprintf(pstring, "请求超时\n");
                    func(buf,pstring);
                    break;
                }
				continue;
			}
			//          strcat( pstring, " *" );
			type = monitorPrPack(inpacket, len, dlen, pstring );
			if (type == ICMP_ECHOREPLY || type == ICMP_TIMXCEED)
			{
				//              printf(("  %d ms", (mid_10ms( ) - g_clock) * 10);

                unsigned int clk;
                struct timespec tp;
                clock_gettime(CLOCK_MONOTONIC, &tp);
                clk = (unsigned int)tp.tv_sec * 100 + (unsigned int)tp.tv_nsec / 10000000;
				snprintf( temp, sizeof(temp), "  %d ms", (clk - g_clock) * 10 );
				strcat( pstring, temp );
				if (type == ICMP_ECHOREPLY)
					ttl = max_ttl + 1;
				break;
			}
		}

		if( monitorGetTraceRouteFlag() == TRACEROUTE_STOP )
			break;
		strcat( pstring, "\n" );
		func(buf,pstring);
        sleep(1);
	}

	if (s != INVALID_SOCKET)
		closesocket(s);
	return 0;
}

