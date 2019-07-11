#include "Tr069Stats.h"

#include "Tr069GroupCall.h"
#include "Tr069FunctionCall.h"
#include "TR069Assertions.h"

#include "tr069_port.h"
#include "mid/mid_time.h"

#include <string.h>

#ifdef TR069_VERSION_2

#define LINESIZE 512

#define DEVNAME1 "eth0:"
#define DEVNAME2 "rausb0:"


struct _if_stats {
    int    begintime;
    unsigned int if_ipackets;    /* packets received on interface */
    unsigned int if_opackets;    /* packets sent on interface */
    unsigned int if_ibytes;        /* total number of octets received */
    unsigned int if_obytes;        /* total number of octets sent */
};

extern struct _if_stats g_tr069Currentday, g_tr069Quarterhour;

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
        ERR_OUT("fopen\n");

    fgets(buf, LINESIZE, fp);	/* eat line */
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
Err:
    return -1;
}


/*------------------------------------------------------------------------------
	网卡连接网络的时间(秒为单位)
 ------------------------------------------------------------------------------*/
static
int tr069_ConnectionUpTime_Read(char* value, unsigned int length)
{
    unsigned int ret = -1;
    char ifname[URL_LEN] = { 0 };
    network_default_ifname(ifname, URL_LEN);
    ret = network_interface_uptime(ifname);
    sprintf(value, "%u", ret);
    return 0;

}

/*------------------------------------------------------------------------------
	Total number of IP payload bytes sent over this interface
	since the device was last restarted as specified in DeviceInfo.UpTime.
 ------------------------------------------------------------------------------*/
static
int tr069_TotalBytesSent_Read(char* value, unsigned int length)
{
    struct net_stats stats;
    if_readlist(&stats);

    if (value)
        sprintf(value, "%d", stats.tx_bytes); // 原先由return 返回，现由value 返回
    else
        return stats.tx_bytes;

    return 0;
}

/*------------------------------------------------------------------------------
	Total number of IP payload bytes received over this interface
	since the device was last restarted as specified in DeviceInfo.UpTime.
 ------------------------------------------------------------------------------*/
static
int tr069_TotalBytesReceived_Read(char* value, unsigned int length)
{
    struct net_stats stats;
    if_readlist(&stats);

    if (value)
        sprintf(value, "%d", stats.tx_bytes); // 原先由return 返回，现由value 返回
    else
        return stats.rx_bytes;

    return 0;
}

/*------------------------------------------------------------------------------
	Total number of IP packets sent over this interface
	since the device was last restarted as specified in DeviceInfo.UpTime.
 ------------------------------------------------------------------------------*/
static
int tr069_TotalPacketsSent_Read(char* value, unsigned int length)
{
    struct net_stats stats;
    if_readlist(&stats);

    if (value)
        sprintf(value, "%u", stats.tx_packets);
    else
        return stats.tx_packets;

    return 0;
}

/*------------------------------------------------------------------------------
	Total number of IP packets received over this interface
	since the device was last restarted as specified in DeviceInfo.UpTime.
 ------------------------------------------------------------------------------*/
static
int tr069_TotalPacketsReceived_Read(char* value, unsigned int length)
{
    struct net_stats stats;
    if_readlist(&stats);

    if (value)
        sprintf(value, "%u", stats.rx_packets);
    else
        return stats.rx_packets;

    return 0;
}

static
int tr069_CurrentDayInterval_Read(char* value, unsigned int length)
{
    unsigned int ret = ((unsigned int)(mid_clock() / 1000) - (unsigned int)g_tr069Currentday.begintime);
    sprintf(value, "u", ret);
}

static
int tr069_CurrentDayBytesSent_Read(char* value, unsigned int length)
{
    unsigned int ret = tr069_TotalBytesSent_Read(NULL, 0) - g_tr069Currentday.if_obytes;
    sprintf(value, "%u", ret);
    return 0;
}

static
int tr069_CurrentDayBytesReceived_Read(char* value, unsigned int length)
{
    unsigned int ret = (tr069_TotalBytesReceived_Read(NULL, 0) - g_tr069Currentday.if_ibytes);
    sprintf(value, "%u", ret);
    return 0;
}

static
int tr069_CurrentDayPacketsSent_Read(char* value, unsigned int length)
{
    unsigned int ret = (tr069_TotalPacketsSent_Read(NULL, 0) - g_tr069Currentday.if_opackets);
    sprintf(value, "%u", ret);
    return 0;
}

static
int tr069_CurrentDayPacketsReceived_Read(char* value, unsigned int length)
{
    unsigned int ret = (tr069_TotalPacketsReceived_Read(NULL, 0) - g_tr069Currentday.if_ipackets);
    sprintf(value, "%u", ret);
    return 0;
}

