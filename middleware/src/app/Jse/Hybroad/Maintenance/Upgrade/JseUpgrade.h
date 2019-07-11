#ifndef JseUpgrade_h
#define JseUpgrade_h

#include "JseGroupCall.h"

#ifdef __cplusplus

class JseUpgrade : public JseGroupCall {
public:
    JseUpgrade();
    ~JseUpgrade();
};

int JseUpgradeInit();

#endif

#endif // JseUpgrade_h

