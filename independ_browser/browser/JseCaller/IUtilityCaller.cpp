#include "IUtilityCaller.h"

IUtilityCaller::IUtilityCaller(const char* name, IPCCaller* parent)
    : IIPCCaller(name, parent)
{
    LogIpcDebug("create %s\n", name);
}

int 
IUtilityCaller::startLocalCfg()
{
    int ret = -1;
    initStack(SP_Call);
    call("startLocalCfg");
    popArg("return", ret);
    freeStack();
    return ret;
}

int 
IUtilityCaller::getEvent(char* event)
{
    int ret = -1;
    initStack(SP_Call);
    call("getEvent");
    popArg("return", ret);
    popArg("event", event, IJSEBUFFSIZE);
    freeStack();
    return ret;
}

int 
IUtilityCaller::sendVendorSpecificCommand(const char* name, const char* value)
{
    int ret = -1;
    initStack(SP_Call);
    pushArg("value", value);
    pushArg("name", name);
    call("sendVendorSpecificCommand");
    popArg("return", ret);
    freeStack();
    return ret;
}

int
IUtilityCaller::getValueByName(const char* name, char* value)
{
    int ret = -1;
    initStack(SP_Call);
    pushArg("name", name);
    call("getValueByName");
    popArg("return", ret);
    popArg("value", value, IJSEBUFFSIZE);
    freeStack();
    return ret;
}

int 
IUtilityCaller::setValueByName(const char* name, const char* value)
{
    int ret = -1;
    initStack(SP_Call);
    pushArg("value", value);
    pushArg("name", name);
    call("setValueByName");
    popArg("return", ret);
    freeStack();
    return ret;
}

//export
IUtility* createIUtility(const char* name)
{
    LogIpcDebug("name<%s>\n", name);
    return Agent()->createIUtility(name);
}


