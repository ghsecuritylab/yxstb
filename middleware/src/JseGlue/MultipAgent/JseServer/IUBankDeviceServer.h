#ifndef IUBANKDEVICEAGENTSERVER_H
#define IUBANKDEVICEAGENTSERVER_H

#include "IIPCServer.h"
#include "IUBankDeviceCell.h"

class IUBankDeviceServer : public IIPCServer, public IUBankDeviceCell {
public:
    virtual int attachServer();

    ~IUBankDeviceServer();
private:
    IUBankDeviceServer();
    IUBankDeviceServer(const IUBankDeviceServer&);
    IUBankDeviceServer& operator = (const IUBankDeviceServer&);

    static int IcreateUBankDeviceInstance(void*);
    static int Iset(void*);
    static int Iget(void*);
    static int Iioctl(void*);
    static int IdeleteEvent(void*);
    static int IaddEvent(void*);

    friend IIPCServer* IcreateIUBankDevice(const char*);
};

extern IIPCServer* IcreateIUBankDevice(const char*);

#endif //IUBANKDEVICEAGENTSERVER_H

