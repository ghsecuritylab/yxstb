#include "NetworkAssertions.h"
#include "NetworkInterface.h"
#include "NetworkCard.h"
#include "NetworkTypes.h"

#include "MessageValueNetwork.h"

#include "libzebra.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <poll.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netinet/ether.h>
#include <netpacket/packet.h>
#include <linux/if.h>

#ifdef ANDROID
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#endif

extern "C" int yos_systemcall_runSystemCMD(char*, int*);

NetworkInterface::NetworkInterface(NetworkCard* device, const char* ifname)
    : mProtocolType(PT_STATIC)
    , mAddressType(AT_IPV4)
    , mNetworkCard(device)
    , mConnectionTimes(0), mConnectionUpTime(0)
    , mActivity(false)
{
    if (ifname && strlen(ifname) > 0)
        mIfaceName = ifname;
    else
        mIfaceName = device->devname();
    mNetworkCard->addIfname(mIfaceName);
    pthread_mutex_init(&mMutex, 0);
}

NetworkInterface::NetworkInterface(NetworkCard* device, int vlanId)
    : mProtocolType(PT_STATIC)
    , mAddressType(AT_IPV4)
    , mNetworkCard(device)
    , mConnectionTimes(0), mConnectionUpTime(0)
    , mActivity(false)
{
    char ifacename[32] = { 0 };
    sprintf(ifacename, "%s.%d", device->devname(), vlanId);
    mIfaceName = ifacename;
    mNetworkCard->addIfname(mIfaceName);
    pthread_mutex_init(&mMutex, 0);
}

NetworkInterface::~NetworkInterface()
{
    pthread_mutex_destroy(&mMutex);
}

int
NetworkInterface::setProtocolType(ProtocolType_e type)
{
    if (mProtocolType == type)
        return 0;
    mProtocolType = type;
    return 0;
}

void
NetworkInterface::setMac(const char* mac)
{
    if (mac)
        mIfaceMac = mac;
}

const char*
NetworkInterface::getMac()
{
    return mIfaceMac.c_str();
}

void
NetworkInterface::setIPv4Setting(IPv4Setting& setting)
{
    mIPv4Conf = setting;
}

IPv4Setting&
NetworkInterface::getIPv4Setting()
{
    return mIPv4Conf;
}

void
NetworkInterface::setIPv6Setting(IPv6Setting& setting)
{
    mIPv6Conf = setting;
}

IPv6Setting&
NetworkInterface::getIPv6Setting()
{
    return mIPv6Conf;
}

void
NetworkInterface::setIPConflictSetting(IPConflictSetting& setting)
{
    mIPConflictConf = setting;
}

IPConflictSetting&
NetworkInterface::getIPConflictSetting()
{
    return mIPConflictConf;
}

void
NetworkInterface::setDHCPSetting(DHCPSetting& setting)
{
    mDHCPConf = setting;
}

DHCPSetting&
NetworkInterface::getDHCPSetting()
{
    return mDHCPConf;
}

void
NetworkInterface::setPPPSetting(PPPSetting& setting)
{
    mPPPConf = setting;
}

PPPSetting&
NetworkInterface::getPPPSetting()
{
    return mPPPConf;
}

unsigned int
NetworkInterface::getConnectionUpTime()
{
    if (!mConnectionUpTime)
        return 0;

    struct timespec times;
    clock_gettime(CLOCK_REALTIME, &times);
    return (times.tv_sec * 1000 + times.tv_nsec / 1000000) - mConnectionUpTime;
}

void
NetworkInterface::stateActiveChanaged(bool active)
{
    mConnectionUpTime = 0;
    if (active) {
        struct timespec times;
        clock_gettime(CLOCK_REALTIME, &times);
        mConnectionUpTime = times.tv_sec * 1000 + times.tv_nsec / 1000000;
    }
    mActivity = active;
}

unsigned int
NetworkInterface::getConnectionTimes()
{
    return mConnectionTimes;
}

