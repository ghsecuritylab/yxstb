
#include <errno.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/igmp.h>

#include "config.h"
#include "app_include.h"
#include "porting/stream_port.h"
#include "sys_msg.h"
#include "sys_key_deal.h"
#include "sys_basic_macro.h"
#include "Tr069.h"
#include "Tr069LogMsg.h"
#include "ind_mem.h"
//#include "tstv.h"
#include "Assertions.h"
#include "SettingModuleNetwork.h"
#include "FrequenceSetting.h"

#ifdef TVMS_OPEN
#include "tvms.h"
//#include "tvms_define.h"
#endif

/* Eagle port. 2011年01月19日 */
#if defined(SQM_INCLUDED)
#include "sqm_port.h"
#endif

#ifdef ENABLE_IGMPV3
typedef struct __Igmpv3Info {
    int srcip1;
    int srcip2;
    int srcip3;
    int srcip4;
    int version;
}Igmpv3Info;
#endif

#define SOCKET_BUF_SIZE 1230 * 1024
/* a2事件使用的状态 */

/* 保存全局的上一次rtsp状态 */
static STRM_STATE g_play_mode = STRM_STATE_CLOSE;

/* 本地显示台标的标志 */
//static int iconrate;

/* 全局标志位 */
static int g_end_begin=0;
static int g_timeshift_flag;
static int g_playRate;
static int g_is_multicast_play = 0;
static u_int g_multicast_addr;
static u_short g_multicast_port;

#ifdef ENABLE_IPV6
struct igmpHeader {
    //mode type 0x03 changed to include, 0x04 changed to exclude, 0x05 allow new sources, 0x06 block old sources;
    char mode;
    char len;
    short reserve;
    struct in6_addr muladdr;
    struct in6_addr srcaddr;
};

struct icmpHeader {
    char type;
    char code;
    short checksum;
    int optnum;
    struct igmpHeader igmp[10];
};
#endif
struct Group
{
    unsigned char igmp_rtype;//Record Type, 0x3 Change to Include Mode, 0x4 Change to Exclude Mode;
    unsigned char igmp_adlen;//Aux Data Len
    unsigned short igmp_numsrc;
    struct in_addr igmp_group;
    unsigned int igmp_sources;
};

struct igmpv3_spc
{
    unsigned char igmp_type;
    unsigned char igmp_code;
    unsigned short igmp_cksum;
    unsigned short igmp_reserve;//unknow data now
    unsigned short igmp_ngr;//Num Group Records
    struct Group group[2];
};

struct igmpv3
{
    unsigned char igmp_type;
    unsigned char igmp_code;
    unsigned short igmp_cksum;
    unsigned short igmp_reserve;//unknow data now
    unsigned short igmp_ngr;//Num Group Records
    struct Group group;
};

typedef struct play_streamstate_
{
    int g_playState;
    int g_playRate;
    int g_play_mode;
    int iconrate;
}play_streamstate;

play_streamstate playstreamstate[3]={{0,0,STRM_STATE_CLOSE,0},{0,0,STRM_STATE_CLOSE,0}};

static stream_msg_handle g_stream_msg_hdl = NULL;
static stream_state_handle g_stream_state_hdl = NULL;

void stream_port_msg_hdl_set(stream_msg_handle msg_hdl)
{
    g_stream_msg_hdl = msg_hdl;
}

void stream_port_state_hdl_set(stream_state_handle state_hdl)
{
    g_stream_state_hdl = state_hdl;
}

void stream_port_set_rate(int rate, int pIndex)
{
    PRINTF("stream_port_set_rate %d\n",rate);
    playstreamstate[pIndex].iconrate = rate;
    //    iconrate = rate;
}

void stream_port_set_state(int state,int pIndex)
{
    PRINTF("stream_port_set_state %d\n",state);
    playstreamstate[pIndex].g_play_mode = state;
    //    iconrate = rate;
}

//背景音乐不产生播放消息
static void stream_port_message_bgmusic(int pIndex, STRM_MSG msg, int arg, unsigned int magic)
{
    PRINTF("@@@@: index = %d, msg = %d, arg = %d, magic = %u\n", pIndex, msg, arg, magic);
}

static void stream_port_state_bgmusic(int pIndex, STRM_STATE state, int rate, unsigned int magic)
{
}

void stream_port_mpahdl_set(void)
{
    g_stream_msg_hdl = (stream_msg_handle)stream_port_message_bgmusic;
    g_stream_state_hdl = (stream_state_handle)stream_port_state_bgmusic;
}

void stream_port_hdl_set(void)
{
    //g_stream_msg_hdl = (stream_msg_handle)stream_port_message_iptv;
    //g_stream_state_hdl = (stream_state_handle)stream_port_state_iptv;
}

int stream_port_get_rate(void)
{
    DBG_PRN("stream_port_get_rate %d\n",playstreamstate[0].iconrate);
    return playstreamstate[0].iconrate;
    //  return iconrate;
}

int stream_port_get_state(void)
{
    DBG_PRN("stream_port_get_state index=%d  mode=%d\n",0,playstreamstate[0].g_play_mode);
    return playstreamstate[0].g_play_mode;
    //  return g_play_mode;
}

int stream_port_get_parameter(int pIndex,int *mode,int *state,int *rate)
{
    *mode = playstreamstate[pIndex].g_play_mode;
    *state = playstreamstate[pIndex].g_playState;
    *rate = playstreamstate[pIndex].g_playRate;
    return 0;
}

/*******************************************************************************

  RTSP Porting

 *******************************************************************************/
