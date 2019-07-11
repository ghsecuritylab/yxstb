#ifndef _BrowserPlayerManagerC20_H_
#define _BrowserPlayerManagerC20_H_

#include "BrowserPlayerManager.h"

namespace Hippo {

class BrowserPlayerManagerC20 : public BrowserPlayerManager {
public:
    BrowserPlayerManagerC20();
    ~BrowserPlayerManagerC20();

    //virtual int maxPlayerNumber();
    virtual Player *createPlayer(int id, const char* name);
    virtual void deletePlayer(Player *);
};

} // namespace Hippo

#endif // _BrowserPlayerManagerC20_H_
