#ifndef JseHWIMS_h
#define JseHWIMS_h

#include "JseGroupCall.h"

#ifdef __cplusplus

class JseHWIMS : public JseGroupCall {
public:
    JseHWIMS();
    ~JseHWIMS();
};

int JseHWIMSInit();

#endif // __cplusplus

#endif // JseHWIMS_h
