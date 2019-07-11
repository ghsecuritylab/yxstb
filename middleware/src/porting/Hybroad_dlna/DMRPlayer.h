#ifndef _DMRPlayer_H_
#define _DMRPlayer_H_

#include <string>
#include <map>

#include "json/json.h"    

#ifdef __cplusplus

namespace Hippo {

class DMRManager;

class DMRPlayer {
public:
    DMRPlayer(int dmrType, DMRManager* dmrManager);
    ~DMRPlayer();

    enum DLNAInfo{
        DLNA_GETVOLUME = 0,
        DLNA_GETMUTE,
        DLNA_GETPOSITION,
        DLNA_GETDURATION,
        DLNA_PLAYSTATE    
    };
    
    int Type() { return m_DmrType; }
  
    bool GetfinishStatus() { return m_isFinished; }
    void SetFinishStatus(bool status) { m_isFinished = status; }
    int GetUltraPlayerInfo(int type, unsigned int *value);
    
    virtual int setDmrEventArg(char*);
    virtual struct json_object *GetResultDrm();
    virtual int parseString(std::string str);
    virtual void handleDmrMessage();
    
protected:
    int m_DmrType;
    DMRManager *m_DmrManager;
    std::string m_RetDmrJson;
    std::map<std::string, std::string> m_playInfo;
    struct json_object *m_Object;
    struct json_object *m_Result;
    bool m_isFinished;    
                
};

} // namespace Hippo


#endif // __cplusplus

#endif 