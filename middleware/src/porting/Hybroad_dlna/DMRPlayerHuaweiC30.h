#ifndef _DMRPlayerHuaweiC30_H_
#define _DMRPlayerHuaweiC30_H_

#include "DMRPlayer.h"

#ifdef __cplusplus

namespace Hippo {
void setIsplay(int temp);
void setIsepgfinish(int temp);

class DMRPlayerHuaweiC30 : public DMRPlayer {
public:
    DMRPlayerHuaweiC30(int dmrType, DMRManager* dmrManager);
    ~DMRPlayerHuaweiC30();

    virtual struct json_object *GetResultDrm();
    virtual void handleDmrMessage(); 
    //void setIsplay(int temp);
    int Isplay();
    int Isepgfinish();
private:
    //int isplay;
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
