#ifndef IDEVICECALLER_H
#define IDEVICECALLER_H

#include "IIPCCaller.h"
#include "IDevice.h"

class IDeviceCaller : public IIPCCaller, public IDevice {
public:
    virtual void read(const char* fieldName, char* fieldValue);
    virtual void write(const char* fieldName, const char* fieldValue);

private:
    IDeviceCaller(const char*, IPCCaller*);
    IDeviceCaller(const IDeviceCaller&);
    IDeviceCaller& operator = (const IDeviceCaller&);

    friend class AgentHelper;
};

#endif //VDEVICECALLER_H

