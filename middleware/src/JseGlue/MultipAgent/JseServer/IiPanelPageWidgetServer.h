#ifndef _IiPANELPAGEWIDGETSERVER_H
#define _IIPANELPAGEWIDGETSERVER_H

#include "IIPCServer.h"
#include "IiPanelPageWidgetCell.h"

class IiPanelPageWidgetServer : public IIPCServer, public IiPanelPageWidgetCell {
public:
    virtual int attachServer();

    ~IiPanelPageWidgetServer();
private:
    IiPanelPageWidgetServer();
    IiPanelPageWidgetServer(const IiPanelPageWidgetServer&);
    IiPanelPageWidgetServer& operator = (const IiPanelPageWidgetServer&);

    static int Iclose(void*);
    static int Ifocus(void*);

    friend IIPCServer* IcreateIiPanelPageWidget(const char*);
};

extern IIPCServer* IcreateIiPanelPageWidget(const char*);

#endif //IDEVICEAGENTSERVER_H

