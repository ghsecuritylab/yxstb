#ifndef _Tr069GroupCall_H_
#define _Tr069GroupCall_H_

#include "Tr069Call.h"

#ifdef __cplusplus

#include <map>

class Tr069GroupCall : public Tr069Call {
public:
    Tr069GroupCall(const char* name);
    ~Tr069GroupCall();

    virtual int call(const char *name, char *str, unsigned int val, int set);

    int regist(const char *name, Tr069Call *call);
    Tr069Call* unregist(const char *name);

protected:
    std::map<std::string, Tr069Call*> m_callMap;
};

#endif // __cplusplus

#endif // _Tr069GroupCall_H_
