#ifndef JseNetwork_h
#define JseNetwork_h

#include "JseGroupCall.h"

#ifdef __cplusplus

class JseNetwork : public JseGroupCall {
public:
    JseNetwork();
    ~JseNetwork();
};

int JseNetworkInit();

#endif // __cplusplus

#endif // JseNetwork_h
