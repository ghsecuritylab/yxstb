#ifndef IIPANELCALLER_H
#define IIPANELCALLER_H

#include "IIPCCaller.h"
#include "IiPanel.h"

class IiPanelCaller : public IIPCCaller, public IiPanel {
public:
    virtual int ioctlRead(const char*, char*);
    virtual int ioctlWrite(const char*, const char*);
    virtual int debug();

private:
    IiPanelCaller(const char*, IPCCaller*);
    IiPanelCaller(IiPanelCaller&);
    IiPanelCaller& operator = (const IiPanelCaller&);

    friend class AgentHelper;
};

#endif //IIPANELCALLER_H

