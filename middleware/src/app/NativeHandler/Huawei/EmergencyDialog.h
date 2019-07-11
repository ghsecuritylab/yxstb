#ifndef _EmergencyDialog_H_
#define _EmergencyDialog_H_

#include "Dialog.h"
#include "Icon.h"

#ifdef __cplusplus

namespace Hippo {

class EmergencyDialog : public Dialog {
public:
    enum {
        NormalSystem,
        MiniSystem
    };
		
    EmergencyDialog(int type);
    ~EmergencyDialog();
    
    virtual bool handleMessage(Message *);
    virtual void draw();

private:
    Icon mEmergencyBackground;
    int mType;	
};

} // namespace Hippo

#endif // __cplusplus

#endif // _UpgradeDialog_H_
