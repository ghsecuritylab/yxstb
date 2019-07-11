#ifndef IAUTHENTICATION_H
#define IAUTHENTICATION_H

#include "JseGlueMacro.h"

class IAuthentication {
public:
    virtual int CTCGetAuthInfo(const char*, char*) = 0;
    virtual int CTCSetConfig(const char*, const char*) = 0;
    virtual int CTCGetConfig(const char*, char*) = 0;
    virtual int CTCStartUpdate() = 0;
    virtual int Login() = 0;
    virtual int ioctlRead(const char*, char*) = 0;
    virtual int ioctlWrite(const char*, const char*) = 0;
    virtual int CUGetAuthInfo(const char*, char*) = 0;
    virtual int CUSetConfig(const char*, const char*) = 0;
    virtual int CUGetConfig(const char*, char*) = 0;
    virtual ~IAuthentication() { }
};

extern IAuthentication* createIAuthentication(const char* name);

#endif //IAUTHENTICATION_H

