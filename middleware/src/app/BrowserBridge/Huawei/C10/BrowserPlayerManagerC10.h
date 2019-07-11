#ifndef _BrowserPlayerManagerC10_H_
#define _BrowserPlayerManagerC10_H_

#include "BrowserPlayerManager.h"

namespace Hippo {

class BrowserPlayerManagerC10 : public BrowserPlayerManager {
public:
    BrowserPlayerManagerC10();
    ~BrowserPlayerManagerC10();

    //virtual int maxPlayerNumber();
    virtual Player *createPlayer(int id, const char* name);
    virtual void deletePlayer(Player *);
};

} // namespace Hippo

#endif // _BrowserPlayerManagerC10_H_
