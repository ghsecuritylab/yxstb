#include "NetworkAssertions.h"
#include "NetworkManager.h"
#include "NetworkCard.h"
#include "NetworkInterface.h"
#include "NetworkErrorCode.h"
#include "NetworkTypes.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <linux/if_arp.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

extern "C" char *if_indextoname(unsigned ifindex, char *ifname);

NetworkManager* gNetworkManager = 0;

NetworkManager::NetworkManager()
    : mLinkSockFd(-1)
    , mActiveInterface(0)
    , mActiveDevice(0)
    , mHasCustomErrCode(false)
{
    mListenFds[0] = -1;
    mListenFds[1] = -1;

    struct sockaddr_nl addr;
    mLinkSockFd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (mLinkSockFd > 0) {
        bzero(&addr, sizeof(addr));
        addr.nl_family = AF_NETLINK;
        addr.nl_groups = RTMGRP_LINK;
        addr.nl_pid = 0;

        bind(mLinkSockFd, (struct sockaddr*)&addr, sizeof(addr));
        fcntl(mLinkSockFd, F_SETFL, O_NONBLOCK);
    }
    pthread_mutex_init(&mMutex, 0);
    pthread_mutex_init(&mExternMutex, 0);
}

NetworkManager::~NetworkManager()
{
    if (mListenFds[0])
        close(mListenFds[0]);
    if (mListenFds[1])
        close(mListenFds[1]);
    //TODO never run here
    pthread_mutex_destroy(&mMutex);
    pthread_mutex_destroy(&mExternMutex);
}

bool
NetworkManager::addDevice(NetworkCard* device)
{
    if (!device)
        return false;
    pthread_mutex_lock(&mMutex);
    std::list<NetworkCard*>::iterator it;
    for (it = mDevices.begin(); it != mDevices.end(); ++it) {
        if (!strcmp((*it)->devname(), device->devname())) {
            pthread_mutex_unlock(&mMutex);
            return false;
        }
    }
    mDevices.push_back(device);
    pthread_mutex_unlock(&mMutex);
    return true;
}

NetworkCard*
NetworkManager::getDevice(const char* devname)
{
    if (!devname)
        return 0;
    pthread_mutex_lock(&mMutex);
    std::list<NetworkCard*>::iterator it;
    for (it = mDevices.begin(); it != mDevices.end(); ++it) {
        if (!strcmp((*it)->devname(), devname)) {
            pthread_mutex_unlock(&mMutex);
            return *it;
        }
    }
    pthread_mutex_unlock(&mMutex);
    return 0;
}

bool
NetworkManager::delDevice(const char* devname)
{
    if (!devname)
        return false;
    NETWORK_LOG_WARN("//TODO need mutex protect for avoiding death\n");
    pthread_mutex_lock(&mMutex);
    std::list<NetworkCard*>::iterator it;
    for (it = mDevices.begin(); it != mDevices.end(); ++it) {
        if (!strcmp((*it)->devname(), devname)) {
            if (*it == mActiveDevice)
                mActiveDevice = 0;
            std::list<NetworkInterface*>::iterator he = mInterfaces.begin();
            while (he != mInterfaces.end()) {
                if (*it == (*he)->device()) {
                    if (*he == mActiveInterface)
                        mActiveInterface = 0;
                    delete(*he);
                    he = mInterfaces.erase(he);
                    continue;
                }
                ++he;
            }
            delete(*it);
            mDevices.erase(it);
            break;
        }
    }
    pthread_mutex_unlock(&mMutex);
    return true;
}

bool
NetworkManager::addInterface(NetworkInterface* iface)
{
    if (!iface)
        return false;
    pthread_mutex_lock(&mMutex);
    std::list<NetworkInterface*>::iterator it;
    for (it = mInterfaces.begin(); it != mInterfaces.end(); ++it) {
        if (!strcmp((*it)->ifname(), iface->ifname())) {
            pthread_mutex_unlock(&mMutex);
            return false;
        }
    }
    mInterfaces.push_back(iface);
    pthread_mutex_unlock(&mMutex);
    return true;
}

NetworkInterface*
NetworkManager::getInterface(const char* ifname)
{
    if (!ifname)
        return 0;
    pthread_mutex_lock(&mMutex);
    std::list<NetworkInterface*>::iterator it;
    for (it = mInterfaces.begin(); it != mInterfaces.end(); ++it) {
        if (!strcmp((*it)->ifname(), ifname)) {
            pthread_mutex_unlock(&mMutex);
            return *it;
        }
    }
    pthread_mutex_unlock(&mMutex);
    return 0;
}

bool
NetworkManager::delInterface(const char* ifname)
{
    if (!ifname)
        return false;
    NETWORK_LOG_WARN("//TODO need mutex protect for avoiding death\n");
    pthread_mutex_lock(&mMutex);
    std::list<NetworkInterface*>::iterator it;
    for (it = mInterfaces.begin(); it != mInterfaces.end(); ++it) {
        if (!strcmp((*it)->ifname(), ifname)) {
            if (*it == mActiveInterface)
                mActiveInterface = 0;
            delete(*it);
            mInterfaces.erase(it);
            break;
        }
    }
    pthread_mutex_unlock(&mMutex);
    return true;
}

bool
NetworkManager::addErrorCode(int hycode, NetworkErrorCode* errcode)
{
    pthread_mutex_lock(&mMutex);
    std::map<int, NetworkErrorCode*>::iterator it = mErrorCodes.find(hycode);
    if (it != mErrorCodes.end()) {
        pthread_mutex_unlock(&mMutex);
        return false;
    }
    mErrorCodes[hycode] = errcode;
    pthread_mutex_unlock(&mMutex);
    return true;
}