//这个函数会被调用两次，创建两任务，个任务参数arg会有所不同
void stream_port_task_create(mid_func_t entry, void *arg)
{
    //  app_rtsp_task_create(entry, arg);
    char name[32];
    sprintf(name, "rtsp_%p", arg);
    mid_task_create_ex(name, entry, arg);
}

/*------------------------------------------------------------------------------
  申请内存
  ------------------------------------------------------------------------------*/
void* stream_port_malloc(int size)
{
    //  char *buf;
    //
    //  if (yhw_mem_alloc(size, 4, (void **)&buf))
    //      return NULL;
    //  return buf;
    return IND_MALLOC(size);
}

/*------------------------------------------------------------------------------
  释放内存
  ------------------------------------------------------------------------------*/
void stream_port_free(void *ptr)
{
    //yhw_mem_free(ptr);
    IND_FREE(ptr);
}

int stream_port_multicast_read(int pIndex, char *addr)
{
    //  return cus_cfg_multicast_get(addr);
    return sysMulticastGet(pIndex, addr);
}

int stream_port_multicast_write(int pIndex, char *addr)
{
    return sysMulticastSet(pIndex, addr);
}

int stream_port_srcip_read(int pIndex, char *addr)
{
#ifdef ENABLE_IGMPV3
    return sysSrcipGet(pIndex, addr);
#else
    return 0;
#endif
}

int stream_port_srcip_write(int pIndex, char *addr)
{
#ifdef ENABLE_IGMPV3
    return sysSrcipSet(pIndex, addr);
#else
    return 0;
#endif
}

/* 由于使用半播控，本地控制播放状态所以在该函数内处理基本状态转变 */
//void stream_port_state(int pIndex, STRM_STATE state, int rate)
void stream_port_state(int pIndex, STRM_STATE state, int rate, unsigned int magic)
{
    PRINTF("stream_port_state index=%d  state=%d,rate = %d,\n",pIndex,state,rate);

    /* Eagle port. 2011年01月19日 */
#ifdef SQM_VERSION_C21
    CHN_STAT chn_stat;

    if(state == STRM_STATE_FAST){
        chn_stat = FAST_BB_BF_TYPE;
        parseChnInfo(chn_stat, NULL, NULL, NULL);

        PRINTF("[SQM_PORT]: channel state = FAST\n");
        sqm_port_msg_write(MSG_SETINFO);
    }
#endif

    if(g_stream_state_hdl)
      g_stream_state_hdl(pIndex, state, rate, magic);

    return;
}

//void stream_port_message(int pIndex, STRM_MSG msg)
void stream_port_message(int pIndex, STRM_MSG msg, int arg, unsigned int magic)
{
    if(g_stream_msg_hdl)
      g_stream_msg_hdl(pIndex, msg, arg, magic);
    //  stream_port_message_iptv(pIndex, msg);

    return;
}


void mosaic_port_message(int key, STRM_MSG msg)
{
    switch(msg) {
        case MOSAIC_MSG_ERROR:
            PRINTF("MOSAIC_MSG_ERROR\n");
            break;
        case MOSAIC_MSG_ERROR_RECT:
            PRINTF("MOSAIC_MSG_ERROR_RECT\n");
            break;
        case MOSAIC_MSG_SUCCESS:
            PRINTF("MOSAIC_MSG_SUCCESS\n");
            break;
        case MOSAIC_MSG_TIMEOUT:
            PRINTF("MOSAIC_MSG_TIMEOUT\n");
            break;
        case MOSAIC_MSG_RESUME:
            PRINTF("MOSAIC_MSG_RESUME\n");
            break;

        default:
            PRINTF("DEFAULT\n");
            break;
    }
}

void stream_port_peername(int pIndex, int tcp, unsigned int addr, unsigned short port)
{
    PRINTF("@@@@@@@@: tcp = %d, addr = 0x%08x, port = %hd\n", tcp, addr, port);
}

int stream_port_in6addr(struct ind_sin *pSin)
{
#ifdef ENABLE_IPV6
    char ipv6[IND_ADDR_LEN];

    pSin->family = AF_INET6;
    pSin->port = 0;

    char ifname[URL_LEN] = { 0 };
    char ifaddr[URL_LEN] = { 0 };
    network_default_ifname(ifname, URL_LEN);
    return inet_pton(AF_INET6, network_address_get(ifname, ifaddr, URL_LEN), &pSin->in6_addr);
#else
    return -1;
#endif
}

#ifdef ENABLE_IGMPV3
#endif

#ifdef ENABLE_IPV6
int stream_port_mldv1_play(void* igmp, int igmp_size, struct ind_sin *mult_sin)
{//, unsigned int addr, unsigned short port
    PRINTF("stream_port_mldv1_play\n");

    int  sock;
    int  ret;
    char ifname[10];
    struct ind_sin *tSin = (struct ind_sin *)igmp;

    sock = socket(AF_INET6, SOCK_DGRAM, 0);
    if (sock == -1) {
        ERR_OUT("socket create err\n");
    }

    struct sockaddr_in6 saddr6;
    memset(&saddr6, 0, sizeof(saddr6));
    saddr6.sin6_family = AF_INET6;
    saddr6.sin6_addr = in6addr_any;
    saddr6.sin6_port = htons(tSin->port);
    ret = bind(sock, (struct sockaddr*)&saddr6, sizeof(saddr6));
    if (ret == -1) {
        close(sock);
        ERR_OUT("socket bind err\n");
    }

    /* set group_source_req parameters */
    struct group_req req;
    struct sockaddr_in6 *paddr;

    memset(&req, 0, sizeof(req));
    IND_MEMSET(ifname, 0, 10);

    sys_get_net_ifname(ifname, 10);
    req.gr_interface = if_nametoindex(ifname);

    paddr = (struct sockaddr_in6*)&req.gr_group;
    paddr->sin6_family = AF_INET6;
    //ret = inet_pton(AF_INET6, "ff02::16", &paddr->sin6_addr);
    memcpy(&paddr->sin6_addr, &tSin->in6_addr, sizeof(struct in6_addr));

    ret = setsockopt(sock, IPPROTO_IPV6, MCAST_JOIN_GROUP, (char *)&req, sizeof(req));
    if (ret == -1) {
        close(sock);
        ERR_OUT("setsockopt(MCAST_JOIN_GROUP)");
    }
    return sock;

Err:
    return -1;
}

