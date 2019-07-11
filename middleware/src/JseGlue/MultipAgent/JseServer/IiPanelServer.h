#ifndef IIPANELAGENTSERVER_H
#define IIPANELAGENTSERVER_H

#include "IIPCServer.h"
#include "IiPanelCell.h"

class IiPanelServer : public IIPCServer, public IiPanelCell {
public:
    virtual int attachServer();

    ~IiPanelServer();

private:
    IiPanelServer();
    IiPanelServer(const IiPanelServer&);
    IiPanelServer& operator = (const IiPanelServer&);

    static int IioctlRead(void*);
    static int IioctlWrite(void*);
    static int Idebug(void*);

    friend IIPCServer* IcreateIiPanel(const char*);
};

extern IIPCServer* IcreateIiPanel(const char*);

#endif //IIPANELAGENTSERVER_H

