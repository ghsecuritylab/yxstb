#include "IDeviceCaller.h"

IDeviceCaller::IDeviceCaller(const char* name, IPCCaller* parent)
    : IIPCCaller(name, parent)
{
    LogIpcDebug("create %s\n", name);
}

void 
IDeviceCaller::read(const char* fieldName, char* fieldValue)
{
    initStack(SP_Call);
    pushArg("name", fieldName);
    call("read");
    popArg("value", fieldValue, IJSEBUFFSIZE);
    freeStack();
    fieldValue[IJSEBUFFSIZE] = '\0';
}

void 
IDeviceCaller::write(const char* fieldName, const char* fieldValue)
{
    initStack(SP_Call);
    pushArg("value", fieldValue);
    pushArg("name", fieldName);
    call("write");
    freeStack();
}

//export
IDevice* createIDevice(const char* name)
{
    LogIpcDebug("name<%s>\n", name);
    return Agent()->createIDevice(name);
}