void stream_port_mldv1_stop(void* igmp, int igmp_size, int sock, struct ind_sin *mult_sin)
{//, unsigned int addr
    PRINTF("stream_port_mldv1_stop\n");

    int  ret;
    char ifname[10];
    struct ind_sin *tSin = (struct ind_sin *)igmp;

    /* set group_source_req parameters */
    struct group_req req;
    struct sockaddr_in6 *paddr;

    memset(&req, 0, sizeof(req));
    IND_MEMSET(ifname, 0, 10);

    sys_get_net_ifname(ifname, 10);
    req.gr_interface = if_nametoindex(ifname);

    paddr = (struct sockaddr_in6*)&req.gr_group;
    paddr->sin6_family = AF_INET6;
    //ret = inet_pton(AF_INET6, "ff02::16", &paddr->sin6_addr);
    memcpy(&paddr->sin6_addr, &tSin->in6_addr, sizeof(struct in6_addr));

    ret = setsockopt(sock, IPPROTO_IPV6, MCAST_LEAVE_GROUP, (char *)&req, sizeof(req));
    if (ret == -1) {
        ERR_PRN("setsockopt(MCAST_LEAVE_GROUP)");
    }
    close(sock);
}

int stream_port_mldv2_play(void* igmp, int igmp_size, struct ind_sin *mult_sin)
{//, unsigned int addr, unsigned short port
    PRINTF("stream_port_mldv2_play\n");

    int  sock;
    int  ret;
    char ifname[10];
    struct ind_sin_ex *tSin = (struct ind_sin_ex *)igmp;

    sock = socket(AF_INET6, SOCK_DGRAM, 0);
    if (sock == -1) {
        ERR_OUT("socket create err\n");
    }

    struct sockaddr_in6 saddr6;
    memset(&saddr6, 0, sizeof(saddr6));
    saddr6.sin6_family = AF_INET6;
    saddr6.sin6_addr = in6addr_any;
    saddr6.sin6_port = htons(tSin->host.port);
    ret = bind(sock, (struct sockaddr*)&saddr6, sizeof(saddr6));
    if (ret == -1) {
        close(sock);
        ERR_OUT("socket bind err\n");
    }

    /* set group_source_req parameters */
    struct group_source_req req;
    struct group_req req_normal;
    struct sockaddr_in6 *paddr;

    memset(&req, 0, sizeof(req));
    memset(&req_normal, 0, sizeof(req_normal));
    IND_MEMSET(ifname, 0, 10);

    sys_get_net_ifname(ifname, 10);
    req_normal.gr_interface = req.gsr_interface = if_nametoindex(ifname);

    //paddr = (struct sockaddr_in6*)&req.gsr_source;
    //paddr->sin6_family = AF_INET6;
    //memcpy(&paddr->sin6_addr, &tSin->srcip1.in6_addr, sizeof(struct in6_addr));

    paddr = (struct sockaddr_in6*)&req.gsr_group;
    paddr->sin6_family = AF_INET6;
    memcpy(&paddr->sin6_addr, &tSin->host.in6_addr, sizeof(struct in6_addr));

    paddr = (struct sockaddr_in6*)&req_normal.gr_group;
    paddr->sin6_family = AF_INET6;
    memcpy(&paddr->sin6_addr, &tSin->host.in6_addr, sizeof(struct in6_addr));

    ret = setsockopt(sock, IPPROTO_IPV6, MCAST_JOIN_GROUP, (char *)&req_normal, sizeof(req_normal));
    if (ret == -1) {
        close(sock);
        ERR_OUT("setsockopt(MCAST_JOIN_SOURCE_GROUP)");
    }
    return sock;

Err:
    return -1;
}

void stream_port_mldv2_stop(void* igmp, int igmp_size, int sock, struct ind_sin *mult_sin)
{//, unsigned int addr
    PRINTF("stream_port_mldv2_stop\n");

    int  ret;
    char ifname[10];
    struct ind_sin_ex *tSin = (struct ind_sin_ex *)igmp;

    /* set group_source_req parameters */
    struct group_source_req req;
    struct group_req req_normal;
    struct sockaddr_in6 *paddr;

    memset(&req, 0, sizeof(req));
    memset(&req_normal, 0, sizeof(req_normal));
    IND_MEMSET(ifname, 0, 10);

    sys_get_net_ifname(ifname, 10);
    req_normal.gr_interface = req.gsr_interface = if_nametoindex(ifname);

    //paddr = (struct sockaddr_in6*)&req.gsr_source;
    //paddr->sin6_family = AF_INET6;
    //memcpy(&paddr->sin6_addr, &tSin->srcip1.in6_addr, sizeof(struct in6_addr));

    paddr = (struct sockaddr_in6*)&req.gsr_group;
    paddr->sin6_family = AF_INET6;
    memcpy(&paddr->sin6_addr, &tSin->host.in6_addr, sizeof(struct in6_addr));

    paddr = (struct sockaddr_in6*)&req_normal.gr_group;
    paddr->sin6_family = AF_INET6;
    memcpy(&paddr->sin6_addr, &tSin->host.in6_addr, sizeof(struct in6_addr));

    ret = setsockopt(sock, IPPROTO_IPV6, MCAST_LEAVE_GROUP, (char *)&req_normal, sizeof(req_normal));
    if (ret == -1) {
        ERR_PRN("setsockopt(MCAST_LEAVE_SOURCE_GROUP)");
    }
    close(sock);
}
#endif

