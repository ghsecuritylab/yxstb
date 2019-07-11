#ifndef _DMRPlayerCTC_H_
#define _DMRPlayerCTC_H_

#include <string>

#include "json/json.h"    

#ifdef __cplusplus

namespace Hippo {
    
class DMRManager;
class DMRPlayer;

class DMRPlayerCTC : public DMRPlayer {
public:
    DMRPlayerCTC(int dmrType, DMRManager* dmrManager);
    ~DMRPlayerCTC();

    virtual struct json_object *GetResultDrm();
    virtual int parseString(std::string str);
    virtual void handleDmrMessage();

private:    
    int handleDmrPlayEvent();
    int handleOrderInfoEvent();
    int handleGetProductInfoEvent(); 
    int handleGetDmrMediaInfoEvent();
    int handlePicShowEvent();        
    int handlePicReleaseEvent();
    int seek();
    int setVolume(); 
                
};

} // namespace Hippo


#endif // __cplusplus

#endif 