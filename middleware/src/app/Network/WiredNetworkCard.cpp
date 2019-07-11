#include "WiredNetworkCard.h"
#include "NetlinkStateHandler.h"

#include <fcntl.h>
#include <errno.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#if defined(hi3560e)
#include "ethtool-util.h"
#else
#include <linux/ethtool.h>
#endif
#include <linux/sockios.h>

#include <string.h>
#include <stdio.h>
#include <stdint.h>

WiredNetworkCard::WiredNetworkCard(const char* devname) : NetworkCard(devname)
{
    setType(NT_ETHERNET);
    mStateHandler = new NetlinkStateHandler(this);
}

WiredNetworkCard::~WiredNetworkCard()
{
    if (mStateHandler)
        delete mStateHandler;
    mStateHandler = 0;
}

int
WiredNetworkCard::linkStatus()
{
    struct ifreq ifr;
    struct ethtool_value status;

    bzero(&ifr, sizeof(ifr));
    strncpy(ifr.ifr_name, devname(), IFNAMSIZ);
    ifr.ifr_data = (char*)&status;
    status.cmd = ETHTOOL_GLINK;
    status.data = 0;

    if (-1 == ioctl(mSockFd, SIOCETHTOOL, &ifr)) {
        NETWORK_LOG_ERR("ioctl: %s\n", strerror(errno));
        return NL_ERR_UNKNOW;
    }

    return status.data > 0 ? NL_FLG_RUNNING : NL_FLG_DOWN;
}

int
WiredNetworkCard::flagChange(int flag)
{
    NETWORK_LOG_INFO("%s\n", NetlinkFlagStr(flag));
    if ((flag & IFF_UP) && (flag & IFF_RUNNING)) {
        if (LS_ETHERNET_UP == mLinkState)
            return 0;
        mLinkState = LS_ETHERNET_UP;
    } else {
        if (LS_ETHERNET_DOWN == mLinkState)
            return 0;
        mLinkState = LS_ETHERNET_DOWN;
    }
    return mStateHandler->handleState(mLinkState);
}

int
WiredNetworkCard::setAutoNego()
{
    struct ifreq ifr;
    struct ethtool_cmd ecmd;
    ecmd.cmd = ETHTOOL_GSET;
    ecmd.autoneg = AUTONEG_ENABLE;
	ifr.ifr_data = (char*)&ecmd;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, devname(), sizeof(ifr.ifr_name) - 1);

    if (-1 == ioctl(mSockFd, SIOCETHTOOL, &ifr)) {
        NETWORK_LOG_ERR("ioctl: %s\n", strerror(errno));
        return -1;
    }

    ecmd.advertising = ecmd.supported &
        ( ADVERTISED_10baseT_Half
        | ADVERTISED_10baseT_Full
        | ADVERTISED_100baseT_Half
        | ADVERTISED_100baseT_Full
        | ADVERTISED_1000baseT_Half
        | ADVERTISED_1000baseT_Full
        | ADVERTISED_2500baseX_Full
        | ADVERTISED_10000baseT_Full);

    ecmd.cmd = ETHTOOL_SSET;
    if (-1 == ioctl(mSockFd, SIOCETHTOOL, &ifr)) {
        NETWORK_LOG_ERR("ioctl: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

int
WiredNetworkCard::setLinkSpeed(int speed)
{
    int sockfd = -1;
    struct ifreq ifr;
    struct ethtool_cmd ecmd;

    ecmd.cmd = ETHTOOL_GSET;
    ecmd.autoneg = AUTONEG_DISABLE;
	ifr.ifr_data = (char*)&ecmd;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, mDeviceName.c_str(), sizeof(ifr.ifr_name) - 1);

    if (-1 == ioctl(mSockFd, SIOCETHTOOL, &ifr)) {
        NETWORK_LOG_ERR("ioctl: %s\n", strerror(errno));
        return -1;
    }

    //ethtool_cmd_speed_set(&ecmd, speed); //hi3560e not support
    ecmd.speed = (uint16_t)speed;
    ecmd.cmd = ETHTOOL_SSET;
    if (-1 == ioctl(mSockFd, SIOCETHTOOL, &ifr)) {
        NETWORK_LOG_ERR("ioctl: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

int 
WiredNetworkCard::getLinkSpeed()
{
    return 0;
}
