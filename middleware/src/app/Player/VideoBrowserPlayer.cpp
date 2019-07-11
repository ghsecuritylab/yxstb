
#include "VideoBrowserPlayer.h"
#include "SystemManager.h"
#include "UltraPlayerVideo.h"

namespace Hippo {
VideoBrowserPlayer::VideoBrowserPlayer(int playerID)
    : m_playerID(playerID)
    , m_isAvailable(false)
{
}

VideoBrowserPlayer::~VideoBrowserPlayer()
{
    
}

void
VideoBrowserPlayer::play(char* url)
{
    SystemManager &sysManager = systemManager();
    UltraPlayer* player = sysManager.obtainMainPlayer();
    if (player) {
        player->close(UltraPlayer::BlackScreenMode);
        sysManager.releaseMainPlayer(player);
        sysManager.detachMainPlayer(player);
    }
    
    player = sysManager.obtainPipPlayer(0); // close pip;
    if (player) {
        player->close(UltraPlayer::BlackScreenMode);        
        sysManager.releasePipPlayer(player, 0);
        sysManager.detachPipPlayer(player, 0);
    }
    
    player = sysManager.obtainPipPlayer(1); // close mosaic
    if (player) 
        sysManager.destoryPipPlayer(1);
        
    player = new UltraPlayerVideo(this, 0, 0);
    sysManager.attachMainPlayer(player);
    ((UltraPlayerVideo*)player)->playerUrlSet(url);
    player->play(0);
    
    player->unref(); 
        
}

void
VideoBrowserPlayer::pause()
{
    SystemManager &sysManager = systemManager();
    UltraPlayer* player = sysManager.obtainMainPlayer(); 
    if (player)
        player->pause();
        
    sysManager.releaseMainPlayer(player);       
}

void
VideoBrowserPlayer::resume()
{
    SystemManager &sysManager = systemManager();
    UltraPlayer* player = sysManager.obtainMainPlayer();
    if (player)
        player->resume();
        
    sysManager.releaseMainPlayer(player);         
}

void
VideoBrowserPlayer::seekTo(unsigned int playTime)
{
    SystemManager &sysManager = systemManager();
    UltraPlayer* player = sysManager.obtainMainPlayer();

}

void
VideoBrowserPlayer::fastForward(int speed)
{
    
}

void
VideoBrowserPlayer::fastRewind(int speed)
{
    
}

void
VideoBrowserPlayer::close()
{
    SystemManager &sysManager = systemManager();
    UltraPlayer* player = sysManager.obtainMainPlayer();
    if (player)
        player->close(UltraPlayer::BlackScreenMode);
    
    sysManager.releaseMainPlayer(player);
    sysManager.detachMainPlayer(player);                       
}

unsigned int
VideoBrowserPlayer::getTotalTime()
{
    unsigned int totalTime = 0;
    
    SystemManager &sysManager = systemManager();
    UltraPlayer* player = sysManager.obtainMainPlayer();
    if (player)
        totalTime = player->getTotalTime();
    
    sysManager.releaseMainPlayer(player);     
    
    return totalTime;
}

unsigned int 
VideoBrowserPlayer::getCurrentTime()
{
    unsigned int currentTime = 0;
    
    SystemManager &sysManager = systemManager();
    UltraPlayer* player = sysManager.obtainMainPlayer();
    if (player)
        currentTime = player->getCurrentTime();
        
    sysManager.releaseMainPlayer(player);    
    
    return currentTime;
}

void
VideoBrowserPlayer::refreshVideoDisplay(int x, int y, int w, int h)
{
    SystemManager &sysManager = systemManager();
    UltraPlayer* player = sysManager.obtainMainPlayer();
    if (player)
        player->setStreamVideoLocation(player->getIndex(), x, y , w, h, 0);
        
    sysManager.releaseMainPlayer(player);
}

}
