#ifndef _Tr069Call_H_
#define _Tr069Call_H_

#ifdef __cplusplus

#include <string>


class Tr069Call{
public:
    Tr069Call(const char* name);
    ~Tr069Call();

    const char* name() { return m_name.c_str(); }
    virtual int call(const char *name, char *str, unsigned int val, int set) = 0;

protected:
    std::string m_name;
};

#endif // __cplusplus

#endif // _Tr069Call_H_
