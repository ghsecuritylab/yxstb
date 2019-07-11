#ifndef _JseFunctionCall_H_
#define _JseFunctionCall_H_

#include "JseCall.h"

#ifdef __cplusplus


typedef int (*JseFunction)(const char* param, char* value, int result);

class JseFunctionCall : public JseCall {
public:
    JseFunctionCall(const char* name, JseFunction readFunc, JseFunction writeFunc);
    ~JseFunctionCall();
    
    virtual int call(const char *name, const char *param, char *value, int length, int set);
    virtual JseCall* getNode(const char* name, JseCall* fatherNode = NULL);
protected:
    JseFunction m_readFunc;
    JseFunction m_writeFunc;
};

#endif // __cplusplus

#endif // _JseFunctionCall_H_
