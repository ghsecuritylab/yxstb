#include "NetworkAssertions.h"
#include "NetworkCard.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#include <arpa/inet.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>

#if defined(hi3560e)
#include "ethtool-util.h"
#else
#include <linux/ethtool.h>
#endif
#include <linux/sockios.h>

NetworkCard::NetworkCard(const char* devname) : mDeviceName(devname), mLinkState(LS_UNKNOWN), mSockFd(-1)
{
    if (-1 == (mSockFd = socket(AF_INET, SOCK_DGRAM, 0))) {
        NETWORK_LOG_ERR("socket: %s\n", strerror(errno));
        return;
    }
}

NetworkCard::~NetworkCard()
{
    mIfaceNames.clear();
    if (mSockFd > 0)
        close(mSockFd);
    mSockFd = -1;
}

int
NetworkCard::linkUp()
{
    NETWORK_LOG_INFO("linkUp\n");
    struct ifreq ifr;

    bzero(&ifr, sizeof(ifr));
    strncpy(ifr.ifr_name, devname(), IFNAMSIZ);
    if (-1 == ioctl(mSockFd, SIOCGIFFLAGS, &ifr)) {
        NETWORK_LOG_ERR("ioctl: %s\n", strerror(errno));
        return -1;
    }
    strncpy(ifr.ifr_name, devname(), IFNAMSIZ);
    ifr.ifr_flags |= (IFF_UP | IFF_RUNNING);
    if (-1 == ioctl(mSockFd, SIOCSIFFLAGS, &ifr)) {
        NETWORK_LOG_ERR("ioctl: %s, Don't worry if Android!\n", strerror(errno));
        return -1;
    }
    return 0;
}

int
NetworkCard::linkDown()
{
    NETWORK_LOG_INFO("linkDown\n");
    struct ifreq ifr;

    bzero(&ifr, sizeof(ifr));
    strncpy(ifr.ifr_name, devname(), IFNAMSIZ);
    if (-1 == ioctl(mSockFd, SIOCGIFFLAGS, &ifr)) {
        NETWORK_LOG_ERR("ioctl: %s\n", strerror(errno));
        return -1;
    }
    strncpy(ifr.ifr_name, devname(), IFNAMSIZ);
    ifr.ifr_flags &= ~(IFF_UP | IFF_RUNNING);
    if (-1 == ioctl(mSockFd, SIOCSIFFLAGS, &ifr)) {
        NETWORK_LOG_ERR("ioctl: %s, Don't worry if Android!\n", strerror(errno));
        return -1;
    }
    return 0;
}

bool 
NetworkCard::addIfname(std::string& ifname)
{
    std::vector<std::string>::iterator it;
    for (it = mIfaceNames.begin(); it != mIfaceNames.end(); ++it) {
        if (ifname == *it)
            return false;
    }
    mIfaceNames.push_back(ifname);
    return true;
}

bool 
NetworkCard::delIfname(std::string& ifname)
{
    std::vector<std::string>::iterator it;
    for (it = mIfaceNames.begin(); it != mIfaceNames.end(); ++it) {
        if (ifname == *it)
            mIfaceNames.erase(it);
    }
    return true;
}

std::vector<std::string> 
NetworkCard::getIfnames()
{
    return mIfaceNames;
}

//Debug Use, will delete
const char* NetlinkFlagStr(unsigned int fl)
{
    static char buf[512] = { 0 };
    static struct flag {
        const char *name;
        unsigned int flag;
    } flags[] = {
#define  F(x)   { #x, IFF_##x }
        F(UP),
        F(BROADCAST),
        F(DEBUG),
        F(LOOPBACK),
        F(POINTOPOINT),
        F(NOTRAILERS),
        F(RUNNING),
        F(NOARP),
        F(PROMISC),
        F(ALLMULTI),
        F(MASTER),
        F(SLAVE),
        F(MULTICAST),
#undef F
    };
    char *cp = buf;

    *cp = '\0';

    for (unsigned int i = 0; i < sizeof(flags)/sizeof(*flags); i++) {
        if (fl & flags[i].flag) {
            fl &= ~flags[i].flag;
            cp += sprintf(cp, "%s,", flags[i].name);
        }
    }

    if (fl != 0)
        cp += sprintf(cp, "%x,", fl);

    if (cp != buf)
        cp[-1] = '\0';

    return buf;
}

const char* NetlinkStatStr(unsigned int st) //Debug
{
    switch (st) {
    case NetworkCard::LS_ETHERNET_UP:
        return "Ethernet Up";
    case NetworkCard::LS_ETHERNET_DOWN:
        return "Ethernet Down";
    case NetworkCard::LS_WIRELESS_UP:
        return "Wireless Up";
    case NetworkCard::LS_WIRELESS_DOWN:
        return "Wireless Down";
    case NetworkCard::LS_WIRELESS_JOIN_FAIL:
        return "Wireless Join Fail";
    case NetworkCard::LS_WIRELESS_JOIN_SECCESS:
        return "Wireless Join Seccess";
    case NetworkCard::LS_WIRELESS_CHECK_SIGNAL:
        return "Wireless Check Signal";
    default:
        ;
    }
    return "State Unknown";
}
