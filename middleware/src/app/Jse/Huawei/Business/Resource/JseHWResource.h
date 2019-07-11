#ifndef JseHWResource_h
#define JseHWResource_h

#include "JseGroupCall.h"

#ifdef __cplusplus

class JseHWResource : public JseGroupCall {
public:
    JseHWResource();
    ~JseHWResource();
};

int JseHWResourceInit();

#endif

#endif // JseHWResource_h

