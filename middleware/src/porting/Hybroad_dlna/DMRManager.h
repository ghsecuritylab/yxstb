#ifndef _DMRManager_H_
#define _DMRManager_H_

#include "Message.h"
#include "MessageTypes.h"
#include "MessageHandler.h"

#include <vector>
#include <string>

#ifdef __cplusplus

namespace Hippo {
class DMRPlayer;

class DMRManager : public MessageHandler{
public:
    DMRManager();
    ~DMRManager();
    
    enum PLAYSTATE {
        PlayState,
        StopState,
        PauseState,
        TickModeState 
           
    };
    
    bool addDmrPlayer(DMRPlayer *drmPlayer);
    bool removeDmrPlayer(DMRPlayer *drmPlayer);
    int SetEPGResult(std::string result, int type);
    int SetPlayUrl(std::string playUrl) { m_currentPlayUrl = playUrl; }
    std::string& GetPlayUrl() { return m_currentPlayUrl; }    
    int SetPlayState(int state) { m_currentPlayState = state; }
    int GetPlayState() { return m_currentPlayState; }
    int SetOrderUrl(std::string orderUrl) { m_orderUrl = orderUrl; }
    int SetProductUrl(std::string productUrl) { m_productUrl = productUrl;}
    std::string& GetProductUrl() { return m_productUrl; }     
    void sendDmrMessage(int what, unsigned int delayMillis);
    void removeDmrMessage(int what);       
    void DMRStart();
    int DMRJsResult();
    
    virtual void handleMessage(Message *msg);
private:
    std::vector<DMRPlayer*> m_playerArray;
    std::string m_currentPlayUrl;
    std::string m_orderUrl;
    std::string m_productUrl;
    int m_currentPlayState;   
    
};

}



#endif // __cplusplus

#endif 