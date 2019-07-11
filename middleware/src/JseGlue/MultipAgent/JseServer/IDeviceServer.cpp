#include "IDeviceServer.h"

IDeviceServer::IDeviceServer()
{ 
    LogJseGlueDebug("new\n");
}

IDeviceServer::~IDeviceServer()
{ 
    LogJseGlueDebug("delete\n");
}

int 
IDeviceServer::attachServer()
{
    addFunction("read", &IDeviceServer::Iread);
    addFunction("write", &IDeviceServer::Iwrite);
    return 0;
}

int 
IDeviceServer::Iread(void* obj)
{
    IDeviceServer* self = (IDeviceServer*)obj;

    std::string name;
    char value[IJSEBUFFSIZE + 1];
    self->popArg("name", name);
    self->freeStack();
    self->read(name.c_str(), value);

    self->initStack(SP_Result);
    self->pushArg("value", value);
    return 0;
}

int 
IDeviceServer::Iwrite(void* obj)
{
    IDeviceServer* self = (IDeviceServer*)obj;

    std::string name;
    std::string value;
    self->popArg("name", name);
    self->popArg("value", value);
    self->freeStack();
    self->write(name.c_str(), value.c_str());

    self->initStack(SP_Result);
    return 0;
}

//export
IIPCServer* IcreateIDevice(const char* name)
{
    LogJseGlueDebug("name<%s>\n", name);
    return new IDeviceServer();
}

