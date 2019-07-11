#include "IiPanelPageWidgetServer.h"

IiPanelPageWidgetServer::IiPanelPageWidgetServer()
{
    LogJseGlueDebug("new\n");
}

IiPanelPageWidgetServer::~IiPanelPageWidgetServer()
{
    LogJseGlueDebug("delete\n");
}

int 
IiPanelPageWidgetServer::attachServer()
{
    addFunction("close", &IiPanelPageWidgetServer::Iclose);
    addFunction("focus", &IiPanelPageWidgetServer::Ifocus);
    return 0;
}

int 
IiPanelPageWidgetServer::Iclose(void* obj)
{
    IiPanelPageWidgetServer* self = (IiPanelPageWidgetServer*)obj;

    self->freeStack();
    self->close();

    self->initStack(SP_Result);
    return 0;
}

int 
IiPanelPageWidgetServer::Ifocus(void* obj)
{
    IiPanelPageWidgetServer* self = (IiPanelPageWidgetServer*)obj;

    self->freeStack();
    self->focus();

    self->initStack(SP_Result);
    return 0;
}

//export
IIPCServer* IcreateIiPanelPageWidget(const char* name)
{
    LogJseGlueDebug("name<%s>\n", name);
    return new IiPanelPageWidgetServer();
}

