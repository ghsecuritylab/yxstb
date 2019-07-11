/*******************************************************************************
	¹«Ë¾£º
			Yuxing software
	¼ÍÂ¼£º
			2008-1-26 21:12:26 create by Liu Jianhua
	Ä£¿é£º
			tr069
	¼òÊö£º
			ÍøÂçÕï¶Ï
 *******************************************************************************/

#include "tr069_header.h"

/*
 * Structure of an internet header, naked of options.
 */
struct iphdr {
	u_char	ip_v_hl;
	u_char	ip_tos;			/* type of service */
	u_short	ip_len;			/* total length */
	u_short	ip_id;			/* identification */
	u_short	ip_off;			/* fragment offset field */
	u_char	ip_ttl;			/* time to live */
	u_char	ip_p;			/* protocol */
	u_short	ip_sum;			/* checksum */
	struct	in_addr ip_src, ip_dst;	/* source and dest address */
};

struct icmphdr
{
	u_char icmp_type;
	u_char icmp_code; // type sub code
	u_short icmp_cksum;
	union {
		struct ih_idseq {
			u_short	icd_id;
			u_short	icd_seq;
		} ih_idseq;
	} icmp_hun;
#define	icmp_id		icmp_hun.ih_idseq.icd_id
#define	icmp_seq	icmp_hun.ih_idseq.icd_seq
	union {
		struct id_ip  {
			struct iphdr idi_ip;
			/* options and then 64 bits of data */
		} id_ip;
	} icmp_dun;
#define	icmp_ip		icmp_dun.id_ip.idi_ip
};

struct pinghdr {
	struct iphdr ip;
	struct icmphdr udp;
};

#define ICMP_ECHOREPLY		0
#define	ICMP_UNREACH		3		/* dest unreachable, codes: */
#define		ICMP_UNREACH_NET	0		/* bad net */
#define		ICMP_UNREACH_HOST	1		/* bad host */
#define		ICMP_UNREACH_PROTOCOL	2		/* bad protocol */
#define		ICMP_UNREACH_PORT	3		/* bad port */
#define		ICMP_UNREACH_NEEDFRAG	4		/* IP_DF caused drop */
#define		ICMP_UNREACH_SRCFAIL	5		/* src route failed */
#define ICMP_ECHO			8
#define	ICMP_TIMXCEED		11		/* time exceeded, code: */
#define		ICMP_TIMXCEED_INTRANS	0		/* ttl==0 in transit */
#define		ICMP_TIMXCEED_REASS	1		/* ttl==0 in reass */

#define	ICMP_MINLEN			8
#define	MAX_PACKET_LEN		65536

static char hostname[16];
static u_int ident;			/* process id to identify our packets */
static u_short seq;

static SOCKET s;
static u_int timeout;
static u_int g_clock;

static struct sockaddr_in whereto;	/* Who to try to reach */
static int datalen;			/* How much data */

static u_char inpacket[MAX_PACKET_LEN];
static u_char outpacket[MAX_PACKET_LEN];


/*
 * in_cksum --
 *      Checksum routine for Internet Protocol family headers (C Version)
 */
static u_short in_cksum(u_short *addr, int len)
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
        while (nleft > 1)  {
                sum += *w++;
                nleft -= 2;
        }

        /* mop up an odd byte, if necessary */
        if (nleft == 1) {
                *(u_char *)(&answer) = *(u_char *)w ;
                sum += answer;
        }

        /* add back carry outs from top 16 bits to low 16 bits */
        sum = (sum >> 16) + (sum & 0xffff);     /* add hi 16 to low 16 */
        sum += (sum >> 16);                     /* add carry */
        answer = (u_short)(~sum);                /* truncate to 16 bits */
        return(answer);
}

static void pinger(void)
{
	struct icmphdr *icp;
	int ret;

	seq ++;

	icp = (struct icmphdr *)outpacket;
	icp->icmp_type = ICMP_ECHO;
	icp->icmp_code = 0;
	icp->icmp_cksum = 0;
	icp->icmp_seq = seq;
	icp->icmp_id = (u_short)ident;			/* ID */

	/* compute ICMP checksum here */
	icp->icmp_cksum = in_cksum((u_short *)icp, datalen);

	ret = tr069_sendto(s, (char *)outpacket, datalen, MSG_NOSIGNAL, &whereto);
	if (ret != datalen)
		TR069Printf("ping: wrote %s %d chars, ret=%d, errno = %d! %s\n", hostname, datalen, ret, errno, strerror(errno));
}

