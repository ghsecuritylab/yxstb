#include "IiPanelServer.h"

#include <stdio.h>

IiPanelServer::IiPanelServer()
{
    LogJseGlueDebug("new\n");
}

IiPanelServer::~IiPanelServer()
{
    LogJseGlueDebug("delete\n");
}

int IiPanelServer::attachServer()
{
    addFunction("ioctlRead", &IiPanelServer::IioctlRead);
    addFunction("ioctlWrite", &IiPanelServer::IioctlWrite);
    return 0;
}

int 
IiPanelServer::IioctlRead(void* obj)
{
    IiPanelServer* self = (IiPanelServer*)obj;

    char value[IJSEBUFFSIZE + 1] = "";
    std::string name;
    self->popArg("name", name);
    self->freeStack();
    int ret = self->ioctlRead(name.c_str(), value);

    self->initStack(SP_Result);
    if (value[0] != 0)
        self->pushArg("value", value);
    else 
        self->pushArg("value", NULL, 0);
    self->pushArg("return", ret);
    return 0;
}

int 
IiPanelServer::IioctlWrite(void* obj)
{
    IiPanelServer* self = (IiPanelServer*)obj;

    std::string name;
    std::string value;
    self->popArg("name", name);
    self->popArg("value", value);
    self->freeStack();
    int ret = self->ioctlWrite(name.c_str(), value.c_str());

    self->initStack(SP_Result);
    self->pushArg("return", ret);
    return 0;
}

int 
IiPanelServer::Idebug(void* obj)
{
    return 0;
}

//export
IIPCServer* IcreateIiPanel(const char* name)
{
    LogJseGlueDebug("name<%s>\n", name);
    return new IiPanelServer;
}