#ifdef ENABLE_IGMPV3
int stream_port_multicast_v2_play(void* igmp, int igmp_size, struct ind_sin *mult_sin)
{//, unsigned int addr, unsigned short port
    Igmpv3Info *igmpv3 = (Igmpv3Info *)igmp;

    return stream_port_multicast_play(mult_sin->in_addr.s_addr, mult_sin->port);
}

void stream_port_multicast_v2_stop(void* igmp, int igmp_size, int sock, struct ind_sin *mult_sin)
{//, unsigned int addr
    Igmpv3Info *igmpv3 = (Igmpv3Info *)igmp;

    stream_port_multicast_stop(sock, mult_sin->in_addr.s_addr);

    mid_net_igmpver_set2proc(NULL, igmpv3->version);
    //mid_net_igmpv3srcip_reset(); //TODO no used.

    return;
}

int stream_port_multicast_v3_play(void* igmp, int igmp_size, struct ind_sin *mult_sin)
{//, unsigned int addr, unsigned short port
    Igmpv3Info *igmpv3 = (Igmpv3Info *)igmp;

    char ip[16]; //need think of ipv6.
    char ifname[10];
    int sock, opt;
    struct sockaddr_in tSin;
    struct ip_mreq ipmreqall;
    struct ip_mreq_source ipmreqsrc;
    struct in_addr srcaddr;

    bzero((char *)&tSin, sizeof(tSin));
    tSin.sin_family = AF_INET;
    tSin.sin_addr.s_addr = mult_sin->in_addr.s_addr;
    tSin.sin_port = htons(mult_sin->port);

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
      ERR_OUT("opening socket\n");

    opt = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void*)&opt, sizeof(opt)) < 0)
      ERR_PRN("setsockopt(allow multiple socket use\n");

    if (bind(sock, (struct sockaddr *)&tSin, sizeof(tSin)) < 0) {
        ERR_PRN("call to bind\n");
    }

    opt = SOCKET_BUF_SIZE;
    if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (void*)&opt, sizeof(opt)) < 0)
      ERR_PRN("Can't change system network size (wanted size = %d)\n", opt);

    PRINTF("IGMPv3 srcip is %u, accept\n", igmpv3->srcip1);
    ipmreqsrc.imr_multiaddr.s_addr = mult_sin->in_addr.s_addr;

    IND_MEMSET(ifname, 0, 10);

    network_default_ifname(ifname, 10);
    network_address_get(ifname, ip, sizeof(ip));

    ipmreqsrc.imr_interface.s_addr = inet_addr(ip);

    ipmreqsrc.imr_sourceaddr.s_addr = igmpv3->srcip1;//组播源地址
    if (setsockopt(sock, IPPROTO_IP, IP_ADD_SOURCE_MEMBERSHIP, (char *)&ipmreqsrc, sizeof(ipmreqsrc)) < 0) {
        ERR_PRN("setsocket(add membership)");
    }

    srcaddr.s_addr = igmpv3->srcip1;
    stream_port_srcip_write(0, inet_ntoa(srcaddr));

    return sock;

Err:
    return -1;
}

void stream_port_multicast_v3_stop(void* igmp, int igmp_size, int sock, struct ind_sin *mult_sin)
{//, unsigned int addr
    Igmpv3Info *igmpv3 = (Igmpv3Info *)igmp;

    char ip[16]; //need think of ipv6
    char ifname[10];
    struct ip_mreq ipmreqall;
    struct ip_mreq_source ipmreqsrc;

    if(sock == -1)
      return;

    PRINTF("IGMPv3 srcip is %u, drop it\n", igmpv3->srcip1);
    ipmreqsrc.imr_multiaddr.s_addr = mult_sin->in_addr.s_addr;

    IND_MEMSET(ifname, 0, 10);

    network_default_ifname(ifname, 10);
    network_address_get(ifname, ip, sizeof(ip));

    ipmreqsrc.imr_interface.s_addr = inet_addr(ip);
    ipmreqsrc.imr_sourceaddr.s_addr = igmpv3->srcip1;//组播源地址

    if(setsockopt(sock, IPPROTO_IP, IP_DROP_SOURCE_MEMBERSHIP, (char *)&ipmreqsrc, sizeof(ipmreqsrc)) < 0) {
        ERR_PRN("setsocket(drop membership)");
    }
    PRINTF("@@@@: socket %d level multicast\n", sock);
    close(sock);

    //mid_net_igmpv3srcip_reset(); //TODO no used.

    return;
}
#endif
#if defined(HUAWEI_C10)
void tr069_stream_port_multicast(int or)
{
        char        lostrate_temp[1024] = {0};
        time_t sec;
        struct tm t;
        IND_MEMSET(lostrate_temp,0,sizeof(lostrate_temp));
        sec = mid_time( );
        gmtime_r(&sec, &t);
        if(or==1)
        {
            sprintf(lostrate_temp, "%02d-%02d-%02d:%02d-%02d-%02d|PkgLostRate|IGMPSUCCEEDInfo:&#xD",
                        (t.tm_year + 1900) % 100,
                        t.tm_mon + 1,
                        t.tm_mday,
                        t.tm_hour,
                        t.tm_min,
                        t.tm_sec
                        );

             TR069_LOG_POST(lostrate_temp,LOG_MSG_IGMPINFO, "IGMPSucceedInfo");
        }
        else
        {
            sprintf(lostrate_temp, "%02d-%02d-%02d:%02d-%02d-%02d|PkgLostRate|IGMPFailInfo:&#xD",
                        (t.tm_year + 1900) % 100,
                        t.tm_mon + 1,
                        t.tm_mday,
                        t.tm_hour,
                        t.tm_min,
                        t.tm_sec
                        );
            TR069_LOG_POST(lostrate_temp,LOG_MSG_IGMPINFO, "IGMPFailInfo");
        }

        return;

}

