#ifndef IDEVICEAGENTSERVER_H
#define IDEVICEAGENTSERVER_H

#include "IIPCServer.h"
#include "IDeviceCell.h"

class IDeviceServer : public IIPCServer, public IDeviceCell {
public:
    virtual int attachServer();
    ~IDeviceServer();
private:
    IDeviceServer();
    IDeviceServer(const IDeviceServer&);
    IDeviceServer& operator = (const IDeviceServer&);

    static int Iread(void*);
    static int Iwrite(void*);

    friend IIPCServer* IcreateIDevice(const char*);
};

extern IIPCServer* IcreateIDevice(const char*);

#endif //IDEVICEAGENTSERVER_H