NetworkErrorCode*
NetworkManager::getErrorCode(int hycode)
{
    pthread_mutex_lock(&mMutex);
    std::map<int, NetworkErrorCode*>::iterator it = mErrorCodes.find(hycode);
    if (it != mErrorCodes.end()) {
        pthread_mutex_unlock(&mMutex);
        return it->second;
    }
    pthread_mutex_unlock(&mMutex);
    return 0;
}

bool
NetworkManager::delErrorCode(int hycode)
{
    pthread_mutex_lock(&mMutex);
    std::map<int, NetworkErrorCode*>::iterator it = mErrorCodes.find(hycode);
    if (it == mErrorCodes.end()) {
        pthread_mutex_unlock(&mMutex);
        return false;
    }
    mErrorCodes.erase(hycode);
    pthread_mutex_unlock(&mMutex);
    return true;
}

void
NetworkManager::setCustomErrCode(bool has)
{
    mHasCustomErrCode = has;
}

bool
NetworkManager::hasCustomErrCode()
{
    return mHasCustomErrCode;
}

void
NetworkManager::setActiveInterface(NetworkInterface* iface)
{
    mActiveInterface = iface;
}

NetworkInterface*
NetworkManager::getActiveInterface()
{
    return mActiveInterface;
}

void
NetworkManager::setActiveDevice(NetworkCard* device)
{
    mActiveDevice = device;
}

NetworkCard*
NetworkManager::getActiveDevice()
{
    return mActiveDevice;
}

int
NetworkManager::refresh()
{
    int nouse = 1;
    return write(mListenFds[1], &nouse, sizeof(nouse));
}

int
NetworkManager::listen()
{
    fd_set rset;
    int retval = 0;
    int retnum = 0;
    int maxFd = -1;
    int retFd = -1;
    char devname[256] = { 0 };
    char buffer[8192] = { 0 };
    struct timeval minTime;
    pipe(mListenFds);
    read(mListenFds[0], &retval, sizeof(retval));
    std::list<NetworkInterface*>::iterator ifaceit;
    NetworkCard* device = 0;
    for (;;) {
        retFd = -1;
        minTime.tv_sec = 60;
        minTime.tv_usec = 0;
        maxFd = mListenFds[0];
        FD_ZERO(&rset);
        //control the NetworkManager::listen function run
        FD_SET(mListenFds[0], &rset);
        //listen netlink info
        FD_SET(mLinkSockFd, &rset);

        //select network interface listen fds
        for (ifaceit = mInterfaces.begin(); ifaceit != mInterfaces.end(); ++ifaceit) {
            retFd = (*ifaceit)->preSelect(&rset, 0, 0, minTime);
            if (retFd > maxFd)
                maxFd = retFd;
        }
        //NETWORK_LOG_INFO("Set select timeout time = %d\n", minTime);
        retnum = select(maxFd + 1, &rset, 0, 0, &minTime);
        if (-1 == retnum) {
            if (EINTR == errno)
                continue;
            return -1;
        }
        if (0 == retnum) {
            NETWORK_LOG_INFO("Listen timeout\n");
            FD_ZERO(&rset); //这边清空是为了方便在postSelect函数里处理超时返回的情况
        }

        if (FD_ISSET(mListenFds[0], &rset)) {
            read(mListenFds[0], &retval, sizeof(retval));
            if (1 == retnum)
                continue;
        }

        // Check : network device up or down
        if (FD_ISSET(mLinkSockFd, &rset)) {
            retval = recv(mLinkSockFd, &buffer, 8192, 0);
            do {
                if (retval < 0) {
                    if (errno == EINTR)
                        continue;
                    break;
                }
                for (struct nlmsghdr* nh = (struct nlmsghdr*)buffer; NLMSG_OK(nh, retval); nh = NLMSG_NEXT(nh, retval)) {
                    if (RTM_NEWLINK != nh->nlmsg_type)
                        continue;
                    struct ifinfomsg* msg = (struct ifinfomsg*)NLMSG_DATA(nh);
                    struct rtattr* rta = 0;
                    int rtalen = IFLA_PAYLOAD(nh);
                    if (ARPHRD_LOOPBACK == msg->ifi_type)
                        continue;
                    device = getDevice(if_indextoname(msg->ifi_index, devname)); // include net/if.h
                    if (!device) //|| NetworkCard::NT_WIRELESS == device->getType()) //wifi is complicated now.
                        continue;
                    device->flagChange(msg->ifi_flags); //msg->ifi_change
                    for (rta = IFLA_RTA(msg); RTA_OK(rta, rtalen); rta = RTA_NEXT(rta, rtalen))
                        device->linkChange(rta->rta_type, (char*)RTA_DATA(rta), RTA_PAYLOAD(rta));
                }
            } while (0 != (retval = recv(mLinkSockFd, &buffer, 8192, 0)));
            if (1 == retnum)
                continue;
        }

        for (ifaceit = mInterfaces.begin(); ifaceit != mInterfaces.end(); ++ifaceit){
            if ((*ifaceit)->postSelect(&rset, 0, 0) > 0 && 1 == retnum)
                break;
        }
    }
    return 0;
}

NetworkManager& networkManager()
{
    if (!gNetworkManager)
        gNetworkManager = new NetworkManager();
    return *gNetworkManager;
}

/*
 * ---------------------------------------------------------------------------------------------
 * |             |       |           |           |               |             |       |     |
 * | nlmsghdr(1) | rtmsg | rtattr(1) | rtattr(2) | rtattr(3) ... | mlmsghdr(1) | rtmsg | ... |
 * |             |       |           |           |               |             |       |     |
 * ---------------------------------------------------------------------------------------------
 */
