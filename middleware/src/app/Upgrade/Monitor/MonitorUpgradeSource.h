#ifndef _MonitorUpgradeSource_H_
#define _MonitorUpgradeSource_H_

#include "UpgradeSource.h"
#include "UpgradeManager.h"

#ifdef __cplusplus

namespace Hippo {

class MonitorUpgradeSource : public UpgradeSource {
public:
    MonitorUpgradeSource();
    ~MonitorUpgradeSource();
    
    virtual int Type() { return UpgradeManager::UMUT_MONITOR_SOFTWARE; }
    
private:
    void* m_memory;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _MonitorUpgradeSource_H_
