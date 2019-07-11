#ifndef MonitorTraceroute_h
#define MonitorTraceroute_h

#include <netinet/in.h>
#include <linux/icmp.h>


struct monitorIphdr {
	u_char  ip_v_hl;
	u_char  ip_tos; // type of service
	u_short ip_len; // total length
	u_short ip_id; // identification
	u_short ip_off; // fragment offset field
	u_char  ip_ttl; // time to live
	u_char  ip_p; // protocol
	u_short ip_sum; // checksum
	struct in_addr ip_src; // source address
	struct in_addr ip_dst; // source address
};
/*
struct icmphdr {
    u_char icmp_type;
    u_char icmp_code; // type sub code
    u_short icmp_cksum;
	union {
		struct ih_idseq {
			u_short icd_id;
			u_short icd_seq;
		} ih_idseq;
	} icmp_hun;
#define icmp_id icmp_hun.ih_idseq.icd_id
#define icmp_seq icmp_hun.ih_idseq.icd_seq
	union {
		struct id_ip {
			struct monitorIphdr idi_ip; // options and then 64 bits of data
		} id_ip;
	} icmp_dun;
#define icmp_ip icmp_dun.id_ip.idi_ip
};
struct icmphdr {
  __u8		type;
  __u8		code;
  __sum16	checksum;
  union {
	struct {
		__be16	id;
		__be16	sequence;
	} echo;
	__be32	gateway;
	struct {
		__be16	__unused;
		__be16	mtu;
	} frag;
  } un;
};
*/
struct pinghdr {
	struct monitorIphdr ip;
	struct icmphdr udp;
};

typedef enum {
    TRACEROUTE_RUN = 0,
    TRACEROUTE_STOP
} TRACEROUTE_STATUS;

typedef struct moni_buf* moni_buf_t;

#define ICMP_ECHOREPLY          0
#define ICMP_UNREACH            3 // dest unreachable, codes:
#define ICMP_UNREACH_NET        0 // bad net
#define ICMP_UNREACH_HOST       1 // bad host
#define ICMP_UNREACH_PROTOCOL   2 // bad protocol
#define ICMP_UNREACH_PORT       3 // bad port
#define ICMP_UNREACH_NEEDFRAG   4 // IP_DF caused drop
#define ICMP_UNREACH_SRCFAIL    5 // src route failed
#define ICMP_ECHO               8
#define ICMP_TIMXCEED           11 // time exceeded, code:
#define ICMP_TIMXCEED_INTRANS   0 // ttl==0 in transit
#define ICMP_TIMXCEED_REASS     1 // ttl==0 in reass
#define ICMP_MINLEN             8
#define MAX_PACKET_LEN          65536

#define INVALID_SOCKET  -1
#define SOCKET          int
#define closesocket     close


int monitorTracerouteConnect(moni_buf_t buf, int len);
void monitorTracerouteFunc(moni_buf_t buf, char* string);
TRACEROUTE_STATUS monitorGetTraceRouteFlag(void);
void monitorSetTraceRouteRun(void);
void monitorSetTraceRouteStop(void);
u_short monitorInCksum(u_short* addr, int len);
void monitorPinger(void);
int monitorPrPack(unsigned char* buf, int cc, int len, char* printf_string);
int monitorDiagInit(char* host, int hl);
int monitorWaitForReply(SOCKET sock, struct sockaddr_in* from);
int monitorSendProbe(int ttl);
int monitorTraceroute(char* url, moni_buf_t buf, void *stb_func);

#endif //MonitorTraceroute_h

