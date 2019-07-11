#include "NetworkAssertions.h"
#include "NetworkInterfaceLinux.h"
#include "NetworkCard.h"
#include "NetworkTypes.h"

#include "MessageValueNetwork.h"

#include "DHCPStateHandler.h"
#include "PPPOEStateHandler.h"

#include "osex_net.h"
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

NetworkInterfaceLinux::NetworkInterfaceLinux(NetworkCard* device, const char* ifname)
    : NetworkInterface(device, ifname)
    , mCheckFd(-1), mStateHandler(0)
    , mRfd(-1)
{
}

NetworkInterfaceLinux::NetworkInterfaceLinux(NetworkCard* device, int vlanId)
    : NetworkInterface(device, vlanId)
    , mCheckFd(-1), mStateHandler(0)
    , mRfd(-1)
{
}

NetworkInterfaceLinux::~NetworkInterfaceLinux()
{
    disconnect();
}

int
NetworkInterfaceLinux::connect(int mode) //TODO param mode is not used.
{
    NETWORK_LOG_INFO("NetworkInterfaceLinux::connect type[%d]\n", mProtocolType);
    if (mRfd > 0) {
        NETWORK_LOG_WARN("already running!\n");
        disconnect();
    }
    pthread_mutex_lock(&mMutex);
    char confpath[128] = { 0 };
    char fifoname[128] = { 0 };
    char scripcmd[128] = { 0 };
    switch (mProtocolType) {
    case PT_STATIC:
        if (AT_IPV4 == mAddressType) {
            osex_ipaddr_set(mIfaceName.c_str(), (char*)mIPv4Conf.getAddress());
            osex_ipmask_set(mIfaceName.c_str(), (char*)mIPv4Conf.getNetmask());
            osex_iproute_add("0.0.0.0", "0.0.0.0", (char*)mIPv4Conf.getGateway(), 5);
            yos_net_setDNSServer((char*)mIPv4Conf.getDns0(), (char*)mIPv4Conf.getDns1());
        } else {
            yos_net_addIPv6Address((char*)mIfaceName.c_str(), (char*)mIPv6Conf.getAddress(), mIPv6Conf.getSubnetPrefix());
            yos_net_setIPv6DefaultGateway((char*)mIfaceName.c_str(), (char*)mIPv6Conf.getGateway());
        }
        mStateHandler = new StateHandler(this);
        mStateHandler->handleState(1);
        goto End;
    case PT_DHCP:
        if (access(DHCP_ROOT_DIR, F_OK) < 0) {
            mkdir(NETWORK_DIR, 0);
            mkdir(DHCP_ROOT_DIR, 0);
        }
        snprintf(confpath, sizeof(confpath), "%s/dhclient-%s.conf", DHCP_ROOT_DIR, mIfaceName.c_str());
        snprintf(fifoname, sizeof(fifoname), "%s/dhclient-%s.pipe", DHCP_ROOT_DIR, mIfaceName.c_str());
        snprintf(scripcmd, sizeof(scripcmd), "dhclient.connect AF=%d DBDIR=%s DEBUG=%d IFACE=%s", (AT_IPV4 == mAddressType ? 4 : 6), DHCP_ROOT_DIR, 1, mIfaceName.c_str());
        InitDhcpConfigureFile(confpath, &mDHCPConf);
        mStateHandler = new DHCPStateHandler(this);
        break;
    case PT_PPPOE:
        if (access(PPPOE_ROOT_DIR, F_OK) < 0) {
            mkdir(NETWORK_DIR, 0);
            mkdir(PPPOE_ROOT_DIR, 0);
        }
        snprintf(fifoname, sizeof(fifoname), "%s/ppp-%s.pipe", PPPOE_ROOT_DIR, mIfaceName.c_str());
        snprintf(scripcmd, sizeof(scripcmd), "ppp.connect.sh %s %s %s",
            mPPPConf.getUsername(),
            mPPPConf.getPassword(),
            mIfaceName.c_str());
        mStateHandler = new PPPOEStateHandler(this);
        break;
    default:
        goto End;
    }
    if (-1 == mkfifo(fifoname, 00777) && (EEXIST != errno)) {
        NETWORK_LOG_ERR("mkfifo:%s %s\n", fifoname, strerror(errno));
        goto End;
    }
    mRfd = open(fifoname, O_RDONLY | O_NONBLOCK, 0); //3560E not supoort: O_CLOEXEC
    NETWORK_LOG_INFO("open :%s [%d]\n", fifoname, mRfd);
    yos_systemcall_runSystemCMD(scripcmd, 0);
    mConnectionTimes++;
End:
    pthread_mutex_unlock(&mMutex);
    return mRfd;
}