const char*
NetworkInterface::getAddress(char* addr, int len)
{
    if (AT_IPV6 == mAddressType)
        snprintf(addr, len, "%s", mIPv6Conf.getAddress());
    else
        snprintf(addr, len, "%s", mIPv4Conf.getAddress());
    return addr;
}

const char*
NetworkInterface::getNetmask(char* addr, int len)
{
    if (AT_IPV6 == mAddressType)
        snprintf(addr, len, "%d", mIPv6Conf.getSubnetPrefix()); //TODO
    else
        snprintf(addr, len, "%s", mIPv4Conf.getNetmask());
    return addr;
}

const char*
NetworkInterface::getGateway(char* addr, int len)
{
    if (AT_IPV6 == mAddressType)
        snprintf(addr, len, "%s", mIPv6Conf.getGateway());
    else
        snprintf(addr, len, "%s", mIPv4Conf.getGateway());
    return addr;
}

const char*
NetworkInterface::getDns0(char* addr, int len)
{
    if (AT_IPV6 == mAddressType)
        snprintf(addr, len, "%s", mIPv6Conf.getDns0());
    else
        snprintf(addr, len, "%s", mIPv4Conf.getDns0());
    return addr;
}

const char*
NetworkInterface::getDns1(char* addr, int len)
{
    if (AT_IPV6 == mAddressType)
        snprintf(addr, len, "%s", mIPv6Conf.getDns1());
    else
        snprintf(addr, len, "%s", mIPv4Conf.getDns1());
    return addr;
}

int
InitArpingSocket(const char* ifname)
{
    int sfd = -1;
    int ret = 0;
    struct ifreq ifr;
    struct sockaddr_ll me;

    sfd = socket(AF_PACKET, SOCK_DGRAM, 0);
    if (sfd < 0) {
        NETWORK_LOG_ERR("socket: %s\n", strerror(errno));
        return -1;
    }

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    ret = ioctl(sfd, SIOCGIFINDEX, (void*)&ifr);
    if (ret < 0) {
        NETWORK_LOG_ERR("ioctl: %s\n", strerror(errno));
        close(sfd);
        return -1;
    }
    me.sll_ifindex = ifr.ifr_ifindex;

    me.sll_family = AF_PACKET;
    me.sll_protocol = htons(ETH_P_ARP);
    if (bind(sfd, (struct sockaddr*)&me, sizeof(me)) < 0) {
        NETWORK_LOG_ERR("bind: %s\n", strerror(errno));
        close(sfd);
        return -1;
    }
    return sfd;
}

