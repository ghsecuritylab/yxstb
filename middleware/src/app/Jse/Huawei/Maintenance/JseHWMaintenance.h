#ifndef JseHWMaintenance_h
#define JseHWMaintenance_h

#include "JseGroupCall.h"

#ifdef __cplusplus

class JseHWDebug : public JseGroupCall {
public:
    JseHWDebug();
    ~JseHWDebug();
};

int JseHWMaintenanceInit();

#endif

#endif // JseHWMaintenance_h

