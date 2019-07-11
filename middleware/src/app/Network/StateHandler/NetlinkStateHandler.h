#ifndef __NETLINKSTATEHANDLE__H_
#define __NETLINKSTATEHANDLE__H_

#include "StateHandler.h"

#ifdef __cplusplus

class NetlinkStateHandler : public StateHandler {
public:
    NetlinkStateHandler(NetworkCard* device);
    ~NetlinkStateHandler();
    virtual int handleState(int state);
};

#endif

#endif
