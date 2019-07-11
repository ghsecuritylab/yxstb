#ifndef JseHWHDDmangment_h
#define JseHWHDDmangment_h

#include "JseGroupCall.h"

#ifdef __cplusplus

class JseHWHDDmangment : public JseGroupCall {
public:
    JseHWHDDmangment();
    ~JseHWHDDmangment();

    int call(const char* name, const char* param, char* value, int length, int set);
};

int JseHWHDDmangmentInit();

#endif

#endif // JseHWHDDmangment_h

