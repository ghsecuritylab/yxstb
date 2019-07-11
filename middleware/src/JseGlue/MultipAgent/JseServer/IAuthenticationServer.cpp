#include "IAuthenticationServer.h"

IAuthenticationServer::IAuthenticationServer()
{
    LogJseGlueDebug("new\n");
}

IAuthenticationServer::~IAuthenticationServer()
{
    LogJseGlueDebug("delete\n");
}

int 
IAuthenticationServer::attachServer()
{
    addFunction("CTCGetAuthInfo", &IAuthenticationServer::ICTCGetAuthInfo);
    addFunction("CTCSetConfig", &IAuthenticationServer::ICTCSetConfig);
    addFunction("CTCGetConfig", &IAuthenticationServer::ICTCGetConfig);
    addFunction("CTCStartUpdate", &IAuthenticationServer::ICTCStartUpdate);
    addFunction("Login", &IAuthenticationServer::ILogin);
    addFunction("ioctlRead", &IAuthenticationServer::IioctlRead);
    addFunction("ioctlWrite", &IAuthenticationServer::IioctlWrite);
    addFunction("CUGetAuthInfo", &IAuthenticationServer::ICUGetAuthInfo);
    addFunction("CUSetConfig", &IAuthenticationServer::ICUSetConfig);
    addFunction("CUGetConfig", &IAuthenticationServer::ICUGetConfig);
    return 0;
}

int 
IAuthenticationServer::ICTCGetAuthInfo(void* obj)
{
    IAuthenticationServer* self = (IAuthenticationServer*)obj;

    std::string pEncToken;
    char info[IJSEBUFFSIZE + 1] = "";
    self->popArg("pEncToken", pEncToken);
    self->freeStack();
    self->CTCGetAuthInfo(pEncToken.c_str(), info);

    self->initStack(SP_Result);
    self->pushArg("info", info);
    return 0;
}

int 
IAuthenticationServer::ICTCSetConfig(void* obj)
{
    IAuthenticationServer* self = (IAuthenticationServer*)obj;

    std::string name;
    std::string value;
    self->popArg("name", name);
    self->popArg("value", value);
    self->freeStack();
    self->CTCSetConfig(name.c_str(), value.c_str());

    self->initStack(SP_Result);
    return 0;
}

int 
IAuthenticationServer::ICTCGetConfig(void* obj)
{
    IAuthenticationServer* self = (IAuthenticationServer*)obj;

    std::string name;
    char info[IJSEBUFFSIZE + 1] = "";
    self->popArg("name", name);
    self->freeStack();
    self->CTCGetConfig(name.c_str(), info); 

    self->initStack(SP_Result);
    self->pushArg("info", info);
    return 0;
}

int 
IAuthenticationServer::ICTCStartUpdate(void* obj)
{
    IAuthenticationServer* self = (IAuthenticationServer*)obj;

    self->freeStack();
    self->CTCStartUpdate();

    self->initStack(SP_Result);
    return 0;
}

int 
IAuthenticationServer::ILogin(void* obj) 
{
    IAuthenticationServer* self = (IAuthenticationServer*)obj;

    self->freeStack();
    self->Login();

    self->initStack(SP_Result);
    return 0;
}

int 
IAuthenticationServer::IioctlRead(void* obj)
{
    IAuthenticationServer* self = (IAuthenticationServer*)obj;

    std::string name;
    char value[IJSEBUFFSIZE + 1] = "";
    self->popArg("name", name);
    self->freeStack();
    self->ioctlRead(name.c_str(), value);

    self->initStack(SP_Result);
    self->pushArg("value", value);
    return 0;
}

int 
IAuthenticationServer::IioctlWrite(void* obj)
{
    IAuthenticationServer* self = (IAuthenticationServer*)obj;

    std::string name;
    std::string value;
    self->popArg("name", name);
    self->popArg("value", value);
    self->freeStack();
    self->ioctlWrite(name.c_str(), value.c_str());

    self->initStack(SP_Result);
    return 0;
}

int 
IAuthenticationServer::ICUGetAuthInfo(void* obj)
{
    IAuthenticationServer* self = (IAuthenticationServer*)obj;

    std::string pEncToken;
    char info[IJSEBUFFSIZE + 1] = "";
    self->popArg("pEncToken", pEncToken);
    self->freeStack();
    self->CUGetAuthInfo(pEncToken.c_str(), info);

    self->initStack(SP_Result);
    self->pushArg("info", info);
    return 0;
}

int 
IAuthenticationServer::ICUSetConfig(void* obj)
{
    IAuthenticationServer* self = (IAuthenticationServer*)obj;

    std::string name;
    std::string value;
    self->popArg("name", name);
    self->popArg("value", value);
    self->freeStack();
    self->CUSetConfig(name.c_str(), value.c_str());

    self->initStack(SP_Result);
    return 0;
}

int 
IAuthenticationServer::ICUGetConfig(void* obj)
{
    IAuthenticationServer* self = (IAuthenticationServer*)obj;

    std::string name;
    char info[IJSEBUFFSIZE + 1] = "";
    self->popArg("name", name);
    self->freeStack();
    self->CUGetConfig(name.c_str(), info); 

    self->initStack(SP_Result);
    self->pushArg("info", info);
    return 0;
}

//export
IIPCServer* IcreateIAuthentication(const char* name)
{
    LogJseGlueDebug("name<%s>\n", name);
    return new IAuthenticationServer;
}