static int pr_pack(unsigned char *buf, int cc, int len)
{
	u_char type, code;
	struct icmphdr *icp;
	struct iphdr *ip;
	int hlen;

	/* Check the IP header */
	ip = (struct iphdr *)buf;
	hlen = (ip->ip_v_hl & 0x0f) << 2;
	if (cc < hlen + ICMP_MINLEN)
		TR069ErrorOut("packet too short (%d bytes) from %s\n", cc,
			  inet_ntoa(ip->ip_src));

	/* Now the ICMP part */
	cc -= hlen;
	buf += hlen;
	icp = (struct icmphdr *)buf;

	type = icp->icmp_type;
	code = icp->icmp_code;

	if (type == ICMP_ECHOREPLY) {
		if (icp->icmp_id != ident || icp->icmp_seq != seq) {
			printf("icmp_id %d/%d icmp_seq = %d/%d\n", icp->icmp_id, ident, icp->icmp_seq, seq);
			return 0;
	}

		cc -= ICMP_MINLEN;
		buf += ICMP_MINLEN;
		printf("%d bytes from %s: icmp_seq=%u ttl=%d", cc, inet_ntoa(ip->ip_src), icp->icmp_seq, ip->ip_ttl);
		if (cc < len)
			TR069ErrorOut("cc %d len = %d\n", cc, len);

		return ICMP_ECHOREPLY;
	} else if (type == ICMP_TIMXCEED && code == ICMP_TIMXCEED_INTRANS) {
		printf("%d bytes from %s: icmp type %d code %d", cc, inet_ntoa(ip->ip_src), type, icp->icmp_code);
		return ICMP_TIMXCEED;
	} else {
		printf("type = %d, code = %d", type, code);
	}

Err:
	return -1;
}

static int diag_init(char *host, int hl)
{
	struct hostent *hp;
	struct sockaddr_in *to;

	seq = 0;

	if (datalen < 0)
		datalen = 1;
	else  if (datalen > MAX_PACKET_LEN - hl)
		datalen = MAX_PACKET_LEN - hl;

	memset(&whereto, 0, sizeof(struct sockaddr_in));
	to = (struct sockaddr_in *)(&whereto);
	to->sin_family = AF_INET;
	to->sin_addr.s_addr = inet_addr(host);
	if (strlen(host) <= 15 && to->sin_addr.s_addr != INADDR_NONE) {
		strcpy(hostname, host);
		TR069Printf("host addr = %s\n", hostname);
	} else {
		hp = gethostbyname(host);
		if (!hp) {
			TR069Printf("unknown host %s\n", host);
			return DIAG_ERROR_RESOLVE;
		}
		to->sin_family = hp->h_addrtype;
		memcpy(&to->sin_addr, hp->h_addr, (u_int)hp->h_length);
		strcpy(hostname, inet_ntoa(to->sin_addr));
		TR069Printf("host name = %s, addr = %s\n", host, hostname);
	}

	ident = tr069_sec( ) & 0xFFFF;

	s = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (s == INVALID_SOCKET)
		TR069ErrorOut("socket");
/*
	{
		int ret = tr069_connect(s, to);
		TR069Printf("connect ret = %d\n", ret);
	}
*/
	return 0;
Err:
	return DIAG_ERROR_OTHER;
}

static int wait_for_reply(SOCKET sock, struct sockaddr_in *from)
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

int tr069_diag_ping(struct Ping *ping)
{
	int ret, len, hold;
	u_int msec, i;
	struct sockaddr_in from;
//#ifdef DEBUG_BUILD
	struct sockaddr_in * to = NULL;
	to = (struct sockaddr_in *)&whereto;
//#endif

	timeout = ping->timeout;
	datalen = (int)ping->datalen;

	TR069Printf("Ping: datalen = %d, count = %d, timeout = %d\n", datalen, ping->count, timeout);

	ping->successcount = 0;
	ping->failurecount = 0;
	ping->averagetime = 0;
	ping->minimumtime = 0;
	ping->maximumtime = 0;

	s = INVALID_SOCKET;
	ret = diag_init(ping->host, sizeof(struct pinghdr));
	if (ret) {
		TR069Printf("ERROR! diag_init = %d\n", ret);
		return ret;
	}
	datalen += ICMP_MINLEN;

	/*
	 * When pinging the broadcast address, you can get a lot of answers.
	 * Doing something so evil is useful if you are trying to stress the
	 * ethernet, or just want to fill the arp cache to get some stuff for
	 * /etc/ethers.
	 */
	hold = 48 * 1024;
	(void)setsockopt(s, SOL_SOCKET, SO_RCVBUF, (char *)&hold, sizeof(hold));

	if (ping->dscp != 0) {
#ifndef WIN32
		int tos = (ping->dscp & 0x3f) << 2;
		if (setsockopt(s, IPPROTO_IP, IP_TOS, (char *)&tos, sizeof(int)) < 0)
	 	    TR069ErrorOut("setsockopt (IP_TOS)\n");
#endif
	 }
	TR069Printf("PING %s (%s): %d data bytes\n", hostname,
	    inet_ntoa(*(struct in_addr *)&to->sin_addr.s_addr), ping->datalen);

	timeout = ping->timeout;
	for (i = 0; i < ping->count; i ++) {
		g_clock = tr069_msec( );

		pinger( );

WAIT:
		len = wait_for_reply(s, &from);
		if (len <= 0) {
			TR069Printf("len = %d\n", len);
			continue;
		}

		ret = pr_pack(inpacket, len, (int)ping->datalen);
		if (ret != ICMP_ECHOREPLY)
			goto WAIT;

		msec = tr069_msec( ) - g_clock;
		if (msec == 0)
			msec = 1;
		printf(" i = %d, msec = %d\n", i, msec);
		ping->averagetime += msec;
		if (ping->successcount == 0) {
			ping->minimumtime = msec;
			ping->maximumtime = msec;
		} else {
			if (ping->minimumtime > msec)
				ping->minimumtime = msec;
			if (ping->maximumtime < msec)
				ping->maximumtime = msec;
		}
		ping->successcount ++;
	}
#ifndef WIN32
Err:
#endif

	ping->failurecount = ping->count - ping->successcount;
	if (ping->successcount > 0)
		ping->averagetime /= ping->successcount;

	if (s != INVALID_SOCKET)
		closesocket(s);

	TR069Printf("success = %d, failure = %d, average = %d, minimum = %d, maximum = %d\n", 
				ping->successcount, ping->failurecount, 
				ping->averagetime, ping->minimumtime, ping->maximumtime);

	TR069Printf("count = %d, successcount = %d, failurecount = %d\n", ping->count, ping->successcount, ping->failurecount);
	if (ping->successcount > 0)
		return DIAG_ERROR_NONE;

	TR069Printf("Ping ERROR!\n");
	return DIAG_ERROR_OTHER;
}

