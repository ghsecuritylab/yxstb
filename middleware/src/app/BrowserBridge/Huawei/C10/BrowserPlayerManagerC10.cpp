
#include "BrowserPlayerManagerC10.h"

#include "BrowserPlayerC10.h"
#include "BrowserPlayerCTC.h"
#include "BrowserPlayer.h"

namespace Hippo {

BrowserPlayerManagerC10::BrowserPlayerManagerC10()
{
}

BrowserPlayerManagerC10::~BrowserPlayerManagerC10()
{
}

Player *
BrowserPlayerManagerC10::createPlayer(int id, const char* name)
{
#if defined(SHANGHAI_HD)||defined(SHANGHAI_SD)
    return new BrowserPlayerCTC(id, BrowserPlayer::PlayerType_eUnknown);
#else
	return new BrowserPlayerC10(id, BrowserPlayer::PlayerType_eMain);
#endif
}

void 
BrowserPlayerManagerC10::deletePlayer(Player *player)
{
    delete player;
}

} //end namespace Hippo
