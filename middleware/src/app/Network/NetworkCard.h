#ifndef __NetworkCard_H_
#define __NetworkCard_H_

#include "NetworkAssertions.h"
#include "NetworkTypes.h"
#include <sys/select.h>

#ifdef __cplusplus
#include <string>
#include <vector>

class NetlinkStateHandler;

class NetworkCard {
public:
    NetworkCard(const char* devname);
    ~NetworkCard();

    enum Type_e {
        NT_ETHERNET,
        NT_WIRELESS,
        NT_BLUETOOTH,
        UNKNOWN
    };

    enum LinkStatus_e {
        LS_UNKNOWN,
        LS_ETHERNET_UP,
        LS_ETHERNET_DOWN,
        LS_WIRELESS_UP,
        LS_WIRELESS_DOWN,
        LS_WIRELESS_JOIN_FAIL,
        LS_WIRELESS_JOIN_SECCESS,
        LS_WIRELESS_CHECK_SIGNAL,
    };

    const char* devname() { return mDeviceName.c_str(); }

    virtual int linkUp();
    virtual int linkDown();
    virtual int linkStatus() = 0;
    virtual int linkChange(int type, char* data, int size) { return 0; }
    virtual int flagChange(int) = 0;

    virtual int setLinkSpeed(int speed) { return 0; }
    virtual int getLinkSpeed() { return 0; }

    void setType(Type_e t) { mType = t; }
    Type_e getType() { return mType; }

    bool addIfname(std::string& ifname);
    bool delIfname(std::string& ifname);
    std::vector<std::string> getIfnames();

protected:
    std::string mDeviceName;
    NetlinkStateHandler* mStateHandler;
    int mLinkState;
    int mSockFd;
    std::vector<std::string> mIfaceNames;
private:
    Type_e mType;
};

const char* NetlinkFlagStr(unsigned int fl); //Debug
const char* NetlinkStatStr(unsigned int st); //Debug
#endif

#endif
