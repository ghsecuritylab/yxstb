#ifndef _JseGroupCall_H_
#define _JseGroupCall_H_

#include "JseCall.h"

#ifdef __cplusplus

#include <map>


class JseGroupCall : public JseCall {
public:
    JseGroupCall(const char* name);
    ~JseGroupCall();

    virtual int call(const char *name, const char *param, char *value, int length, int set);
    virtual JseCall* getNode(const char* name, JseCall* fatherNode = NULL);
    int regist(const char *name, JseCall *call);

    JseCall* unregist(const char *name);

protected:
    std::map<std::string, JseCall*> m_callMap;
};

#endif // __cplusplus

#endif // _JseGroupCall_H_
