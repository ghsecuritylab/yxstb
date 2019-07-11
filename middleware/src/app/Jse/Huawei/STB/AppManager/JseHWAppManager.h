#ifndef JseHWAppManager_h
#define JseHWAppManager_h

#include "JseGroupCall.h"

#ifdef __cplusplus

class JseHWAppManager : public JseGroupCall {
public:
    JseHWAppManager();
    ~JseHWAppManager();
};

int JseHWAppManagerInit();

#endif

#endif // JseHWAppManager_h