static int send_probe(int ttl)
{
#ifndef WIN32
	if (setsockopt(s, IPPROTO_IP, IP_TTL, (char *)&ttl, sizeof(int)) < 0)
	     TR069ErrorOut("setsockopt (IP_TTL)\n");
#endif
	pinger( );
	return 0;
#ifndef WIN32
Err:
	return -1;
#endif
}

int tr069_diag_trace(struct Trace *trace)
{
	int len, ret, err = DIAG_ERROR_INTERNAL;
	int i, ttl, max_ttl;
	u_int clk;
	struct sockaddr_in from, *to = (struct sockaddr_in *)&whereto;

	max_ttl = (int)trace->hopmax;
	timeout = trace->timeout;
	datalen = (int)trace->datalen;

	TR069Printf("Trace: hopmax = %d, timeout = %d\n", max_ttl, trace->timeout);

	trace->hopnum = 0;
	trace->resptime = 0;
	clk = tr069_msec( );

	s = INVALID_SOCKET;
	ret = diag_init(trace->host, sizeof(struct pinghdr));
	if (ret) {
		TR069Printf("ERROR! diag_init = %d\n", ret);
		return ret;
	}
	datalen += ICMP_MINLEN;

	if (trace->dscp != 0) {
#ifndef WIN32
		int tos = (trace->dscp & 0x3f) << 2;
		if (setsockopt(s, IPPROTO_IP, IP_TOS, (char *)&tos, sizeof(int)) < 0)
	 	    TR069ErrorOut("setsockopt (IP_TOS)\n");
#endif
	}

	printf("traceroute to %s (%s)\n", hostname, inet_ntoa(to->sin_addr));

	for (ttl = 1; ttl <= max_ttl; ++ttl) {
		int type;
		uint clk, minimum, maximum, total;

		printf("%2d ", ttl);

		g_clock = tr069_msec( );
		seq ++;
		type = 0;

		minimum = 0;
		maximum = 0;
		total = 0;
		trace->routehop[trace->hopnum].hophost[0] = 0;
		for (i = 0; i < 3; i ++) {
			clk = tr069_msec( );
			send_probe(ttl);
			len = wait_for_reply(s, &from);
			if (len < 0)
				TR069ErrorOut("len = %d\n", len);
			if (len == 0) {
				printf(" #");
			} else {
				printf(" *");
				type = pr_pack(inpacket, len, (int)trace->datalen);
				if (type == ICMP_ECHOREPLY || type == ICMP_TIMXCEED) {
					printf("  %d ms", tr069_msec( ) - g_clock);
					strcpy(trace->routehop[trace->hopnum].hophost, inet_ntoa(from.sin_addr));
					if (from.sin_addr.s_addr == to->sin_addr.s_addr) {
						err = DIAG_ERROR_NONE;
						type = ICMP_ECHOREPLY;
					}
					if (type == ICMP_ECHOREPLY)
						ttl = max_ttl + 1;
				}
			}
			clk = tr069_msec( ) - clk;
			if (i == 0) {
				minimum = clk;
				maximum = clk;
				total = clk;
			} else {
				if (minimum > clk)
					minimum = clk;
				if (maximum < clk)
					maximum = clk;
				total += clk;
			}
		}
		trace->routehop[trace->hopnum].minimumResponseTime = minimum;
		trace->routehop[trace->hopnum].averageResponseTime = total / 3;
		trace->routehop[trace->hopnum].maximumResponseTime = maximum;
		trace->hopnum ++;

		printf("\n");
		if (err == DIAG_ERROR_NONE)
			break;
	}
Err:
	TR069Printf("hopnum = %d\n", trace->hopnum);

	if (s != INVALID_SOCKET)
		closesocket(s);

	if (err == DIAG_ERROR_NONE) {
		trace->resptime = tr069_msec( ) - clk;
		return DIAG_ERROR_NONE;
	}

	trace->hopnum = 0;

	return DIAG_ERROR_INTERNAL;
}
