#include "IUBankDeviceServer.h"

IUBankDeviceServer::IUBankDeviceServer()
{
    LogJseGlueDebug("new\n");
}

IUBankDeviceServer::~IUBankDeviceServer()
{
    LogJseGlueDebug("delete\n");
}

int 
IUBankDeviceServer::attachServer()
{
    addFunction("set", &IUBankDeviceServer::Iset);
    addFunction("get", &IUBankDeviceServer::Iget);
    addFunction("ioctl", &IUBankDeviceServer::Iioctl);
    addFunction("deleteEvent", &IUBankDeviceServer::IdeleteEvent);
    addFunction("addEvent", &IUBankDeviceServer::IaddEvent);
    return 0;
}

int 
IUBankDeviceServer::IcreateUBankDeviceInstance(void* obj)
{
    IUBankDeviceServer* self = (IUBankDeviceServer*)obj;

    std::string name;
    self->popArg("name", name);
    self->freeStack();
    int ret = self->createUBankDeviceInstance(name.c_str());

    self->initStack(SP_Result);
    self->pushArg("return", ret);
    return ret;
}

int 
IUBankDeviceServer::Iset(void* obj)
{
    IUBankDeviceServer* self = (IUBankDeviceServer*)obj;

    std::string ioStr;
    std::string wrStr;
    self->popArg("ioStr", ioStr);
    self->popArg("wrStr", wrStr);
    self->freeStack();
    int ret = self->set(ioStr.c_str(), wrStr.c_str());

    self->initStack(SP_Result);
    self->pushArg("return", ret);
    return ret;
}

int 
IUBankDeviceServer::Iget(void* obj)
{
    IUBankDeviceServer* self = (IUBankDeviceServer*)obj;
    
    char wrStr[IJSEBUFFSIZE + 1] = "";
    std::string ioStr;
    self->popArg("ioStr", ioStr);
    self->freeStack();
    int ret = self->get(ioStr.c_str(), wrStr);

    self->initStack(SP_Result);
    self->pushArg("wrStr", wrStr, IJSEBUFFSIZE);
    self->pushArg("return", ret);
    return ret;
}

int IUBankDeviceServer::Iioctl(void* obj)
{
    IUBankDeviceServer* self = (IUBankDeviceServer*)obj;
    
    char jsStr[IJSEBUFFSIZE + 1] = "";
    std::string ioStr;
    self->popArg("ioStr", ioStr);
    self->freeStack();
    int ret = self->ioctl(ioStr.c_str(), jsStr);

    self->initStack(SP_Result);
    self->pushArg("jsStr", jsStr, IJSEBUFFSIZE);
    self->pushArg("return", ret);
    return ret;
}

int IUBankDeviceServer::IdeleteEvent(void* obj)
{
    IUBankDeviceServer* self = (IUBankDeviceServer*)obj;

    int eventId = -1;
    self->popArg("eventId", eventId);
    self->freeStack();
    bool ret = self->deleteEvent(eventId);

    self->initStack(SP_Result);
    self->pushArg("return", ret);
    return 0;
}

int IUBankDeviceServer::IaddEvent(void* obj)
{
    IUBankDeviceServer* self = (IUBankDeviceServer*)obj;

    int eventId = -1;
    self->popArg("eventId", eventId);
    self->freeStack();
    bool ret = self->deleteEvent(eventId);

    self->initStack(SP_Result);
    self->pushArg("return", ret);
    return 0;
}

//export
IIPCServer* IcreateIUBankDevice(const char* name)
{
    LogJseGlueDebug("name<%s>\n", name);
    return new IUBankDeviceServer;
}

