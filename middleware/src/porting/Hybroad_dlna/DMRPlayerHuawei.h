#ifndef _DMRPlayerHuawei_H_
#define _DMRPlayerHuawei_H_

#include "DMRPlayer.h"

#ifdef __cplusplus

namespace Hippo {
void setIsplay_huawei(int temp);
void setIsepgfinish_huawei(int temp);
class DMRPlayerHuawei : public DMRPlayer {
public:
    DMRPlayerHuawei(int dmrType, DMRManager* dmrManager);
    ~DMRPlayerHuawei();

    virtual struct json_object *GetResultDrm();
    virtual void handleDmrMessage(); 
    //void setIsplay(int temp);
    int Isplay();
    int Isepgfinish();

private:
    int Local_flog;
    int play();
    int stop();
    int seek();
    int pause();
    int resume();
    int setMute();
    int getMute();
    int setVolume();
    int getVolume();        
                
};

} // namespace Hippo


#endif // __cplusplus

#endif 
