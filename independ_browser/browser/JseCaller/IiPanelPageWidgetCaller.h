#ifndef IIPANELPAGEWIDGETCALLER_H
#define IIPANELPAGEWIDGETCALLER_H

#include "IiPanelPageWidget.h"
#include "IIPCCaller.h"

class IiPanelPageWidgetCaller : public IIPCCaller, public IiPanelPageWidget {
public:
    virtual void close();
    virtual void focus();

private:
    IiPanelPageWidgetCaller(const char*, IPCCaller*);
    IiPanelPageWidgetCaller(const IiPanelPageWidgetCaller&);
    IiPanelPageWidgetCaller& operator = (const IiPanelPageWidgetCaller&);
    
    friend class AgentHelper;
};

#endif //IIPANELPAGEWIDGETSERVER_H

