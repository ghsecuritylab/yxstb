#ifndef _UpgradeTemplateBurner_H_
#define _UpgradeTemplateBurner_H_

#include "UpgradeBurner.h"

#ifdef __cplusplus

namespace Hippo {

class UpgradeManager;

class UpgradeTemplateBurner : public UpgradeBurner {	
public:
    UpgradeTemplateBurner(UpgradeManager*, UpgradeData*);
    ~UpgradeTemplateBurner();	
    
    virtual bool start();
    virtual bool stop();

    
};

}

#endif

#endif //_UpgradeTemplateBurner_H_