int NetworkInterfaceLinux::disconnect()
{
    NETWORK_LOG_INFO("NetworkInterfaceLinux::disconnect\n");
    pthread_mutex_lock(&mMutex);

    char confpath[128] = { 0 };
    char fifoname[128] = { 0 };
    char scripcmd[128] = { 0 };

    stateActiveChanaged(false);
    if (mStateHandler)
        delete(mStateHandler);
    mStateHandler = 0;
    if (AT_IPV4 == mAddressType) {
        osex_ipaddr_set(mIfaceName.c_str(), (char*)"0.0.0.0");
        osex_ipmask_set(mIfaceName.c_str(), (char*)"255.255.0.0");
    } else {
        yos_net_delIPv6Address((char*)mIfaceName.c_str(), (char*)mIPv6Conf.getAddress(), mIPv6Conf.getSubnetPrefix());
    }
    switch (mProtocolType) {
    case PT_STATIC:
        break;
    case PT_DHCP:
        snprintf(confpath, sizeof(confpath), "%s/dhclient-%s.conf", DHCP_ROOT_DIR, mIfaceName.c_str());
        snprintf(fifoname, sizeof(fifoname), "%s/dhcpcd-%s.pipe", DHCP_ROOT_DIR, mIfaceName.c_str());
        snprintf(scripcmd, sizeof(scripcmd), "dhclient.disconnect AF=%d DBDIR=%s IFACE=%s", (AT_IPV4 == mAddressType ? 4 : 6), DHCP_ROOT_DIR, mIfaceName.c_str());
        unlink(confpath);
        break;
    case PT_PPPOE:
        snprintf(fifoname, sizeof(fifoname), "%s/ppp-%s.pipe", PPPOE_ROOT_DIR, mIfaceName.c_str());
        snprintf(scripcmd, sizeof(scripcmd), "ppp.disconnect.sh %s", mIfaceName.c_str());
        break;
    default:
        goto End;
    }
    stopCheckIP();
    if (mRfd > 0) {
        close(mRfd);
        mRfd = -1;
        unlink(fifoname);
        yos_systemcall_runSystemCMD(scripcmd, 0);
    }
End:
    pthread_mutex_unlock(&mMutex);
    return 0;
}

int
NetworkInterfaceLinux::_SendArpPacket()
{
    if (AT_IPV4 != mAddressType)
        return -1;
    return SendArpingPacket(mCheckFd, 0, mIPv4Conf.getAddress());
}

int
NetworkInterfaceLinux:: _RecvArpPacket()
{
    if (AT_IPV4 != mAddressType)
        return -1;
    char mac[32] = { 0 };
    if (RecvArpingPacket(mCheckFd, mIPv4Conf.getAddress(), mac, 31) > 0) {
        if (!memcmp( mIfaceMac.c_str(), (struct ether_addr*)ether_aton(mac), 6))
            return 0;
        NETWORK_LOG_WARN("Ip Confilict: [%s]:[%s]\n", mIPv4Conf.getAddress(), mac);
        return 1;
    }
}

int
NetworkInterfaceLinux::startCheckIP()
{
    NETWORK_LOG_INFO("start check ip!\n");
    mCheckFd = InitArpingSocket(mIfaceName.c_str());
    if (mCheckFd > 0) {
        clock_gettime(CLOCK_REALTIME, &mCheckTime[1]);
        mCheckTime[0].tv_sec = 0;
    }
    return mCheckFd;
}

int
NetworkInterfaceLinux::stopCheckIP()
{
    if (mCheckFd < 0)
        return -1;

    close(mCheckFd);
    mCheckFd = -1;
    return 0;
}

int
NetworkInterfaceLinux::getStats(NetDevStats_t* stats)
{
    //"%6s: %8lu %7lu %4lu %4lu %4lu %5lu %10lu %9lu " "%8lu %7lu %4lu %4lu %4lu %5lu %7lu %10lu\n"
    FILE* f = 0;
    if ((f = fopen(NETDEV_PATH, "r"))) {
        char line[1024] = { 0 };
        while (fgets(line, 1024, f)) {
            if (!strstr(line, mIfaceName.c_str()))
                continue;
            sscanf(strchr(line, ':') + 1, "%8lu %7lu %4lu %4lu %*u %*u %*u %*u %8lu %7lu %4lu %4lu %*u %*u %*u %*u",
                &stats->rxBytes, &stats->rxPackets, &stats->rxErrors, &stats->rxDrop,
                &stats->txBytes, &stats->txPackets, &stats->txErrors, &stats->txDrop);
        }
        fclose(f);
    }
    return 0;
}