static
int tr069_QuarterHourInterval_Read(char* value, unsigned int length)
{
    unsigned int ret = ((unsigned int)(mid_clock() / 1000) - (unsigned int)g_tr069Quarterhour.begintime);
    sprintf(value, "%u", ret);
    return 0;
}

static
int tr069_QuarterHourBytesSent_Read(char* value, unsigned int length)
{
    unsigned int ret = tr069_TotalBytesSent_Read(NULL, 0) - g_tr069Currentday.if_obytes;
    sprintf(value, "%u", ret);
    return 0;
}

static
int tr069_QuarterHourBytesReceived_Read(char* value, unsigned int length)
{
    unsigned int ret = tr069_TotalBytesReceived_Read(NULL, 0) - g_tr069Currentday.if_ibytes;
    sprintf(value, "%u", ret);
    return 0;
}

static
int tr069_QuarterHourPacketsSent_Read(char* value, unsigned int length)
{
    unsigned int ret = tr069_TotalPacketsSent_Read(NULL, 0) - g_tr069Currentday.if_opackets;
    sprintf(value, "%u", ret);
    return 0;
}

static
int tr069_QuarterHourPacketsReceived_Read(char* value, unsigned int length)
{
    unsigned int ret = tr069_TotalPacketsReceived_Read(NULL, 0) - g_tr069Currentday.if_ipackets;
    sprintf(value, "%u", ret);
    return 0;
}



Tr069Stats::Tr069Stats()
	: Tr069GroupCall("Stats")
{

    Tr069Call* connup = new Tr069FunctionCall("ConnectionUpTime",               tr069_ConnectionUpTime_Read,           NULL);
    Tr069Call* totlBSent = new Tr069FunctionCall("TotalBytesSent",              tr069_TotalBytesSent_Read,             NULL);
    Tr069Call* totlBRecv = new Tr069FunctionCall("TotalBytesReceived",          tr069_TotalBytesReceived_Read,         NULL);
    Tr069Call* totlPsent = new Tr069FunctionCall("TotalPacketsSent",            tr069_TotalPacketsSent_Read,           NULL);
    Tr069Call* totlPRecv = new Tr069FunctionCall("TotalPacketsReceived",        tr069_TotalPacketsReceived_Read,       NULL);
    Tr069Call* dayInterv = new Tr069FunctionCall("CurrentDayInterval",          tr069_CurrentDayInterval_Read,         NULL);
    Tr069Call* dayByteSent = new Tr069FunctionCall("CurrentDayBytesSent",       tr069_CurrentDayBytesSent_Read,        NULL);
    Tr069Call* dayByteRecv = new Tr069FunctionCall("CurrentDayBytesReceived",   tr069_CurrentDayBytesReceived_Read,    NULL);
    Tr069Call* dayPSent = new Tr069FunctionCall("CurrentDayPacketsSent",        tr069_CurrentDayPacketsSent_Read,      NULL);
    Tr069Call* dayPRecv = new Tr069FunctionCall("CurrentDayPacketsReceived",    tr069_CurrentDayPacketsReceived_Read,  NULL);
    Tr069Call* quterInterv = new Tr069FunctionCall("QuarterHourInterval",       tr069_QuarterHourInterval_Read,        NULL);
    Tr069Call* quterBSent = new Tr069FunctionCall("QuarterHourBytesSent",       tr069_QuarterHourBytesSent_Read,       NULL);
    Tr069Call* quterBRecv = new Tr069FunctionCall("QuarterHourBytesReceived",   tr069_QuarterHourBytesReceived_Read,   NULL);
    Tr069Call* quterPsent = new Tr069FunctionCall("QuarterHourPacketsSent",     tr069_QuarterHourPacketsSent_Read,     NULL);
    Tr069Call* quterPRecv = new Tr069FunctionCall("QuarterHourPacketsReceived", tr069_QuarterHourPacketsReceived_Read, NULL);

    regist(connup->name(), connup);
    regist(totlBSent->name(), totlBSent);
    regist(totlBRecv->name(), totlBRecv);
    regist(totlPsent->name(), totlPsent);
    regist(totlPRecv->name(), totlPRecv);
    regist(dayInterv->name(), dayInterv);
    regist(dayByteSent->name(), dayByteSent);
    regist(dayByteRecv->name(), dayByteRecv);
    regist(dayPSent->name(), dayPSent);
    regist(dayPRecv->name(), dayPRecv);
    regist(quterInterv->name(), quterInterv);
    regist(quterBSent->name(), quterBSent);
    regist(quterBRecv->name(), quterBRecv);
    regist(quterPsent->name(), quterPsent);
    regist(quterPRecv->name(), quterPRecv);

}

Tr069Stats::~Tr069Stats()
{
}
#endif

