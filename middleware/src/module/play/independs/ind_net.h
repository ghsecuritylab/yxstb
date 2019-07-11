
#ifndef __IND_NET_H__
#define __IND_NET_H__

#include <stdint.h>

#include <netinet/in.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IND_HOST_LEN	64

#ifdef ENABLE_IPV6
#define IND_ADDR_LEN	48
struct ind_sin {
	uint16_t	family;
	uint16_t	port;
	struct in_addr in_addr;
	struct in6_addr	in6_addr;
};
#else
#define IND_ADDR_LEN	16
struct ind_sin {
	uint16_t	family;
	uint16_t  port;
	struct in_addr in_addr;
};
#endif

//0x00000001  更新地址消息
//0x00000010  更新地址响应消息
//0x00000100  KeepAlive消息
#define NAT_MSG_RENEW 0x01
#define NAT_MSG_RESPONSE 0x02
#define NAT_MSG_KEEPALIVE 0x04

struct natpkt {
	unsigned char version;
	unsigned char type;
	unsigned short reserved;
	unsigned int ssrc;
};

struct natpkt_zte {
    char    stbtype[8];
    unsigned int session;
    unsigned int data_addr;
    short   data_port;
    short   cmd_port;
    char    padding[64];
};

//len / 4 - 1
struct rtcppkt_app {
    char version;
    char subtype;
    short len;
    unsigned int ssrc;
    char name[4];
    unsigned int ssrc_app;
    unsigned short delay;//playout delay;
    unsigned short nsn;//nsn rtp num;
    short reserved;
    unsigned short fbs;//free buffer space;
};

struct rtcppkt_rr {
    char version;
    char subtype;
    short len;
    unsigned int ssrc;
};

struct rtcppkt_sd {
    char version;
    char subtype;
    short len;
    unsigned int ssrc;
    char type;
    char sd_len;
    char text[28];
    short end;
};

struct rtcppkt_fb {
    char version;
    char subtype;
    short len;
    unsigned int ssrc;//sender ssrc;
    unsigned int ssrc_rtp;//rtp ssrc;
    unsigned short rtcp_fb;//transport feedback nack;
    unsigned short rtcp_blp;//transport feedback nack blp, additional frame lost ;
};

struct rtcppkt_lost {
    struct rtcppkt_rr pktrr;
    struct rtcppkt_sd pktsd;
    struct rtcppkt_fb pktfb;
};

typedef struct ind_sin* ind_sin_t;

typedef void (IndRouteCall)(unsigned int addr);

unsigned int ind_net_index(ind_sin_t sin);
#define ind_net_equal ind_net_equal_v1
int ind_net_equal(ind_sin_t sin1, struct ind_sin* sin2);

int ind_net_ntop(ind_sin_t sin, char *buf, int len);
int ind_net_ntop_ex(ind_sin_t sin, char *buf, int len);//附加端口
int ind_net_pton(char* url, struct ind_sin* sin);
int ind_net_connect(int sock, struct ind_sin *sin);
int ind_net_bind(int sock, struct ind_sin *sin);
int ind_net_aton(char *cp, struct sockaddr_in* sin);

/*ind_net_ntoa和ind_net_ntoa_ex都是把struct sockaddr_in 转化为字符串形式，
	ind_net_ntoa_ex 还会在ip地址后面添加端口，如"239.255.0.1:5001"
	返回值都是生成字符串长度
 */
int ind_net_ntoa(struct sockaddr_in* sin, char *cp);
int ind_net_ntoa_ex(struct sockaddr_in* sin, char *cp);//附加端口

int ind_net_stoa(unsigned s_addr, char *buf);

void ind_net_noblock(int fd);

void ind_net_setRouteCall(IndRouteCall func);

int ind_net_sendto(int, const void *, int, unsigned int, struct sockaddr_in *);

#ifdef __cplusplus
}
#endif

#endif//__IND_NET_H__

