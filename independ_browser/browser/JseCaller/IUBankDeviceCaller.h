#ifndef IUBANKDEVICECALLER_H
#define IUBANKDEVICECALLER_H

#include "IIPCCaller.h"
#include "IUBankDevice.h"

class IUBankDeviceCaller : public IIPCCaller, public IUBankDevice {
public:
    virtual int createUBankDeviceInstance(const char*);
    virtual int set(const char* ioStr, const char* wrStr);
    virtual int get(const char* ioStr, char* wrStr);
    virtual int ioctl(const char* cmdJson, char* jsStr);
    virtual bool deleteEvent(int eventId);
    virtual bool addEvent(int eventId);

private:
    IUBankDeviceCaller(const char* name, IPCCaller* parent);
    IUBankDeviceCaller(const IUBankDeviceCaller&);
    IUBankDeviceCaller& operator = (const IUBankDeviceCaller&);

    friend class AgentHelper;
};

#endif //IUBANKDEVICECALLER_H

