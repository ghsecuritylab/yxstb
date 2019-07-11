
#include "SystemManager.h"

#include "UltraPlayer.h"


namespace Hippo {

SystemManager::SystemManager()
    : mainPlayer(0)
{
}

SystemManager::~SystemManager()
{
}

int
SystemManager::attachMainPlayer(UltraPlayer *player)
{
    if(mainPlayer)
        return -1;
    mainPlayer = player;
    mainPlayer->safeRef();
    return 0;
}

int
SystemManager::detachMainPlayer(UltraPlayer *player)
{
    if(mainPlayer == player) {
        mainPlayer->safeUnref();
        mainPlayer = 0;
        return 0;
    }
    return -1;
}


UltraPlayer *
SystemManager::obtainMainPlayer()
{
    if (!mainPlayer)
        return NULL;

    mainPlayer->safeRef();
    return mainPlayer;
}

int
SystemManager::releaseMainPlayer(UltraPlayer *player)
{
    if(mainPlayer == player) {
        mainPlayer->safeUnref();
        return 0;
    }
    return -1;
}

void
SystemManager::destoryMainPlayer()
{
    if(mainPlayer != NULL) {
        mainPlayer->close(UltraPlayer::BlackScreenMode);
        mainPlayer->unref();
        mainPlayer = 0;
    }
}

void
SystemManager::destoryAllPlayer()
{
    destoryMainPlayer();
    destoryPipPlayer(0);
    destoryPipPlayer(1);
    destoryFreePlayer();
    mid_stream_close(0, 1);
    mid_stream_close(1, 1);
}

#define MAXMOSAICPlAYERS 9
static int MosaicPlayerNum[MAXMOSAICPlAYERS] = {0};
int
SystemManager::attachPipPlayer(UltraPlayer *player, int InstanceFlag)
{
    std::vector<UltraPlayer*>::iterator it;

    if(InstanceFlag == 0) {
        if(mPlayerArray.empty()) {
            mPlayerArray.push_back(player);
        } else {
            mPlayerArray[0] = player;
        }
        mPlayerArray[0]->safeRef();
    } else {
        if(getMosaicAllNum() >= MAXMOSAICPlAYERS) {
            for(it = mPlayerArray.end() - 1; it > mPlayerArray.begin(); it--) {
                if (*it != NULL) {	
                    (*it)->close(UltraPlayer::BlackScreenMode);
                    (*it)->safeUnref();
                    mPlayerArray.erase(it);
                }
            }
            for (int i = 0; i < MAXMOSAICPlAYERS; i++) {
                setMosaicPlayerNum(i, 0);
            }
        }
        if(mPlayerArray.empty()) {
            mPlayerArray.push_back((UltraPlayer *)0);
        }
        mPlayerArray.push_back(player);
        setMosaicPlayerNum(getMosaicPlayerNum(), 1);
    }
    return 0;
}

UltraPlayer *
SystemManager::obtainPipPlayer(int InstanceFlag)
{
    if(mPlayerArray.empty()) {
        return NULL;
    }
    if(InstanceFlag == 0) {
        mPlayerArray[0]->safeRef();
        return mPlayerArray[0];
    } else {
        if(mPlayerArray.size() > 1) {
            return mPlayerArray[1];
        } else {
            return NULL;
        }
    }
}

UltraPlayer *
SystemManager::obtainMosaicPlayer(UltraPlayer *player)
{
    if (!player)
        return 0;
        
    std::vector<UltraPlayer*>::iterator it;
    it = mPlayerArray.begin();
    it++;
    for(; it != mPlayerArray.end(); ++it) {
        if (*it == player) {
            return *it;    
        }   
    }
    return 0;
}    

int
SystemManager::detachPipPlayer(UltraPlayer *player, int InstanceFlag)
{
    std::vector<UltraPlayer*>::iterator it;
    if(mPlayerArray.empty()) {
        return -1;
    }
    if(InstanceFlag == 0) {
        if(mPlayerArray[0] == player) {
            mPlayerArray[0]->safeUnref();
            mPlayerArray[0] = 0;
            return 0;
        }
        return -1;
    } else {
        for(it = mPlayerArray.begin(); it != mPlayerArray.end(); ++it) {
            if(*it == player) {
                setMosaicPlayerNum((*it)->getIndex() - 2, 0);
                (*it)->close(UltraPlayer::BlackScreenMode);
                (*it)->safeUnref();
                mPlayerArray.erase(it);
                break;
            }
        }
        return 0;
    }
}

int
SystemManager::releasePipPlayer(UltraPlayer *player, int InstanceFlag)
{
    if(mPlayerArray.empty()) {
        return -1;
    }
    if(InstanceFlag == 0) {
        if(mPlayerArray[0] == player) {
            mPlayerArray[0]->safeUnref();
	        mPlayerArray[0] = 0;
            return 0;
        }
        return -1;
    } else {
        return 0;
    }
}

void
SystemManager::destoryPipPlayer(int InstanceFlag)
{
    std::vector<UltraPlayer*>::iterator it;
    if(mPlayerArray.empty()) {
        return;
    }
    if(InstanceFlag == 0) {
        if (mPlayerArray[0] != NULL) {
            mPlayerArray[0]->close(UltraPlayer::BlackScreenMode);
            mPlayerArray[0]->unref();
            mPlayerArray[0] = 0;
        }
    } else {
        for(it = mPlayerArray.end() - 1; it > mPlayerArray.begin(); it--) {
            if (*it != NULL) {		
                (*it)->close(UltraPlayer::BlackScreenMode);
                (*it)->unref();
                mPlayerArray.erase(it);
            }
        }
        for (int i = 0; i < MAXMOSAICPlAYERS; i++) {
            setMosaicPlayerNum(i, 0);
        }
    }
}

int
SystemManager::getMosaicPlayerNum(void)
{
    int num = 0;

    for (int i = 0; i < MAXMOSAICPlAYERS; i++) {
        if (MosaicPlayerNum[i] == 0){
            num = i;
        }
    }
    return num;
}

int
SystemManager::getMosaicAllNum(void){
    int num = 0;
    
    for(int i = 0; i < MAXMOSAICPlAYERS; i++){
       if (MosaicPlayerNum[i] == 1)
            num += 1;
    }
    return num;
}

int
SystemManager::setMosaicPlayerNum(int num, int flag)
{   
    if (flag == 1)
       MosaicPlayerNum[num] = 1;
    else
       MosaicPlayerNum[num] = 0;
    return 0;
}

static SystemManager gSystemManager;

SystemManager &systemManager()
{
    return gSystemManager;
}

int
SystemManager::attachFreePlayer(UltraPlayer *player, int id)
{
    player->mInstanceId = id;
    player->safeRef();
    mPlayerArray.push_back(player);
    return 0;
}

int
SystemManager::detachFreePlayer(UltraPlayer *player)
{
    std::vector<UltraPlayer*>::iterator it;

    if(mPlayerArray.empty()) {
        return -1;
    }
    for(it = mPlayerArray.begin(); it != mPlayerArray.end(); ++it) {
        if(*it == player) {
            (*it)->safeUnref();
            mPlayerArray.erase(it);
            break;
        }
    }
    return 0;
}

UltraPlayer *
SystemManager::obtainFreePlayer(int id)
{
    std::vector<UltraPlayer*>::iterator it;

    if(mPlayerArray.empty()) {
        return NULL;
    }
    for(it = mPlayerArray.begin(); it != mPlayerArray.end(); ++it) {
        if((*it)->mInstanceId == id) {
            (*it)->safeRef();
            return (*it);
        }
    }
    return NULL;
}

int
SystemManager::releaseFreePlayer(UltraPlayer *player)
{
    std::vector<UltraPlayer*>::iterator it;

    if(mPlayerArray.empty()) {
        return -1;
    }
    for(it = mPlayerArray.begin(); it != mPlayerArray.end(); ++it) {
        if(*it == player) {
            (*it)->safeUnref();
            break;
        }
    }
    return 0;
}

void
SystemManager::destoryFreePlayer()
{
    std::vector<UltraPlayer*>::iterator it;

    if(mPlayerArray.empty()) {
        return;
    }
    for(it = mPlayerArray.end() - 1; it >= mPlayerArray.begin(); it--) {
	if(*it != NULL) {	
        (*it)->close(UltraPlayer::BlackScreenMode);
        (*it)->safeRef();
        mPlayerArray.erase(it);
	}
    }
}

} // namespace Hippo