#endif
int stream_port_multicast_play(u_int addr, u_short port)
{
    char ip[16]; //need think of ipv6
    int sock, opt;
    struct sockaddr_in tSin;
    struct ip_mreq ipmreq;

    bzero((char *)&tSin, sizeof(tSin));
    tSin.sin_family = AF_INET;
    tSin.sin_addr.s_addr = addr;
    tSin.sin_port = htons(port);

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
      ERR_OUT("opening socket\n");

    opt = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void*)&opt, sizeof(opt)) < 0)
      ERR_PRN("setsockopt(allow multiple socket use\n");
    if (bind(sock, (struct sockaddr *)&tSin, sizeof(tSin)) < 0) {
        ERR_PRN("call to bind\n");
    }

    opt = SOCKET_BUF_SIZE;
    if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (void*)&opt, sizeof(opt)) < 0)
      ERR_PRN("Can't change system network size (wanted size = %d)\n", opt);

    ipmreq.imr_multiaddr.s_addr = addr;

    char ifname[10];
    network_default_ifname(ifname, 10);
    int connectType = network_connecttype_get(ifname);
#if defined(Jiangsu)
    if (NETTYPE_PPPOE != connectType)
#endif
    {
        network_address_get(ifname, ip, sizeof(ip));
        ipmreq.imr_interface.s_addr = inet_addr(ip);
        if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&ipmreq, sizeof(ipmreq)) < 0)
            ERR_PRN("setsocket(add membership)");
    }

    if (NETTYPE_PPPOE == connectType) {
        char address[64];
        network_address_get("ppp0", address, sizeof(address));
        ipmreq.imr_interface.s_addr = inet_addr(address);
        if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&ipmreq, sizeof(ipmreq)) < 0) {
            ERR_PRN("setsocket(add membership)");
        }
    }

    g_is_multicast_play = 1;
    g_multicast_addr = addr;
    g_multicast_port = port;
#if defined(HUAWEI_C10)
    tr069_stream_port_multicast(1);
#endif
    return sock;
Err:
    return -1;
}

void stream_port_multicast_stop(int sock, u_int addr)
{
    char ip[16];
    struct ip_mreq ipmreq;

    g_is_multicast_play = 0;
    if (sock == -1)
      return;
    ipmreq.imr_multiaddr.s_addr = addr;

    char ifname[10];
    network_default_ifname(ifname, 10);
    int connectType = network_connecttype_get(ifname);
#if defined(Jiangsu)
    if (NETTYPE_PPPOE != connectType)
#endif
    {
        network_address_get(ifname, ip, sizeof(ip));
        ipmreq.imr_interface.s_addr = inet_addr(ip);
        if (setsockopt(sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char *)&ipmreq, sizeof(ipmreq)) < 0)
            ERR_PRN("setsocket(add membership)");
    }

    if (NETTYPE_PPPOE == connectType) {
        char    address[64];
        network_address_get("ppp0", address, sizeof(address));
        ipmreq.imr_interface.s_addr = inet_addr(address);
        if (setsockopt(sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char *)&ipmreq, sizeof(ipmreq)) < 0) {
            ERR_PRN("setsocket(add membership)");
        }
    }

    close(sock);
}

static int in_cksum(u_short *addr, int len)
{
    register int nleft = len;
    register u_short *w = addr;
    register int sum = 0;
    u_short answer = 0;

    while (nleft > 1) {
        sum += *w++;
        nleft -= 2;
    }

    if (nleft == 1) {
        *(u_char *)(&answer) = *(u_char *)w;
        sum += answer;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = ~sum;
    return(answer);
}

static int igmp_send(u_int dstaddr, caddr_t igmpHeader, int type)
{
    int i = 0;
    int sock = -1;
    char buf[24] = {0};
    char opt[4] = {0x94, 0x04, 0x00, 0x00};

    struct in_addr mif;
    struct sockaddr_in tSin;

    sock = socket(AF_INET, SOCK_RAW, IPPROTO_IGMP);
    if (sock < 0)
      ERR_OUT("socket\n");

    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char*)&i, sizeof(i)) < 0)
      ERR_OUT("setsockopt SO_BROADCAST\n");

    if (setsockopt(sock, IPPROTO_IP, IP_OPTIONS, opt, 4) < 0)
      ERR_OUT("setsockopt IP_OPTIONS\n");

    char ifname[10];
    network_default_ifname(ifname, 10);
    network_address_get(ifname, buf, sizeof(buf));
    mif.s_addr = inet_addr(buf);
    if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_IF, (char*)&mif, sizeof(mif)) < 0)
      ERR_OUT("setsockopt IP_MULTICAST_IF\n");

    memset(&tSin, 0, sizeof(struct sockaddr_in));
    tSin.sin_family = AF_INET;
    tSin.sin_addr.s_addr = inet_addr(buf);
    if (bind(sock, (struct sockaddr*)&tSin, sizeof(struct sockaddr)) < 0)
      ERR_OUT("bind\n");

    memset(&tSin, 0 ,sizeof(struct sockaddr_in));
    tSin.sin_family = AF_INET;
    tSin.sin_addr.s_addr = dstaddr;

    if(type == 0) {
        if(sendto(sock, (caddr_t)igmpHeader, sizeof(struct igmp), 0, (struct sockaddr*)&tSin, sizeof(struct sockaddr)) < 0)
          ERR_OUT("sendto\n");
    } else if(type == 1) {
        if(sendto(sock, (caddr_t)igmpHeader, sizeof(struct igmpv3), 0, (struct sockaddr*)&tSin, sizeof(struct sockaddr)) < 0)
          ERR_OUT("sendto\n");
    } else if(type == 2) {
        if(sendto(sock, (caddr_t)igmpHeader, sizeof(struct igmpv3_spc), 0, (struct sockaddr*)&tSin, sizeof(struct sockaddr)) < 0)
          ERR_OUT("sendto\n");
    } else {
        ERR_OUT("sendto type err, is %d\n", type);
    }

    PRINTF("igmp_send OK!\n");

    close(sock);
    return 0;
