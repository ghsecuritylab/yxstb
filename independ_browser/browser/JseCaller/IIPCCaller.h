#ifndef IIPCCALLER_H
#define IIPCCALLER_H

#include "browser/AgentHelper.h"
#include "IPCCaller.h"

class IIPCCaller : public Hippo::IPCCaller {
protected:
    virtual int attachServer() { return 0; }
    virtual int detachServer() { return 0; }
    IIPCCaller(const char* name, IPCCaller* parent)
        : IPCCaller(name, parent)
    { }
    virtual ~IIPCCaller() { }
};

#endif //IIPCCALLER_H

