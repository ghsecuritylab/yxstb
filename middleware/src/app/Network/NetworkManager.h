#ifndef __NetworkManager__H_
#define __NetworkManager__H_

#include <pthread.h>

#ifdef __cplusplus
#include <list>
#include <map>

class NetworkCard;
class NetworkInterface;
class NetworkErrorCode;

class NetworkManager {
public:
    NetworkManager();

    int refresh();
    int listen();

    bool addDevice(NetworkCard* device);
    NetworkCard* getDevice(const char* devname);
    bool delDevice(const char* devname);

    bool addInterface(NetworkInterface* iface);
    NetworkInterface* getInterface(const char* ifname);
    bool delInterface(const char* ifname);

    bool addErrorCode(int hycode, NetworkErrorCode* errcode);
    NetworkErrorCode* getErrorCode(int hycode);
    bool delErrorCode(int hycode);

    void setCustomErrCode(bool has);
    bool hasCustomErrCode();

    void setActiveInterface(NetworkInterface* iface);
    NetworkInterface* getActiveInterface();

    void setActiveDevice(NetworkCard* device);
    NetworkCard* getActiveDevice();

    //int lock() { return pthread_mutex_lock(&mExternMutex); }
    //int unlock() { return pthread_mutex_unlock(&mExternMutex); } 

private:
    ~NetworkManager();

    int mListenFds[2];
    int mLinkSockFd;

    NetworkInterface* mActiveInterface;
    NetworkCard* mActiveDevice;

    std::list<NetworkInterface*> mInterfaces;
    std::list<NetworkCard*> mDevices;

    bool mHasCustomErrCode;
    std::map<int, NetworkErrorCode*> mErrorCodes;

    pthread_mutex_t mMutex;
    pthread_mutex_t mExternMutex;
};

NetworkManager& networkManager();

#endif

#endif
