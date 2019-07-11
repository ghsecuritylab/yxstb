
#include "BrowserPlayerManagerC20.h"

#include "BrowserPlayerC20.h"
#include "BrowserPlayer.h"

namespace Hippo {

BrowserPlayerManagerC20::BrowserPlayerManagerC20()
{
}

BrowserPlayerManagerC20::~BrowserPlayerManagerC20()
{
}

Player *
BrowserPlayerManagerC20::createPlayer(int id, const char* name)
{
	BrowserPlayer::player_type_e playerInstanceType;
	if( !strcmp(name, "PIP") ){
		playerInstanceType = BrowserPlayer::PlayerType_ePIP;
	} else if( !strcmp(name, "MOSAIC")){
		playerInstanceType = BrowserPlayer::PlayerType_eMosaic;
	} else {
		playerInstanceType = BrowserPlayer::PlayerType_eMain;
	}
    return new BrowserPlayerC20(id, playerInstanceType);
}

void 
BrowserPlayerManagerC20::deletePlayer(Player *player)
{
    delete player;
}

} //end namespace Hippo
