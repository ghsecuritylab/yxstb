#ifndef __STATEHANDLE__H_
#define __STATEHANDLE__H_

#include "NetworkErrorCode.h"

#ifdef __cplusplus
#include <string>

class NetworkCard;
class NetworkInterface;

class StateHandler {
public:
    StateHandler(NetworkInterface* iface);
    StateHandler(NetworkCard* device);
    virtual int handleState(int state);
    virtual int handleIPConflict(int state);
    virtual ~StateHandler() { }
protected:
    NetworkInterface* mIface;
    NetworkCard* mDevice;
};

const char*
EraseQuoteMark(std::string, std::string&);

#endif

#endif
