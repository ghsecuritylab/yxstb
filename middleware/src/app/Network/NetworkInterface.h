#ifndef __NetworkInterface__H_
#define __NetworkInterface__H_

#include "NetworkTypes.h"
#include "IPv4Setting.h"
#include "IPv6Setting.h"
#include "IPConflictSetting.h"
#include "DHCPSetting.h"
#include "PPPSetting.h"
#include "NetworkDiagnose.h"

#include <sys/select.h>
#include <pthread.h>

#ifdef __cplusplus
#include <list>

class NetworkCard;

class NetworkInterface {
public:
    NetworkInterface(NetworkCard* d, const char* ifname = 0);
    NetworkInterface(NetworkCard* d, int vlanId);
    ~NetworkInterface();
    const char* ifname() { return mIfaceName.c_str(); }

    enum ProtocolType_e {
        PT_STATIC,
        PT_DHCP,
        PT_PPPOE,
    };
    int setProtocolType(ProtocolType_e pt);
    ProtocolType_e getProtocolType() { return mProtocolType; }

    enum AddressType_e {
        AT_IPV4,
        AT_IPV6
    };
    void setAddressType(AddressType_e at) { mAddressType = at; }
    AddressType_e getAddressType() { return mAddressType; }

    NetworkCard* device() { return mNetworkCard; }

    virtual int connect(int mode = 0) = 0;
    virtual int disconnect() = 0;
    virtual int preSelect(fd_set* rset, fd_set* wset, fd_set* eset, struct timeval& time) = 0;
    virtual int postSelect(fd_set* rset, fd_set* wset, fd_set* eset) = 0;

    virtual int startCheckIP() = 0;
    virtual int stopCheckIP() = 0;

    virtual int getStats(NetDevStats_t* stats) { return -1; }

    const char* getAddress(char* addr, int len);
    const char* getNetmask(char* addr, int len);
    const char* getGateway(char* addr, int len);
    const char* getDns0(char* addr, int len);
    const char* getDns1(char* addr, int len);

    void setMac(const char* mac);
    const char* getMac();

    void setIPv4Setting(IPv4Setting& setting);
    IPv4Setting& getIPv4Setting();

    void setIPv6Setting(IPv6Setting& setting);
    IPv6Setting& getIPv6Setting();

    void setIPConflictSetting(IPConflictSetting& setting);
    IPConflictSetting& getIPConflictSetting();

    void setDHCPSetting(DHCPSetting& setting);
    DHCPSetting& getDHCPSetting();

    void setPPPSetting(PPPSetting& setting);
    PPPSetting& getPPPSetting();

    void stateActiveChanaged(bool active);
    unsigned int getConnectionUpTime();
    bool isActive() { return mActivity; }

    unsigned int getConnectionTimes();

protected:
    std::string mIfaceName; //eg: eth0,rausb0,ppp,eth0.xxxx,rausb0.xxxx
    std::string mIfaceMac;
    ProtocolType_e mProtocolType;
    AddressType_e mAddressType;
    NetworkCard* mNetworkCard;
    IPv4Setting mIPv4Conf;
    IPv6Setting mIPv6Conf;
    DHCPSetting mDHCPConf;
    PPPSetting  mPPPConf;
    IPConflictSetting mIPConflictConf;
    bool mActivity;
    unsigned int mConnectionUpTime;
    unsigned int mConnectionTimes;
    pthread_mutex_t mMutex;
};

int InitArpingSocket(const char* ifname);
int SendArpingPacket(int sfd, const char* srcip, const char* dstip);
int RecvArpingPacket(int sfd, const char* ip, char* mac, int len);
#endif

#endif

