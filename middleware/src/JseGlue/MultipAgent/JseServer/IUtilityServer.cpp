#include "IUtilityServer.h"

IUtilityServer::IUtilityServer()
{
    LogJseGlueDebug("new\n");
}

IUtilityServer::~IUtilityServer()
{
    LogJseGlueDebug("delete\n");
}

int 
IUtilityServer::attachServer()
{
    addFunction("startLocalCfg", &IUtilityServer::IstartLocalCfg);
    addFunction("getEvent", &IUtilityServer::IgetEvent);
    addFunction("sendVendorSpecificCommand", &IUtilityServer::IsendVendorSpecificCommand);
    addFunction("getValueByName", &IUtilityServer::IgetValueByName);
    addFunction("setValueByName", &IUtilityServer::IsetValueByName);
    return 0;
}

int 
IUtilityServer::IstartLocalCfg(void* obj)
{
    IUtilityServer* self = (IUtilityServer*)obj;

    self->freeStack();
    int ret = self->startLocalCfg();

    self->initStack(SP_Result);
    self->pushArg("return", ret);
    return ret;
}

int 
IUtilityServer::IgetEvent(void* obj)
{
    IUtilityServer* self = (IUtilityServer*)obj;

    self->freeStack();
    char event[IJSEBUFFSIZE + 1] = "";
    int ret = self->getEvent(event);

    self->initStack(SP_Result);
    self->pushArg("event", event, IJSEBUFFSIZE);
    self->pushArg("return", ret);
    return ret;
}

int
IUtilityServer::IsendVendorSpecificCommand(void* obj)
{
    IUtilityServer* self = (IUtilityServer*)obj;

    std::string name;
    std::string value;
    self->popArg("name", name);
    self->popArg("value", value);
    self->freeStack();
    int ret = self->sendVendorSpecificCommand(name.c_str(), value.c_str());

    self->initStack(SP_Result);
    self->pushArg("return", ret);
    return ret;
}

int
IUtilityServer::IgetValueByName(void* obj)
{
    IUtilityServer* self = (IUtilityServer*)obj;

    std::string name;
    char value[IJSEBUFFSIZE + 1] = "";
    self->popArg("name", name);
    self->freeStack();
    int ret = self->getValueByName(name.c_str(), value);

    self->initStack(SP_Result);
    if (value[0] != 0)
        self->pushArg("value", value);
    else 
        self->pushArg("value", NULL, 0);
    self->pushArg("return", ret);
    return ret;
}

int 
IUtilityServer::IsetValueByName(void* obj)
{
    IUtilityServer* self = (IUtilityServer*)obj;

    std::string name;
    std::string value;
    self->popArg("name", name);
    self->popArg("value", value);
    self->freeStack();
    int ret = self->setValueByName(name.c_str(), value.c_str());

    self->initStack(SP_Result);
    self->pushArg("return", ret);
    return ret;
}

//export
IIPCServer* IcreateIUtility(const char* name)
{
    LogJseGlueDebug("name<%s>\n", name);
    return new IUtilityServer;
}
