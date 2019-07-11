#ifndef JseHybroad_h
#define JseHybroad_h

#include "JseGroupCall.h"

#ifdef __cplusplus

class JseHybroad : public JseGroupCall {
public:
    JseHybroad();
    ~JseHybroad();
};

int JseHybroadInit();

#endif

#endif // JseHybroad_h