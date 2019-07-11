#ifndef JseHWTr069_h
#define JseHWTr069_h

#include "JseGroupCall.h"

#ifdef __cplusplus

class JseHWTr069 : public JseGroupCall {
public:
    JseHWTr069();
    ~JseHWTr069();

    int call(const char* name, const char* param, char* value, int length, int set);
};


int JseHWTr069Init();
int JseManagementDomainSet(const char *buf);

#endif

#endif // JseHWTr069_h

