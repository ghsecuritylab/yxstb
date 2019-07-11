#ifndef IUBANKDEVICESERVERCELL_H
#define IUBANKDEVICESERVERCELL_H

#include "IUBankDevice.h"
#include "Hippo_CTCUBankDevice.h"

class IUBankDeviceCell : public IUBankDevice {
public:
    virtual int createUBankDeviceInstance(const char*);
    virtual int set(const char* ioStr, const char* wrStr);
    virtual int get(const char* ioStr, char* wrStr);
    virtual int ioctl(const char* cmdJson, char* jsStr);
    virtual bool deleteEvent(int eventId);
    virtual bool addEvent(int eventId);

protected:
    IUBankDeviceCell();
    IUBankDeviceCell(const IUBankDeviceCell&);
    IUBankDeviceCell& operator = (const IUBankDeviceCell&);

    Hippo::CTCUBankDevice* m_device;

    friend IUBankDevice* createIUBankDevice(const char*);
};

#endif //IUBANKDEVICESERVER_H

