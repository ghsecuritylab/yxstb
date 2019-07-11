#ifndef _BrowserPlayerManager_H_
#define _BrowserPlayerManager_H_

#include "../IPTVPlatformSpec/Huaweibase/Hippo_PlayerHWBase.h"

#include <vector>

namespace Hippo {

class Player;

//typedef std::vector<Player *> PlayerVector;

class BrowserPlayerManager : public PlayerMgrHWBase {
public:
    BrowserPlayerManager();
    ~BrowserPlayerManager();

    virtual Player* createPlayerInstance( int& aPlayerId,const char* name, HActiveObjectClient* actObj, void* browserHandler );
    virtual Player *getPlayerInstanceById(int playerId);
    virtual int releasePlayerInstanceById(int playerId);

    virtual int maxPlayerNumber();
    virtual Player* createPlayer(int id, const char* name) = 0;
    virtual void deletePlayer(Player *) = 0;
    
    virtual int getCurrPlayerId();
//protected:
//    PlayerVector m_players;
};

} // namespace Hippo

#endif // _BrowserPlayerManager_H_
