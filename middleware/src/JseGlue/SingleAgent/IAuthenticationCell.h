#ifndef IAUTHENTICATIONSERVERCELL_H
#define IAUTHENTICATIONSERVERCELL_H

#include "IAuthentication.h"

class IAuthenticationCell : public IAuthentication {
public:
    virtual int CTCGetAuthInfo(const char*, char*);
    virtual int CTCSetConfig(const char*, const char*);
    virtual int CTCGetConfig(const char*, char*);
    virtual int CTCStartUpdate();
    virtual int Login();
    virtual int ioctlRead(const char*, char*);
    virtual int ioctlWrite(const char*, const char*);
    virtual int CUGetAuthInfo(const char*, char*);
    virtual int CUSetConfig(const char*, const char*);
    virtual int CUGetConfig(const char*, char*);

protected:
    IAuthenticationCell();
    IAuthenticationCell(const IAuthenticationCell&);
    IAuthenticationCell& operator = (const IAuthenticationCell&);

    friend IAuthentication* createIAuthentication(const char*);
};

#endif //IAUTHENTICATIONBSERVER_H

