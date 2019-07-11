#ifndef IDEVICESERVERCELL_H
#define IDEVICESERVERCELL_H

#include "IDevice.h"

class IDeviceCell : public IDevice {
public:
    virtual void read(const char* fieldName, char* fieldValue);
    virtual void write(const char* fieldName, const char* fieldValue);

protected:
    IDeviceCell();
    IDeviceCell(const IDeviceCell&);
    IDeviceCell& operator = (const IDeviceCell&);

    friend IDevice* createIDevice(const char*);
};

#endif //IDEVICESERVER_H

