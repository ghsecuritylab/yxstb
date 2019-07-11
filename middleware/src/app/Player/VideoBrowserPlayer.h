#ifndef _VideoBrowserPlayer_H_
#define _VideoBrowserPlayer_H_

#include "RefCnt.h"
#include "UltraPlayerClient.h"
#include <string.h>

#ifdef __cplusplus

namespace Hippo {
    
class VideoBrowserPlayer : public UltraPlayerClient, public RefCnt {
public:
    VideoBrowserPlayer(int playerID);
    ~VideoBrowserPlayer();
    
    virtual void play(char* url);
    virtual void pause();
    virtual void resume();
    virtual void seekTo(unsigned int playerTime);
    virtual void fastForward(int speed);
    virtual void fastRewind(int speed);
    virtual void close();
    virtual unsigned int getTotalTime();
    virtual unsigned int getCurrentTime();
    virtual void refreshVideoDisplay(int x, int y, int w, int h);
    
    virtual Type type() { return Browser; }
    virtual int instanceId() { return m_isAvailable; }
    virtual void onDestroy() {}
    
    void isAvailable(bool it) { m_isAvailable = it; }
    bool isAvailable() { return m_isAvailable; }
    int playerInstanceID() { return m_playerID; }
    void playerInstanceIDSet(int playerID) { m_playerID = playerID; }

private:
    int m_playerID;
    bool m_isAvailable;
    
};

}
#endif //__cplusplus

#endif //_VideoBrowserPlayer_H_
