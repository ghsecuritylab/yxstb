#ifndef __NetworkInterfaceAndroid__H_
#define __NetworkInterfaceAndroid__H_

#include "NetworkInterface.h"
#include <sys/select.h>

#ifdef __cplusplus

class NetworkInterfaceAndroid : public NetworkInterface {
public:
    NetworkInterfaceAndroid(NetworkCard* d, const char* ifname = 0);
    NetworkInterfaceAndroid(NetworkCard* d, int vlanId);
    ~NetworkInterfaceAndroid();
    virtual int connect(int mode = 0);
    virtual int disconnect();
    virtual int preSelect(fd_set* rset, fd_set* wset, fd_set* eset, struct timeval& time);
    virtual int postSelect(fd_set* rset, fd_set* wset, fd_set* eset);

    virtual int startCheckIP();
    virtual int stopCheckIP();
};

#endif

#endif

