#ifndef IAUTHENTICATIONCALLER_H
#define IAUTHENTICATIONCALLER_H

#include "IIPCCaller.h"
#include "IAuthentication.h"

class IAuthenticationCaller : public IIPCCaller, public IAuthentication {
public:
    virtual int CTCGetAuthInfo(const char*, char*);
    virtual int CTCSetConfig(const char*, const char*);
    virtual int CTCGetConfig(const char* name, char* value);
    virtual int CTCStartUpdate();
    virtual int Login();
    virtual int ioctlRead(const char*, char*);
    virtual int ioctlWrite(const char*, const char*);
    virtual int CUGetAuthInfo(const char*, char*);
    virtual int CUSetConfig(const char*, const char*);
    virtual int CUGetConfig(const char*, char*);

private:
    IAuthenticationCaller(const char*, IPCCaller*);
    IAuthenticationCaller(const IAuthenticationCaller&);
    IAuthenticationCaller& operator = (const IAuthenticationCaller&);

    friend class AgentHelper;
};

#endif //IAUTHENTICATIONBCALLER_H