Err:
    if (sock >= 0)
      close(sock);
    return -1;
}

static void igmp_report(u_int addr, u_int srcip)
{
    struct igmp igmpHeader;
    int ver = 2;
    int type = 0;

    if (addr == INADDR_ANY || addr == INADDR_NONE)
      ERR_OUT("addr = %08x\n", addr);

#ifdef ENABLE_IGMPV3
    struct igmpv3 igmp;
    struct igmpv3_spc igmp_spc;

    appSettingGetInt("igmpversion", &ver, 0);
    if(ver == 2) {
#endif
        PRINTF("igmp version is %d", ver);
        type = 0;
        memset(&igmpHeader, 0 , sizeof(igmpHeader));
        igmpHeader.igmp_type = 0x16;
        igmpHeader.igmp_code = 0;
        igmpHeader.igmp_group.s_addr = addr;
        igmpHeader.igmp_cksum = in_cksum((u_short *)&igmpHeader, sizeof(igmpHeader));

        igmp_send(addr, (void *)&igmpHeader, type);
#ifdef ENABLE_IGMPV3
    } else if(ver == 3) {
        PRINTF("igmp version is %d, srcip = %d", ver, srcip);
        if(srcip == 0) {
            type = 1;
            memset(&igmp, 0 , sizeof(struct igmpv3));
            igmp.igmp_type = 0x22;
            igmp.igmp_code = 0;
            igmp.igmp_reserve = 0;
            igmp.igmp_ngr = htons(1);

            igmp.group.igmp_rtype = 0x4;
            igmp.group.igmp_adlen = 0;
            igmp.group.igmp_numsrc = htons(0);
            igmp.group.igmp_group.s_addr = addr;
            igmp.group.igmp_sources = 0x0;

            igmp.igmp_cksum = in_cksum((u_short *)&igmp, sizeof(struct igmpv3));
            igmp_send(inet_addr("224.0.0.22"), (void *)&igmp, type);
        } else {
            type = 2;
            memset(&igmp_spc, 0 , sizeof(struct igmpv3_spc));
            igmp_spc.igmp_type = 0x22;
            igmp_spc.igmp_code = 0;
            igmp_spc.igmp_reserve = 0;
            igmp_spc.igmp_ngr = htons(2);

            igmp_spc.group[0].igmp_rtype = 0x5;
            igmp_spc.group[0].igmp_adlen = 0;
            igmp_spc.group[0].igmp_numsrc = htons(1);
            igmp_spc.group[0].igmp_group.s_addr = addr;
            igmp_spc.group[0].igmp_sources = srcip;

            igmp_spc.group[1].igmp_rtype = 0x3;
            igmp_spc.group[1].igmp_adlen = 0;
            igmp_spc.group[1].igmp_numsrc = htons(1);
            igmp_spc.group[1].igmp_group.s_addr = addr;
            igmp_spc.group[1].igmp_sources = srcip;

            igmp_spc.igmp_cksum = in_cksum((u_short *)&igmp_spc, sizeof(struct igmpv3_spc));
            igmp_send(inet_addr("224.0.0.22"), (void *)&igmp_spc, type);
        }
    } else {
        ERR_OUT("igmpver err, is %d\n", ver);
    }
#endif

Err:
    return;
}

