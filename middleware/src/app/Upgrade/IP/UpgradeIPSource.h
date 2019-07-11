#ifndef _UpgradeIPSource_H_
#define _UpgradeIPSource_H_

#include "UpgradeSource.h"
#include "UpgradeManager.h"

#ifdef __cplusplus

namespace Hippo {

class UpgradeManager;

class UpgradeIPSource : public UpgradeSource {
public:
    UpgradeIPSource();
    ~UpgradeIPSource();
    
    virtual int Type() { return UpgradeManager::UMUT_IP_SOFTWARE; }

private:
    void* m_memory;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _IPUpgradeSource_H_
