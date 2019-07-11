#ifndef _VideoPlayerManager_H_
#define _VideoPlayerManager_H_

#ifdef __cplusplus

#include <vector>

namespace Hippo {
    
class VideoBrowserPlayer;    

class VideoPlayerManager {
public:
    VideoPlayerManager();
    ~VideoPlayerManager();
    
    int createVideoPlayerInstance();
    VideoBrowserPlayer* getVideoPlayerInstanceById(int playerID);
    int releaseVideoPlayerInstanceById(int playerID);  
        
private:
    std::vector<VideoBrowserPlayer *> m_videoPlayer; 
};
    
}

#endif

#endif //_VideoPlayerManager_H_