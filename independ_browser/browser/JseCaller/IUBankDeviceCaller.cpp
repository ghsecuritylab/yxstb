#include "IUBankDeviceCaller.h"

IUBankDeviceCaller::IUBankDeviceCaller(const char* name, IPCCaller* parent)
    : IIPCCaller(name, parent)
{
    LogIpcDebug("create %s\n", name);
}

int 
IUBankDeviceCaller::createUBankDeviceInstance(const char* name)
{
    int ret = -1;
    initStack(SP_Call);
    pushArg("name", name);
    call("UBankDeviceInstance");
    popArg("return", ret);
    freeStack();
    return ret;
}

int
IUBankDeviceCaller::set(const char* ioStr, const char* wrStr)
{
    int ret = -1;
    initStack(SP_Call);
    pushArg("wrStr", wrStr);
    pushArg("ioStr", ioStr);
    call("set");
    popArg("return", ret);
    freeStack();
    return ret;
}

int 
IUBankDeviceCaller::get(const char* ioStr, char* wrStr)
{
    int ret = -1;
    initStack(SP_Call);
    pushArg("ioStr", ioStr);
    call("get");
    popArg("return", ret);
    popArg("wrStr", wrStr, IJSEBUFFSIZE);
    freeStack();
    return ret;
}

int 
IUBankDeviceCaller::ioctl(const char* cmdJson, char* jsStr)
{
    int ret = -1;
    initStack(SP_Call);
    pushArg("cmdJson", cmdJson);
    call("ioctl");
    popArg("return", ret);
    popArg("jsStr", jsStr, IJSEBUFFSIZE);
    freeStack();
    return ret;
}

bool 
IUBankDeviceCaller::deleteEvent(int eventId)
{
    bool ret = false;
    initStack(SP_Call);
    pushArg("eventId", eventId);
    call("deleteEvent");
    popArg("return", ret);
    freeStack();
    return ret;
}

bool 
IUBankDeviceCaller::addEvent(int eventId)
{
    bool ret = false;
    initStack(SP_Call);
    pushArg("eventId", eventId);
    call("addEvent");
    popArg("return", ret);
    freeStack();
    return ret;
}

//export
IUBankDevice* createIUBankDevice(const char* name)
{
    LogIpcDebug("name<%s>\n", name);
    return Agent()->createIUBankDevice(name);
}

