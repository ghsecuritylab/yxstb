
#include "Tr069PlayInfo.h"
#include "TR069Assertions.h"
#include "SystemManager.h"
#include "UltraPlayer.h"

#include "Hippo_HString.h"
#include <string>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


namespace Hippo {

static int GetCurrentPlayUrlFormMainPlayer(std::string& playUrl)
{
    UltraPlayer *player = systemManager().obtainMainPlayer();
    int ret = 0;

    if(player) {
        player->GetCurrentPlayUrl(playUrl);
    }
    if(strlen(playUrl.c_str()) == 0)
        ret = -1;
    systemManager().releaseMainPlayer(player);
    return ret;
}

static int CurrentPlayStateFormMainPlayer(HString& playState)
{
    UltraPlayer *player = systemManager().obtainMainPlayer();
    int ret = 0;

    if(player) {
        player->GetPlayBackMode(playState);
        ret = 0;
    } else {
        ret = -1;
    }
    systemManager().releaseMainPlayer(player);
    return ret;
}

} // namespace Hippo

int Tr069GetCurrentPlayURL(char *url, int pUrlLen)
{
    std::string playUrl = "";

    if(url) {
        if(0 == Hippo::GetCurrentPlayUrlFormMainPlayer(playUrl)) {
            strncpy(url, playUrl.c_str(), pUrlLen - 1);
            return 0;
        }
    }
    return -1;
}

int Tr069GetCurrentPlayState()
{
    Hippo::HString playState = "";

    if(0 == Hippo::CurrentPlayStateFormMainPlayer(playState)) {
        if ((playState.find("Stop") != std::string::npos)
			 || (playState.find("Pause") != std::string::npos))		//WZW modified to fix pc-lint warning 568
            return 2;

        if ((playState.find("Normal Play") != std::string::npos)
			 || (playState.find("Trickmode") != std::string::npos))	//WZW modified to fix pc-lint warning 568
            return 1;
    }
    return 2;
}

