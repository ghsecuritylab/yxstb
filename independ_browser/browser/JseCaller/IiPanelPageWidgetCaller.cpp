#include "IiPanelPageWidgetCaller.h"

IiPanelPageWidgetCaller::IiPanelPageWidgetCaller(const char* name, IPCCaller* parent)
    : IIPCCaller(name, parent)
{
    LogIpcDebug("create %s\n", name);
}

void 
IiPanelPageWidgetCaller::close()
{
    initStack(SP_Call);
    call("close");
    freeStack();
}

void 
IiPanelPageWidgetCaller::focus()
{
    initStack(SP_Call);
    call("focus");
    freeStack();
}

IiPanelPageWidget* createIiPanelPageWidget(const char* name)
{
    LogIpcDebug("name<%s>\n", name);
    return Agent()->createIiPanelPageWidget(name);
}

