#ifndef IIPCSERVER_H
#define IIPCSERVER_H

#include "JseGlueAssertions.h"
#include "IPCServer.h"
class IIPCServer : public Hippo::IPCServer {
public:
    virtual int attachServer() { return 0;} // do any necessary registrations and hooks
    virtual int detachServer() { return 0;} // undo registrations and unhook
    virtual ~IIPCServer() { }
protected:
    IIPCServer() { }
};

#endif //IIPCSERVER_H

