#ifndef __WiredNetworkCard__H_
#define __WiredNetworkCard__H_

#include "NetworkCard.h"

#ifdef __cplusplus

class WiredNetworkCard : public NetworkCard {
public:
    WiredNetworkCard(const char* devname = "eth0");
    ~WiredNetworkCard();
    virtual int linkStatus();
    virtual int flagChange(int flag);
    virtual int setLinkSpeed(int speed);
    virtual int getLinkSpeed();
    int setAutoNego();
};

#endif

#endif