int
NetworkInterfaceLinux::preSelect(fd_set* rset, fd_set* wset, fd_set* eset, struct timeval& time)
{
    if (mCheckFd < 0 && mRfd < 0)
        return -1;

    if (rset && mRfd > 0) {
        FD_SET(mRfd, rset);
    }

    int ret = -1;
    struct timeval tmpTime;
    tmpTime.tv_sec = 60;
    if (rset && mCheckFd > 0) {
        struct timespec etime;
        clock_gettime(CLOCK_REALTIME, &etime);
        if (etime.tv_sec - mCheckTime[1].tv_sec >= mCheckTime[0].tv_sec) {
            ret = _SendArpPacket();
            if (-1 == ret) {
                NETWORK_LOG_ERR("%s _SendArpPacket failed\n", mIfaceName.c_str());
                goto Err;
            }
            mCheckTime[0].tv_sec = mIPConflictConf.getReplyTime();
            clock_gettime(CLOCK_REALTIME, &mCheckTime[1]);
            tmpTime.tv_sec = mCheckTime[0].tv_sec;
        } else {
            tmpTime.tv_sec = mCheckTime[0].tv_sec - (etime.tv_sec - mCheckTime[1].tv_sec);
        }
        if(mIPConflictConf.getReplyTime() == mCheckTime[0].tv_sec)
            FD_SET(mCheckFd, rset);
    }
    time.tv_sec = time.tv_sec < tmpTime.tv_sec ? time.tv_sec : tmpTime.tv_sec;
    return mRfd > mCheckFd ? mRfd : mCheckFd;
Err:
    close(mCheckFd);
    mCheckFd = -1;
    time.tv_sec = 60;
    return mRfd;
}

int
NetworkInterfaceLinux::postSelect(fd_set* rset, fd_set* wset, fd_set* eset)
{
    if (mCheckFd < 0 && mRfd < 0)
        return -1;

    int ret = 0;
    int state = -1;

    if (mRfd > 0 && FD_ISSET(mRfd, rset)) { //mRfd return.
        ret = read(mRfd, &state, sizeof(state));
        if (0 == ret) { //write process close
            NETWORK_LOG_WARN("read = 0\n");
            char fifoname[128] = { 0 };
            snprintf(fifoname, sizeof(fifoname), "%s/dhclient-%s.pipe", DHCP_ROOT_DIR, mIfaceName.c_str());
            close(mRfd); //close and reopen the fifo
            mRfd = open(fifoname, O_RDONLY | O_NONBLOCK, 0); //3560E not support: O_CLOEXEC
        } else if (ret > 0)
            mStateHandler->handleState(state);
        else
            NETWORK_LOG_ERR("read error: %s\n", strerror(errno));
        return 1;
    }

    struct timespec etime;
    clock_gettime(CLOCK_REALTIME, &etime);
    if (mCheckFd > 0 && FD_ISSET(mCheckFd, rset)) { //mCheckFd return.
        ret = _RecvArpPacket();
        if (1 == ret) { //IP addr conflict.
            mStateHandler->handleIPConflict(1);
            NETWORK_LOG_WARN(" %s IPConflict\n", mIfaceName.c_str());
            mCheckTime[0].tv_sec = mIPConflictConf.getConflictTime();
            clock_gettime(CLOCK_REALTIME, &mCheckTime[1]);
        } else if (-1 == ret) {
            NETWORK_LOG_ERR("%s _RecvArpPacket failed\n", mIfaceName.c_str());
            goto Err;
        }
    } else if (mCheckFd > 0 && (etime.tv_sec - mCheckTime[1].tv_sec >= mCheckTime[0].tv_sec)) { //No recved packet.
        if (mIPConflictConf.getReplyTime() == mCheckTime[0].tv_sec) { //IP addr unconflict.
            mStateHandler->handleIPConflict(0);
            NETWORK_LOG_INFO(" %s IPUnconflict\n", mIfaceName.c_str());
            mCheckTime[0].tv_sec = mIPConflictConf.getUnconflictTime();
        } else {
            mCheckTime[0].tv_sec = 0;
        }
        clock_gettime(CLOCK_REALTIME, &mCheckTime[1]);
    }
    return 0;
Err:
    close(mCheckFd);
    mCheckFd = -1;
    return -1;
}
