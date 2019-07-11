
#include "BrowserPlayerManager.h"

#include "BrowserPlayer.h"


namespace Hippo {
static unsigned int CurrPlayerId = -1;
BrowserPlayerManager::BrowserPlayerManager()
{
}

BrowserPlayerManager::~BrowserPlayerManager()
{
}

Player *
BrowserPlayerManager::createPlayerInstance( int& playerId,const char* name, HActiveObjectClient* actObj, void* browserHandler )
{
    PlayerVector::iterator it;
	
	BrowserPlayer::player_type_e playerInstanceType;
    if(name == NULL){
        playerInstanceType = BrowserPlayer::PlayerType_eMain;
    } else if( !strcmp(name, "PIP") ){
		playerInstanceType = BrowserPlayer::PlayerType_ePIP;
	} else if( !strcmp(name, "MOSAIC")){
		playerInstanceType = BrowserPlayer::PlayerType_eMosaic;
	} else {
		playerInstanceType = BrowserPlayer::PlayerType_eMain;
	}
    for(it = m_players.begin(); it != m_players.end(); ++it){
        if ((*it)->isAvailable()) {
            if (playerId < 0) {
                Player *player = *it;
                playerId = player->instanceId();
		  		((BrowserPlayer *)player)->SetPlayInstanceType(playerInstanceType);
                player->isAvailable(false);
                CurrPlayerId = playerId;
                return player;
            }
        }
        else { //?
            if ((*it)->instanceId() == playerId) {
                CurrPlayerId = playerId;
                return (*it);
            }
        }
    }
    static int playerCount = 0;
    Player *player;

    if (playerCount >= maxPlayerNumber()) {
        it = m_players.begin();
        player = *it;
        m_players.erase(it);
        player->clearForRecycle();
        player->isAvailable(true);
        ((BrowserPlayer *)player)->SetPlayInstanceType(playerInstanceType);	
    }
    else{
        player = createPlayer(++playerCount, name);
    }
    playerId = player->instanceId();
    player->isAvailable(false);
    m_players.push_back(player);
    CurrPlayerId = playerId;
    return player;
}

Player *
BrowserPlayerManager::getPlayerInstanceById(int playerId)
{
    PlayerVector::iterator it;

    for (it = m_players.begin(); it != m_players.end(); ++it) {
        if ((*it)->instanceId() == playerId)
            return (*it);
    }
    return 0;
}

int 
BrowserPlayerManager::releasePlayerInstanceById(int playerId)
{
    PlayerVector::iterator it;

    for (it = m_players.begin(); it != m_players.end(); ++it) {
        if ((*it)->instanceId() == playerId) {
            Player *player = *it;
            m_players.erase(it);
            player->clearForRecycle();
            player->isAvailable(true);
            m_players.push_back(player);
            break;
        }
    }
    if(playerId == CurrPlayerId){
        CurrPlayerId = -1;
    }
    return 0;
}

int 
BrowserPlayerManager::maxPlayerNumber()
{
    return 18;
}

int
BrowserPlayerManager::getCurrPlayerId()
{
    return CurrPlayerId;
}

} // namespace Hippo
