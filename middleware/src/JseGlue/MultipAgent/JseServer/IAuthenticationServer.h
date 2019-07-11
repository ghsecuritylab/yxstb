#ifndef IAUTHENTICATIONAGENTSERVER_H
#define IAUTHENTICATIONAGENTSERVER_H

#include "IIPCServer.h"
#include "IAuthenticationCell.h"

class IAuthenticationServer : public IIPCServer, public IAuthenticationCell {
public:
    virtual int attachServer();

    ~IAuthenticationServer();
private:
    IAuthenticationServer();
    IAuthenticationServer(const IAuthenticationServer&);
    IAuthenticationServer& operator = (const IAuthenticationServer&);

    static int ICTCGetAuthInfo(void*);
    static int ICTCSetConfig(void*);
    static int ICTCGetConfig(void*);
    static int ICTCStartUpdate(void*);
    static int ILogin(void*);
    static int IioctlRead(void*);
    static int IioctlWrite(void*);
    static int ICUGetAuthInfo(void*);
    static int ICUSetConfig(void*);
    static int ICUGetConfig(void*);

    friend IIPCServer* IcreateIAuthentication(const char*);
};

extern IIPCServer* IcreateIAuthentication(const char*);

#endif //IAUTHENTICATIONBAGENTSERVER_H

