#ifndef __DHCPStateHandler__H
#define __DHCPStateHandler__H

#include "StateHandler.h"

#ifdef __cplusplus

class DHCPStateHandler : public StateHandler {
public:
    DHCPStateHandler(NetworkInterface* iface);
    ~DHCPStateHandler();
    virtual int handleState(int state);
};

#endif

#endif
