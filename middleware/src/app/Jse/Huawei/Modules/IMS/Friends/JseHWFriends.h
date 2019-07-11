#ifndef JseHWFriends_h
#define JseHWFriends_h

#include "JseGroupCall.h"

#ifdef __cplusplus

class JseHWFriends : public JseGroupCall {
public:
    JseHWFriends();
    ~JseHWFriends();

    virtual int call(const char *name, const char *param, char *value, int length, int set);
};

#endif // __cplusplus

#endif // JseHWFriends_h
