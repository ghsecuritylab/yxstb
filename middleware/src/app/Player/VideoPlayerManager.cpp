
#include "VideoPlayerManager.h"
#include "VideoBrowserPlayer.h"

namespace Hippo {

#define MAXPLAYERCOUNT 3

VideoPlayerManager::VideoPlayerManager()
{
    
}

VideoPlayerManager::~VideoPlayerManager()
{
    
}

int
VideoPlayerManager::createVideoPlayerInstance()
{
    std::vector<VideoBrowserPlayer *>::iterator it;
    VideoBrowserPlayer* player;
        
    int maxPlayerID = 0;
    int playerCount = 0;
    int retPlayerID = -1;
    
    for(it = m_videoPlayer.begin(); it != m_videoPlayer.end(); it++) {
        if (!(*it)->isAvailable())
            return (*it)->playerInstanceID();
        if (maxPlayerID < (*it)->playerInstanceID())
            maxPlayerID = (*it)->playerInstanceID();
        playerCount++;
    }
    if (playerCount < MAXPLAYERCOUNT || playerCount == 0) {
        retPlayerID = maxPlayerID + 1;
        player = new VideoBrowserPlayer(maxPlayerID + 1);
    } else {
        it = m_videoPlayer.begin();
        player = *it;
        m_videoPlayer.erase(it);
        player->isAvailable(true);
        retPlayerID = player->playerInstanceID();
    }
    m_videoPlayer.push_back(player); 
    
    return retPlayerID;   
}

VideoBrowserPlayer*
VideoPlayerManager::getVideoPlayerInstanceById(int playerID)
{
    std::vector<VideoBrowserPlayer *>::iterator it;
    
    for(it = m_videoPlayer.begin(); it != m_videoPlayer.end(); it++) {
        VideoBrowserPlayer* player = *it;
        if (player->playerInstanceID() == playerID)
            return player;
    }
    
    return 0;
}

int
VideoPlayerManager::releaseVideoPlayerInstanceById(int playerID)
{
    std::vector<VideoBrowserPlayer *>::iterator it;
    
    for(it = m_videoPlayer.begin(); it != m_videoPlayer.end(); it++) {
        VideoBrowserPlayer* player = *it;
        if (player->playerInstanceID() == playerID) {
            m_videoPlayer.erase(it);
            player->safeUnref();
        }
    }
    return 0;    
}

};