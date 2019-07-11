#ifndef JseVersion_h
#define JseVersion_h

#include "JseGroupCall.h"

#ifdef __cplusplus

class JseVersion : public JseGroupCall {
public:
    JseVersion();
    ~JseVersion();
};

int JseVersionInit();

#endif // __cplusplus

#endif // JseVersion_h
