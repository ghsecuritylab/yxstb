#ifndef __PPPOEStateHandler__H
#define __PPPOEStateHandler__H

#include "StateHandler.h"

#ifdef __cplusplus

class PPPOEStateHandler : public StateHandler {
public:
    PPPOEStateHandler(NetworkInterface* iface);
    ~PPPOEStateHandler();
    virtual int handleState(int state);
};

#endif

#endif
