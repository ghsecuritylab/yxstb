#ifndef _UpgradeUdiskSource_H_
#define _UpgradeUdiskSource_H_

#include "UpgradeSource.h"

#include "UpgradeManager.h"

#ifdef __cplusplus

namespace Hippo {

class UpgraseSource;

class UpgradeUdiskSource : public UpgradeSource {
public:
    UpgradeUdiskSource();
    ~UpgradeUdiskSource();
    
    virtual int Type() { return  UpgradeManager::UMUT_UDISK_SOFTWARE; }

private:
    void* m_memory;           
    
};    
        
} // namespace Hippo

#endif

#endif
