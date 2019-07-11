/*******************************************************************************
    公司：
            Yuxing software
    纪录：
            2008-1-26 21:12:26 create by Liu Jianhua
    模块：
            tr069
    简述：
            标准C语言函数扩展
 *******************************************************************************/

#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <net/if.h>
#include <net/route.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "tr069_header.h"
#include "tr069_api.h"

#if defined(ANDROID) && defined(Sichuan)
#include "cutils/properties.h"
#endif

#define DEVNAME1 "eth0:"
#define DEVNAME2 "rausb0:"

static const unsigned char g_base64o[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const unsigned char g_base64i[81] = "\76XXX\77\64\65\66\67\70\71\72\73\74\75XXXXXXX\00\01\02\03\04\05\06\07\10\11\12\13\14\15\16\17\20\21\22\23\24\25\26\27\30\31XXXXXX\32\33\34\35\36\37\40\41\42\43\44\45\46\47\50\51\52\53\54\55\56\57\60\61\62\63";

/******************************************************************************/
int base64_encode(const u_char *inbuf, u_int inlen, u_char *outbuf, u_int outsize)
{
    u_int outlen;
    u_int m;

    if (outsize <= ((inlen + 3) / 3) * 4)
        TR069ErrorOut("outsize = %d, inlen = %d\n", outsize, inlen);
    outlen = 0;
    for (; inlen > 2; inlen -= 3, inbuf += 3) {
        m = inbuf[0];
        m = (m << 8) | inbuf[1];
        m = (m << 8) | inbuf[2];

        outbuf[0] = g_base64o[(m >> 18) & 0x3F];
        outbuf[1] = g_base64o[(m >> 12) & 0x3F];
        outbuf[2] = g_base64o[(m >> 6 ) & 0x3F];
        outbuf[3] = g_base64o[(m >> 0 ) & 0x3F];
        outlen += 4;
        outbuf += 4;
    }
    if (inlen > 0) {
        m = inbuf[0];
        if (inlen == 2)
            m = (m << 8) | inbuf[1];
        else
            m <<= 8;
        m <<= 8;
        outbuf[0] = g_base64o[(m >> 18) & 0x3F];
        outbuf[1] = g_base64o[(m >> 12) & 0x3F];
        if (inlen == 2)
            outbuf[2] = g_base64o[(m >> 6 ) & 0x3F];
        else
            outbuf[2] = '=';
        outbuf[3] = '=';
        outlen += 4;
    }

    return (int)outlen;
Err:
    return -1;
}

/******************************************************************************/
int base64_decode(const u_char *inbuf, u_int inlen, u_char *outbuf, u_int outsize)
{
    u_int i, j, loops, fill, outlen;
    u_int m;
    u_char ch, *buf;

    fill = 0;
    buf = outbuf;
    loops = inlen / 4;
    outlen = 0;
    if (inlen % 4)
        TR069ErrorOut("(inlen = %d\n", inlen);
    if (outsize <= loops * 3)
        TR069ErrorOut("outsize = %d, loops = %d\n", outsize, loops);

    for (i = 0; i < loops; i ++) {
        m = 0;
        for (j = 0; j < 4; j ++) {
            ch = inbuf[j];
            if (ch == '=') {
                fill ++;
                ch = 0;
            } else {
                if (fill > 0)
                    TR069ErrorOut("fill = %d\n", fill);
                if (ch < '+' || ch > 'z')
                    TR069ErrorOut("ch = 0x%02x\n", (u_int)ch);
                ch = g_base64i[ch - '+'];
                if (ch == 'X')
                    TR069ErrorOut("X\n");
            }
            m = (m << 6) + ch;
        }
        if (buf) {
            buf[0] = (unsigned char)((m >> 16) & 0xff);
            buf[1] = (unsigned char)((m >> 8 ) & 0xff);
            buf[2] = (unsigned char)((m >> 0 ) & 0xff);
            buf += 3;
        }
        outlen += 3;
        inbuf += 4;
    }
    if (outbuf)
        outbuf[outlen - fill] = 0;

    return (int)outlen;
Err:
    return -1;
}

//------------------------------------------------------------------------------
u_int tr069_atoui(const char *str)
{
    int i;
    char ch;
    u_int n = 0;

    for (i = 0; i < 10; i ++) {
        ch = str[i];
        if (ch < '0' || ch > '9')
            return n;
        n = n * 10 + (ch - '0');
    }
    return n;
}

//------------------------------------------------------------------------------
int tr069_strdup(char **pp, const char *str)
{
    int len;
    char *p;

    p = *pp;
    if (p) {
        free(p);
        *pp = NULL;
    }
    if (!str)
        return -1;

    len = strlen(str);

    p = (char *)IND_MALLOC(len + 1);
    if (!p)
        return -1;
    strcpy(p, str);
    *pp = p;

    return 0;
}

//------------------------------------------------------------------------------
void tr069_strncpy(const char *src, char *dst, int len)
{
    while (len > 0 && *src) {
        *dst ++ = *src ++;
        len --;
    }
    *dst = 0;
}

//------------------------------------------------------------------------------
void tr069_linencpy(const char *src, char *dst, int len)
{
    while (len > 0 && isprint(*src)) {
        *dst ++ = *src ++;
        len --;
    }
    *dst = 0;
}

//CCYY-MM-DDThh:mm:ss
int tr069_str2time(const char* str, u_int *pVal)
{
    struct tm tm;

    if (!str)
        return 0;
    if (sscanf(str, "%04d-%02d-%02dT%02d:%02d:%02d", 
            &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
            &tm.tm_hour, &tm.tm_min, &tm.tm_sec) != 6)
        return -1;

    tm.tm_year -= 1900;
    tm.tm_mon -= 1;

    *pVal = mktime(&tm);

    return 0;
}

int tr069_time2str(uint32_t sec, char* buf)
{
    int len = 0;

    if (!buf)
        TR069ErrorOut("buf is null\n");

    if (sec == 0) {
        buf[0] = 0;
    } else {
        time_t t;
        struct tm tm;

        memset(&tm, 0, sizeof(tm));
        t = sec;
        localtime_r(&t, &tm);
        len = sprintf(buf, "%04d-%02d-%02dT%02d:%02d:%02d",
                        tm.tm_year + 1900,
                        tm.tm_mon + 1,
                        tm.tm_mday,
                        tm.tm_hour,
                        tm.tm_min,
                        tm.tm_sec);
    }

    return len;
Err:
    return 0;
}

//------------------------------------------------------------------------------
int tr069_str2int(const char *strval, int *pVal)
{
    int value;

    if (sscanf(strval, "%d", &value) != 1)
        return -1;
    *pVal = value;

    return 0;
}

//------------------------------------------------------------------------------
int tr069_str2uint(const char *strval, u_int *pVal)
{
    u_int value;

    if (sscanf(strval, "%u", &value) != 1)
        return -1;
    *pVal = value;

    return 0;
}

int tr069_checkurl(char* url, char *uri, int uri_len, char *host, int host_len, unsigned short *port)
{
    char            *p, *p2;
    int                len;

    if (!url)
        TR069ErrorOut("url is NULL\n");
    if (strnicmp(url, "http://", 7))
        TR069ErrorOut("url error! %s\n", url);

    url += 7;
    p = strchr(url, '/');
    if (!p) {
        p = url + strlen(url);
        if (2 >= uri_len)
            TR069ErrorOut("uri_len = %d\n", uri_len);
        if (uri)
            strcpy(uri, "/");
    } else {
        if ((int)strlen(p) >= uri_len)
            TR069ErrorOut("uri_len = %d\n", uri_len);
        if (uri)
            strcpy(uri, p);
    }
    p2 = strchr(url, ':');
    if (!p2 || p < p2) {
        len = p - url;
        if (port)
            *port = 80;
    } else {
        len = p2 - url;
        if (port)
            *port = (unsigned short)atoi(p2 + 1);
    }
    if (len <= 4 || len > host_len)
        TR069ErrorOut("len = %d, host_len = %d\n", len, host_len);

    if (host) {
        strncpy(host, url, (u_int)len);
        host[len] = '\0';
    }

    return 0;
Err:
    return -1;
}

#define LOG_LINE_SIZE    95

unsigned int tr069_ipAddrDot2Int(char *src)
{
   int byte1, byte2, byte3, byte4;
   unsigned int addr;

   sscanf(src, "%d.%d.%d.%d", &byte1, &byte2, &byte3, &byte4);
   addr = (byte1&0xFF) << 24 |
          (byte2&0xFF) << 16 |
          (byte3&0xFF) << 8 |
          (byte4&0xFF);
   
#if 0
   int i;
   printf("%#x.%#x.%#x.%#x\n", byte1, byte2, byte3, byte4);
   char *byte = (char*)&addrDeci;
   for(i = 0; i < 4; i++){
       printf("%#x ", byte[i]);
   }
   printf("\n");
#endif

   return addr;
}

int tr069_ipAddrInt2Dot(unsigned int src, char *dst, int dstLen)
{
    char *byte = (char*)&src;

    if(!dst || dstLen < 16){
        TR069Printf("invalid parameter dst or dstLen\n");
        return -1;
    }

    snprintf(dst, dstLen, "%d.%d.%d.%d", byte[3]&0xFF, byte[2]&0xFF, byte[1]&0xFF, byte[0]&0xFF);
 
    TR069Printf("Address dotFomat = %s\n", dst);
    
    return 0;
}

unsigned int tr069_10ms(void)
{
    unsigned int clk;
    struct timespec tp;

    clock_gettime(CLOCK_MONOTONIC, &tp);

    clk = (unsigned int)tp.tv_sec * 100 + (unsigned int)tp.tv_nsec / 10000000;

    return clk;
}

unsigned int tr069_msec(void)
{
    unsigned int msec;
    struct timespec tp;

    clock_gettime(CLOCK_MONOTONIC, &tp);

    msec = (unsigned int)tp.tv_sec * 1000 + (unsigned int)tp.tv_nsec / 1000000;

    return msec;
}

unsigned int tr069_sec(void)
{
    unsigned int sec;
    struct timespec tp;

    clock_gettime(CLOCK_MONOTONIC, &tp);

    sec = (unsigned int)tp.tv_sec;

    return sec;
}

struct TR069Msgq {
    int size;
    int fds[2];
};

TR069Msgq_t tr069_msgq_create(int msg_num, int msg_size)
{
    TR069Msgq_t msgq;

    msgq = (TR069Msgq_t)malloc(sizeof(struct TR069Msgq));
    if (!msgq)
        TR069ErrorOut("malloc\n");

    if (pipe(msgq->fds)) {
        free(msgq);
        TR069ErrorOut("pipe\n");
    }
    fcntl(msgq->fds[0], F_SETFL, fcntl(msgq->fds[0], F_GETFL) | O_NONBLOCK);
    fcntl(msgq->fds[1], F_SETFL, fcntl(msgq->fds[1], F_GETFL) | O_NONBLOCK);
    msgq->size = msg_size;

    return msgq;
Err:
    return NULL;
}

void tr069_msgq_delete(TR069Msgq_t msgq)
{
    if (!msgq)
        TR069ErrorOut("msgq = %p\n", msgq);
    close(msgq->fds[0]);
    close(msgq->fds[1]);
    free(msgq);

Err:
    return;
}

int tr069_msgq_fd(TR069Msgq_t msgq)
{
    if (!msgq)
        TR069ErrorOut("msgq = %p\n", msgq); 
    return msgq->fds[0];
Err:
    return -1;
}

int tr069_msgq_putmsg(TR069Msgq_t msgq, char *msg)
{
    int ret;

    if (!msgq)
        TR069ErrorOut("msgq = %p\n", msgq); 

    ret = write(msgq->fds[1], msg, msgq->size);
    if (ret != msgq->size)
        TR069ErrorOut("ret = %d, msgq->size = %d\n", ret, msgq->size);

    return 0;
Err:
    return -1;
}

int tr069_msgq_getmsg(TR069Msgq_t msgq, char *msg)
{
    int ret;

    if (!msgq)
        TR069ErrorOut("msgq = %p\n", msgq); 

    ret = read(msgq->fds[0], msg, msgq->size);
    if (ret != msgq->size)
        TR069ErrorOut("ret = %d, msgq->size = %d\n", ret, msgq->size);

    return 0;
Err:
    return -1;
}

#define LINESIZE 512
struct net_stats {
    unsigned long long rx_packets; /* total packets received */
    unsigned long long tx_packets; /* total packets transmitted */
    unsigned long long rx_bytes; /* total bytes received */
    unsigned long long tx_bytes; /* total bytes transmitted */
    unsigned long rx_errors;     /* bad packets received */
    unsigned long tx_errors;     /* packet transmit problems */
    unsigned long rx_dropped; /* no space in linux buffers */
    unsigned long tx_dropped; /* no space available in linux */
    unsigned long rx_multicast; /* multicast packets received */
    unsigned long rx_compressed;
    unsigned long tx_compressed;
    unsigned long collisions;

    /* detailed rx_errors: */
    unsigned long rx_frame_errors;    /* recv'd frame alignment error */
    unsigned long rx_fifo_errors;    /* recv'r fifo overrun          */
    /* detailed tx_errors */
    unsigned long tx_carrier_errors;
    unsigned long tx_fifo_errors;
};

static int if_readlist(struct net_stats *stats)
{
    FILE *fp = NULL;
    char buf[LINESIZE] = {0};
    char DEVNAME[10] = {0};

    IND_MEMSET(stats, 0, sizeof(struct net_stats));

    fp = fopen("/proc/net/dev", "r");
    if (!fp)
        return -1;

    fgets(buf, LINESIZE, fp);    /* eat line */
    fgets(buf, LINESIZE, fp);

    if (1)
        IND_STRCPY(DEVNAME, DEVNAME1);
    else
        IND_STRCPY(DEVNAME, DEVNAME2);

    while (fgets(buf, LINESIZE, fp)) {
        char *p = strstr(buf, DEVNAME);
        if (!p)
            continue;
        sscanf(p + strlen(DEVNAME), "%llu%llu%lu%lu%lu%lu%lu%lu%llu%llu%lu%lu%lu%lu%lu%lu",
            &stats->rx_bytes, /* missing for 0 */
            &stats->rx_packets,
            &stats->rx_errors,
            &stats->rx_dropped,
            &stats->rx_fifo_errors,
            &stats->rx_frame_errors,
            &stats->rx_compressed, /* missing for <= 1 */
            &stats->rx_multicast, /* missing for <= 1 */
            &stats->tx_bytes, /* missing for 0 */
            &stats->tx_packets,
            &stats->tx_errors,
            &stats->tx_dropped,
            &stats->tx_fifo_errors,
            &stats->collisions,
            &stats->tx_carrier_errors,
            &stats->tx_compressed /* missing for <= 1 */
        );
        break;
    }
    fclose(fp);

    return 0;
}

/*------------------------------------------------------------------------------
    Total number of IP payload bytes sent over this interface
    since the device was last restarted as specified in DeviceInfo.UpTime.
 ------------------------------------------------------------------------------*/
u_int tr069_get_TotalBytesSent(void)
{
    struct net_stats stats;
    if_readlist(&stats);
    return stats.tx_bytes;
}

/*------------------------------------------------------------------------------
    Total number of IP payload bytes received over this interface
    since the device was last restarted as specified in DeviceInfo.UpTime.
 ------------------------------------------------------------------------------*/
u_int tr069_get_TotalBytesReceived(void)
{
    struct net_stats stats;
    if_readlist(&stats);
    return stats.rx_bytes;
}

/*------------------------------------------------------------------------------
    Total number of IP packets sent over this interface
    since the device was last restarted as specified in DeviceInfo.UpTime.
 ------------------------------------------------------------------------------*/
u_int tr069_get_TotalPacketsSent(void)
{
    struct net_stats stats;
    if_readlist(&stats);
    return stats.tx_packets;
}

/*------------------------------------------------------------------------------
    Total number of IP packets received over this interface
    since the device was last restarted as specified in DeviceInfo.UpTime.
 ------------------------------------------------------------------------------*/
u_int tr069_get_TotalPacketsReceived(void)
{
    struct net_stats stats;
    if_readlist(&stats);
    return stats.rx_packets;
}

void tr069_logPrefix(const char *file, const int line, const char *func)
{
    uint32_t ms, h, m, s;
    char *name;

    ms = tr069_msec( );

    s  = ms / 1000;
    ms = ms % 1000;

    m = s / 60;
    s = s % 60;

    h = m / 60;
    m = m % 60;

    name = strrchr((char*)file, '/');
    if (name)
        name ++;
    else
        name = (char*)file;

    printf("[%03d:%02d:%02d.%03d] : [%s:%04d:%s] ", h, m, s, ms, name, line, func);
}

static void network_route_configure(unsigned int address)
{
#if defined(ANDROID) && defined(Sichuan)
    char netmask[20] = { 0 };
    char gateway[20] = { 0 };
    char ipaddr[20] = { 0 };
    struct in_addr netmaskTmp;
    struct in_addr gatewayTmp;
    inet_ntop(AF_INET, (void *)&address, ipaddr, 128);

    tr069_port_getValue("Device.LAN.SubnetMask", netmask, sizeof(netmask));
    TR069Printf("vlan netmask = %s\n", netmask);
    tr069_port_getValue("Device.LAN.DefaultGateway", gateway, sizeof(gateway));
    TR069Printf("vlan gateway = %s\n", gateway);
    property_set("hw.iptvConnectIp", ipaddr);
    property_set("hw.iptvConnectNetmask", netmask);
    property_set("hw.iptvConnectGateWay", gateway);
    property_set("ctl.start", "hwroute");
#endif
}

ssize_t tr069_sendto(int socket, const void *message, size_t length,
    int flags, const struct sockaddr_in *address)
{
    network_route_configure(address->sin_addr.s_addr);
    return sendto(socket, message, length, flags, (struct sockaddr*)address, sizeof(struct sockaddr_in));
}

int tr069_connect(int socket, const struct sockaddr_in *address)
{
    network_route_configure(address->sin_addr.s_addr);
    return connect(socket, (struct sockaddr*)address, sizeof(struct sockaddr_in));
}