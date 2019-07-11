#ifndef JseHWNagra_h
#define JseHWNagra_h

#include "JseGroupCall.h"

#ifdef __cplusplus

class JseHWNagra : public JseGroupCall {
public:
    JseHWNagra();
    ~JseHWNagra();
    
    virtual int call(const char *name, const char *param, char *value, int length, int set);
};

int JseHWNagraInit();

#endif

#endif // JseHWNagra_h