int
SendArpingPacket(int sfd, const char* srcip, const char* dstip)
{
    NETWORK_LOG_INFO("sockfd[%d] destip[%s]\n", sfd, dstip);
    /*  arp
     *  |----------------------------------------------------------------------------------------------------------------------|
     *  |hardwareType | protocolType | len_hardware_addr | len_protocol_adrr | oper_code | src_mac | src_ip | dst_mac | dst_ip |
     *  |   16bit     |    16bit     |        8bit       |       8bit        |   16bit   |  48bit  |  32bit |  48bit  |  32bit |
     *  |----------------------------------------------------------------------------------------------------------------------|
     */
    int ret = -1;
    socklen_t alen;
    struct in_addr src;
    struct in_addr dst;
    struct sockaddr_ll me;
    struct sockaddr_ll he;

    memset(&src, 0, sizeof(src));
    if (srcip && !inet_aton(srcip, &src)) {
        NETWORK_LOG_ERR("address invalid\n");
        return -1;
    }

    memset(&dst, 0, sizeof(dst));
    if (!dstip || !inet_aton(dstip, &dst)) {
        NETWORK_LOG_ERR("address invalid\n");
        return -1;
    }

    alen = sizeof(me);
    ret = getsockname(sfd, (struct sockaddr*)&me, &alen);
    if (ret < 0) {
        NETWORK_LOG_ERR("getsockname: %s\n", strerror(errno));
        return -1;
    }
    if (!me.sll_halen) {
        NETWORK_LOG_ERR("is not ARPable (no ll address)");
        return -1;
    }

    he = me;
    memset(he.sll_addr, -1, he.sll_halen);

    //NETWORK_LOG_INFO("ARPING Dst %s\n", inet_ntoa(dst));
    //NETWORK_LOG_INFO("ARPING Src %s\n", inet_ntoa(src));

    unsigned char arpmsg[256] = { 0 };
    struct arphdr *ah = (struct arphdr*)arpmsg;
    unsigned char *p = (unsigned char*)(ah + 1);

    ah->ar_hrd = htons(ARPHRD_ETHER);
    ah->ar_pro = htons(ETH_P_IP);
    ah->ar_hln = me.sll_halen;
    ah->ar_pln = 4;
    ah->ar_op = htons(ARPOP_REQUEST);

    memcpy(p, &me.sll_addr, ah->ar_hln);
    p += ah->ar_hln;
    memcpy(p, &src, 4);
    p += 4;
    memcpy(p, &he.sll_addr, ah->ar_hln);
    p += ah->ar_hln;
    memcpy(p, &dst, 4);
    p += 4;

    if (sendto(sfd, arpmsg, p - arpmsg, 0, (struct sockaddr*)&he, sizeof(he)) < 0) {
        NETWORK_LOG_ERR("sendto: %s", strerror(errno));
        return -1;
    }
    return 0;
}

int
RecvArpingPacket(int sfd, const char* dstip, char* mac, int len)
{
    unsigned char packet[4096] = { 0 };
    unsigned char *p = 0;
    socklen_t alen = 0;
    int ret = -1, num = 0;
    struct sockaddr_ll me;
    struct sockaddr_ll from;
    struct in_addr dst;
    struct in_addr src;
    struct arphdr *ah = 0;
    struct ifreq ifr;

    alen = sizeof(me);
    ret = getsockname(sfd, (struct sockaddr*)&me, &alen);
    if (ret < 0) {
        NETWORK_LOG_ERR("getsockname: %s\n", strerror(errno));
        return -1;
    }

    if (!inet_aton(dstip, &dst)) {
        NETWORK_LOG_ERR("address invalid\n");
        return -1;
    }

    alen = sizeof(from);
    bzero(&from, alen);
    if ((num = recvfrom(sfd, packet, 4096, 0, (struct sockaddr*)&from, &alen)) <= 0)
        return -1;

    ah = (struct arphdr*)packet;
    p = (unsigned char*)(ah + 1);
    struct in_addr retsrc, retdst;

    if (from.sll_pkttype != PACKET_HOST
        && from.sll_pkttype != PACKET_BROADCAST
        && from.sll_pkttype != PACKET_MULTICAST)
        return 0;

    if (ah->ar_op != htons(ARPOP_REQUEST) && ah->ar_op != htons(ARPOP_REPLY))
        return 0;

    if (ah->ar_hrd != htons(from.sll_hatype)
        && (from.sll_hatype != ARPHRD_FDDI || ah->ar_hrd != htons(ARPHRD_ETHER)))
        return 0;

    if (ah->ar_pro != htons(ETH_P_IP)
        || (ah->ar_pln != 4)
        || (ah->ar_hln != me.sll_halen)
        || (num < (int)(sizeof(*ah) + 2 * (4 + ah->ar_hln))))
        return 0;

    memcpy(&retsrc.s_addr, p + ah->ar_hln, 4);
    memcpy(&retdst.s_addr, p + ah->ar_hln + 4 + ah->ar_hln, 4);

    if (dst.s_addr != retsrc.s_addr)
        return 0;

    memset(&src, 0, sizeof(src));
    if ((memcmp(p, &me.sll_addr, me.sll_halen) == 0)
        || (src.s_addr && src.s_addr != retdst.s_addr))
        return 0;

    memcpy(mac, ether_ntoa((struct ether_addr*)p), len);

    return 1;
}