#ifdef ENABLE_IPV6
static int igmp_leave_v6(const char *addr)
{
    int i = 0;
    int sock = -1;
    int hoplimit = 1;
    char ifname[10];

    IND_MEMSET(ifname, 0, 10);
    sys_get_net_ifname(ifname, 10);

    unsigned int if_index = if_nametoindex(ifname);
    char ip6hbh[8] = {0x3a, 0x00, 0x05, 0x02, 0x00, 0x00, 0x01, 0x00};//ip6 hop by hop option, insteed of ip4 option

    struct icmpHeader icmp;
    struct sockaddr_in6 tSin;

    icmp.igmp[0].mode = 0x06;
    icmp.igmp[0].len = 0x00;
    icmp.igmp[0].reserve = htons(1);
    inet_pton(AF_INET6, "ff02::16", &icmp.igmp[0].muladdr);
    inet_pton(AF_INET6, addr, &icmp.igmp[0].srcaddr);

#if 0
    //join
    icmp.igmp[1].mode = 0x03;
    icmp.igmp[1].len = 0x00;
    icmp.igmp[1].reserve = htons(1);
    inet_pton(AF_INET6, "ff02::16", &icmp.igmp[1].muladdr);
    inet_pton(AF_INET6, addr, &icmp.igmp[1].srcaddr);
#endif

    icmp.type = 0x8f;
    icmp.code = 0x00;
    icmp.optnum = htonl(1);//opt num;
    icmp.checksum = htons(0);

    sock = socket(PF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
    if (sock < 0)
      ERR_OUT("socket\n");

    if (setsockopt(sock, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &hoplimit, sizeof(hoplimit)) < 0)
      ERR_OUT("setsockopt IP_OPTIONS, %s\n", strerror(errno));

    if (setsockopt(sock, IPPROTO_IPV6, IPV6_HOPOPTS, ip6hbh, sizeof(ip6hbh)) < 0)
      ERR_OUT("setsockopt IP_MULTICAST_IF, %s\n", strerror(errno));

    if (setsockopt(sock, IPPROTO_IPV6, IPV6_MULTICAST_IF, &if_index, sizeof(if_index)) < 0)
      ERR_OUT("setsockopt IP_MULTICAST_IF, %s\n", strerror(errno));

    memset(&tSin, 0, sizeof(struct sockaddr_in6));

    tSin.sin6_family = PF_INET6;
    inet_pton(AF_INET6, "ff02::16", &tSin.sin6_addr.s6_addr);

    if(sendto(sock, &icmp, sizeof(icmp), 0, (struct sockaddr*)&tSin, sizeof(struct sockaddr_in6)) < 0)
      ERR_OUT("sendto\n");

    PRINTF("sendto OK!\n");
    close(sock);
    return 0;
Err:
    if (sock >= 0)
      close(sock);
    return -1;
}
#endif

static void igmp_leave(u_int addr, u_int srcip)
{
    struct igmp igmpHeader;
    int ver = 2;
    int type = 0;

    if (addr == INADDR_ANY || addr == INADDR_NONE)
      ERR_OUT("addr = %08x\n", addr);

#ifdef ENABLE_IGMPV3
    struct igmpv3 igmp;

    appSettingGetInt("igmpversion", &ver, 0);
    if(ver == 2) {
#endif
        PRINTF("igmp version is [%d]\n", ver);
        type = 0;
        memset(&igmpHeader, 0 , sizeof(igmpHeader));
        igmpHeader.igmp_type = 0x17;
        igmpHeader.igmp_code = 0;
        igmpHeader.igmp_group.s_addr = addr;
        igmpHeader.igmp_cksum = in_cksum((u_short *)&igmpHeader, sizeof(igmpHeader));

        igmp_send(inet_addr("224.0.0.2"), (void *)&igmpHeader, type);
#ifdef ENABLE_IGMPV3
    } else if(ver == 3) {
        PRINTF("igmp version is %d, srcip = %d", ver, srcip);
        type = 1;
        memset(&igmp, 0 , sizeof(struct igmpv3));
        igmp.igmp_type = 0x22;
        igmp.igmp_code = 0;
        igmp.igmp_reserve = 0;
        igmp.igmp_ngr = htons(1);

        if(srcip == -1 || srcip == 0) {
            igmp.group.igmp_rtype = 0x3;
            igmp.group.igmp_numsrc = htons(0);
            igmp.group.igmp_sources = htonl(0);
        } else {
            igmp.group.igmp_rtype = 0x6;
            igmp.group.igmp_numsrc = htons(1);
            igmp.group.igmp_sources = srcip;
        }

        igmp.group.igmp_adlen = 0;
        igmp.group.igmp_group.s_addr = addr;

        igmp.igmp_cksum = in_cksum((u_short *)&igmp, sizeof(struct igmpv3));

        igmp_send(inet_addr("224.0.0.22"), (void *)&igmp, type);
    } else {
        ERR_OUT("igmpver err, is %d\n", ver);
    }
#endif

Err:
    return;
}

void stream_port_multicast_report(u_int addr)
{
    PRINTF("addr = %08x\n", addr);
    unsigned int srcip = 0;

#ifdef ENABLE_IGMPV3
    //srcip = mid_net_igmpv3srcip_get(); //TODO no used.
    //unsigned int srcip = inet_addr("110.1.1.103");
#endif
    igmp_report(addr, srcip);
}

static void int_multicast_leave(int idx)
{
    char buf[40];
    u_int addr = 0;
    u_int srcip = 0;

    stream_port_multicast_read(idx, buf);
    if ('[' == buf[0]) {
#ifdef ENABLE_IPV6
    igmp_leave_v6(buf);
#endif
    } else {
        addr = inet_addr(buf);
        stream_port_srcip_read(idx, buf);
        srcip = inet_addr(buf);
        if (addr != INADDR_ANY && addr != INADDR_NONE)
          igmp_leave(addr, srcip);
    }
}

void stream_multicast_leave(void)
{
    PRINTF("run here !\n");

    int_multicast_leave(0);
    int_multicast_leave(1);
}

/*
mld: Multicast Listener Discovery
*/
void stream_port_mld_report(struct ind_sin* mld_sin)
{
}

void stream_port_post_datavalid(int pIndex, int valid)
{
}

void stream_port_post_bandwidth(int mult, int width, int rate)
{
    //PRINTF("@@@@@@@@: bandwidth = %d\n", rate);
}

extern void mqm_set_sockadd(struct sockaddr_in* serv_sin, struct sockaddr_in* data_sin, char *url);

#if (defined( SQM_VERSION_C22 )|| defined(SQM_VERSION_C23) ||defined(SQM_VERSION_C26) || defined(SQM_VERSION_C28)||defined(SQM_VERSION_ANDROID))
static int mqm_vaild = 0;
#endif

#if (defined(SQM_VERSION_C26) || defined(SQM_VERSION_C28)||defined(SQM_VERSION_ANDROID))
void stream_port_post_datasock(int pIndex, int tcp, struct sockaddr_in* serv_sin, struct sockaddr_in* data_sin, char *url)
{
    if(pIndex != 0){
        PRINTF("@@@@@@@@@  index:%d\n", pIndex);
        return;
    }
    if (serv_sin)
      PRINTF("[SQM_PORT_C26]: server ip = %s port = %d\n", inet_ntoa(serv_sin->sin_addr), (unsigned int)ntohs(serv_sin->sin_port));
    if (data_sin)
      PRINTF("[SQM_PORT_C26]: data ip = %s port = %d\n", inet_ntoa(data_sin->sin_addr), (unsigned int)ntohs(data_sin->sin_port));
    if(url)
      PRINTF("[SQM_PORT_C26]: url = %s\n", url);
    PRINTF("[SQM_PORT_C26]: tcp = %d\n", tcp);

    /* Eagle port. 2011年01月19日 */
    CHN_STAT chn_stat;
    SQM_MSG  sqm_msg;

    if(tcp == -1) {
        chn_stat = STB_IDLE;
        sqm_msg = MSG_STOP;
        mqm_vaild = 0;
    }else if(tcp == STRM_STATE_PAUSE ) {
        chn_stat = PAUSE_TYPE;
        sqm_msg = MSG_PAUSE;
        mqm_vaild = 0;
    }else if(tcp == STRM_STATE_FAST) {
        chn_stat = FAST_BB_BF_TYPE;
        sqm_msg = MSG_FAST;
        mqm_vaild = 0;
    } else {
        if(serv_sin != NULL)
            chn_stat = UNICAST_CHANNEL_TYPE;
        else
            chn_stat = MULTICAST_CHANNEL_TYPE;
        sqm_msg = MSG_PLAY;
        mqm_vaild = 1;
    }

    PRINTF("[SQM_PORT_C26]: channel state = %d\n", chn_stat);
    parseChnInfo(chn_stat, serv_sin, data_sin, url);
    sqm_port_pushdata_clear();
    int ret = sqm_port_buildmsg( sqm_msg);
    if (-1 == ret)
        mid_timer_create(15, 1,  sqm_port_buildmsg, sqm_msg);

    return;
}
#else
void stream_port_post_datasock(int pIndex, int tcp, struct sockaddr_in* serv_sin, struct sockaddr_in* data_sin, char *url)
{
    if(pIndex != 0) {
        PRINTF("@@@@@@@@@  index:%d\n", pIndex);
        return;
    }

    if (serv_sin)
        PRINTF("[SQM_PORT]: server ip = %s port = %d\n", inet_ntoa(serv_sin->sin_addr), (unsigned int)ntohs(serv_sin->sin_port));
    if (data_sin)
        PRINTF("[SQM_PORT]: data ip = %s port = %d\n", inet_ntoa(data_sin->sin_addr), (unsigned int)ntohs(data_sin->sin_port));
    if(url)
        PRINTF("[SQM_PORT]: url = %s\n", url);
    PRINTF("[SQM_PORT]: tcp = %d\n", tcp);

#if (defined( SQM_VERSION_C21) || defined( SQM_VERSION_C22 )|| defined(SQM_VERSION_C23) || defined(C23T_SQM_OPEN))
    CHN_STAT chn_stat;
    if(tcp == -1) {
        chn_stat = STB_IDLE;
#if (defined( SQM_VERSION_C22 )|| defined(SQM_VERSION_C23) || defined(C23T_SQM_OPEN))
        mqm_vaild = 0;
#endif
    } else if(tcp == STRM_STATE_PAUSE || tcp == STRM_STATE_FAST) {
        chn_stat = FAST_BB_BF_TYPE;
#if (defined( SQM_VERSION_C22 )|| defined(SQM_VERSION_C23) || defined(C23T_SQM_OPEN))
        mqm_vaild = 0;
#endif
    } else {
        if(serv_sin != NULL)
            chn_stat = UNICAST_CHANNEL_TYPE;
        else
            chn_stat = MULTICAST_CHANNEL_TYPE;
/*#ifdef SQM_VERSION_C23
        if(APP_TYPE_HLS == mid_stream_get_apptype(pIndex)
            || APP_TYPE_HTTP_HLS == mid_stream_get_apptype(pIndex)) {
            chn_stat = HTTP_UNICAST_TYPE;
        }
#endif*/
#if (defined( SQM_VERSION_C22 )|| defined(SQM_VERSION_C23) || defined(C23T_SQM_OPEN))
        mqm_vaild = 1;
#endif
    }

    PRINTF("[SQM_PORT]: channel state = %d\n", chn_stat);

    parseChnInfo(chn_stat, serv_sin, data_sin, url);

    sqm_port_pushdata_clear();
    sqm_port_msg_write(MSG_SETINFO);
#endif
    return;
}
#endif

void stream_port_post_datapush(int pIndex, char *buf, int len, int scale)
{
#if (defined( SQM_VERSION_C22 )|| defined(SQM_VERSION_C23) ||defined(SQM_VERSION_C26) || defined(SQM_VERSION_C28) ||defined(SQM_VERSION_ANDROID))
    if(1 == mqm_vaild){
        sqm_port_pushdata(pIndex, buf, len, scale);
    }else{
        ;
    }
#endif
}

int stream_port_multicast_get_address(unsigned int *addr, unsigned short *port)
{
    *addr = g_multicast_addr;
    *port = g_multicast_port;
    return g_is_multicast_play;
}

