#include "IiPanelCaller.h"

IiPanelCaller::IiPanelCaller(const char* name, IPCCaller* parent)
    : IIPCCaller(name, parent)
{
    LogIpcDebug("create %s\n", name);
}

int 
IiPanelCaller::ioctlRead(const char* name, char* value)
{
    int ret = -1;
    initStack(SP_Call);
    pushArg("name", name);
    call("ioctlRead");
    popArg("return", ret);
    popArg("value", value, IJSEBUFFSIZE);
    freeStack();
    return ret;
}

int 
IiPanelCaller::ioctlWrite(const char* name, const char* value)
{
    int ret = -1;
    initStack(SP_Call);
    pushArg("value", value);
    pushArg("name", name);
    call("ioctlWrite");
    popArg("return", ret);
    freeStack();
    return ret;
}

int 
IiPanelCaller::debug()
{
    return 0;
}

//export
IiPanel* createIiPanel(const char* name)
{
    LogIpcDebug("name<%s>\n", name);
    return Agent()->createIiPanel(name);
}
