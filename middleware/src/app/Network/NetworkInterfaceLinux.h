#ifndef __NetworkInterfaceLinux__H_
#define __NetworkInterfaceLinux__H_

#include "NetworkInterface.h"
#include <sys/select.h>

#ifdef __cplusplus

class StateHandler;
class NetworkInterfaceLinux : public NetworkInterface {
public:
    NetworkInterfaceLinux(NetworkCard* d, const char* ifname = 0);
    NetworkInterfaceLinux(NetworkCard* d, int vlanId);
    ~NetworkInterfaceLinux();

    virtual int connect(int mode = 0);
    virtual int disconnect();
    virtual int preSelect(fd_set* rset, fd_set* wset, fd_set* eset, struct timeval& time);
    virtual int postSelect(fd_set* rset, fd_set* wset, fd_set* eset);

    virtual int startCheckIP();
    virtual int stopCheckIP();

    virtual int getStats(NetDevStats_t* stats);

private:
    int _SendArpPacket();
    int _RecvArpPacket();

    int mRfd;
    int mCheckFd; //ip address checking
    struct timespec mCheckTime[2]; //0:interval time，1:begin time
    StateHandler* mStateHandler;
};

#endif

#endif

