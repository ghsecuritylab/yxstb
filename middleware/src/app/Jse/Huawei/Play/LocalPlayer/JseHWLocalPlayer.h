#ifndef JseHWLocalPlayer_h
#define JseHWLocalPlayer_h

#include "JseGroupCall.h"

#ifdef __cplusplus

class JseHWLocalPlayer : public JseGroupCall {
public:
    JseHWLocalPlayer();
    ~JseHWLocalPlayer();
};

int JseHWLocalPlayerInit();

#endif

#endif // JseHWLocalPlayer_h

