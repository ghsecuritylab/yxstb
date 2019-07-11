#include "IAuthenticationCaller.h"

IAuthenticationCaller::IAuthenticationCaller(const char* name, IPCCaller* parent)
    : IIPCCaller(name, parent)
{
    LogIpcDebug("create %s\n", name);
}

int 
IAuthenticationCaller::CTCGetAuthInfo(const char* pEncToken, char* info)
{
    initStack(SP_Call);
    pushArg("pEncToken", pEncToken);
    call("CTCGetAuthInfo");
    popArg("info", info, IJSEBUFFSIZE);
    freeStack();
    return 0;
}

int 
IAuthenticationCaller::CTCSetConfig(const char* name, const char* value)
{
    initStack(SP_Call);
    pushArg("value", value);
    pushArg("name", name);
    call("CTCSetConfig");
    freeStack();
    return 0;
}

int 
IAuthenticationCaller::CTCGetConfig(const char* name, char* value)
{
    initStack(SP_Call);
    pushArg("name", name);
    call("CTCGetConfig");
    popArg("value", value, IJSEBUFFSIZE);
    freeStack();
    return 0;
}

int 
IAuthenticationCaller::CTCStartUpdate()
{
    initStack(SP_Call);
    call("CTCStartUpdate");
    freeStack();
    return 0;
}

int 
IAuthenticationCaller::Login()
{
    initStack(SP_Call);
    call("Login");
    freeStack();
    return 0;
}

int 
IAuthenticationCaller::ioctlRead(const char* name, char* value)
{
    initStack(SP_Call);
    pushArg("name", name);
    call("ioctlRead");
    popArg("value", value, IJSEBUFFSIZE);
    freeStack();
    return 0;
}

int 
IAuthenticationCaller::ioctlWrite(const char* name, const char* value)
{
    initStack(SP_Call);
    pushArg("value", value);
    pushArg("name", name);
    call("ioctlWrite");
    freeStack();
    return 0;
}

int 
IAuthenticationCaller::CUGetAuthInfo(const char* pEncToken, char* info)
{
    initStack(SP_Call);
    pushArg("pEncToken", pEncToken);
    call("CUGetAuthInfo");
    popArg("info", info, IJSEBUFFSIZE);
    freeStack();
    return 0;
}

int 
IAuthenticationCaller::CUSetConfig(const char* name, const char* value)
{
    initStack(SP_Call);
    pushArg("value", value);
    pushArg("name", name);
    call("CUSetConfig");
    freeStack();
    return 0;
}

int 
IAuthenticationCaller::CUGetConfig(const char* name, char* value)
{
    initStack(SP_Call);
    pushArg("name", name);
    call("CUGetConfig");
    popArg("value", value, IJSEBUFFSIZE);
    freeStack();
    return 0;
}

//export
IAuthentication* createIAuthentication(const char* name)
{
    LogIpcDebug("name<%s>\n", name);
    return Agent()->createIAuthentication(name);
}

