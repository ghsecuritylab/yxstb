#ifndef _Tr069FunctionCall_H_
#define _Tr069FunctionCall_H_

#include "Tr069Call.h"

#ifdef __cplusplus

typedef int (*Tr069Function)(char* str, unsigned int val);

class Tr069FunctionCall : public Tr069Call {
public:
    Tr069FunctionCall(const char* name, Tr069Function readFunc, Tr069Function writeFunc);
    ~Tr069FunctionCall();
    
    virtual int call(const char *name, char *str, unsigned int val, int set);

protected:
    Tr069Function m_readFunc;
    Tr069Function m_writeFunc;
};

#endif // __cplusplus

#endif // _Tr069FunctionCall_H_
