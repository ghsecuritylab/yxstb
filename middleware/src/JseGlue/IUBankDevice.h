#ifndef IUBANKDEVICE_H
#define IUBANKDEVICE_H

#include "JseGlueMacro.h"

class IUBankDevice {
public:
    virtual int createUBankDeviceInstance(const char*) = 0;
    virtual int set(const char* ioStr, const char* wrStr) = 0;
    virtual int get(const char* ioStr, char* wrStr) = 0;
    virtual int ioctl(const char* cmdJson, char* jsStr) = 0;
    virtual bool deleteEvent(int eventId) = 0;
    virtual bool addEvent(int eventId) = 0;
    virtual ~IUBankDevice() { }
};

extern IUBankDevice* createIUBankDevice(const char* name);

#endif //IUBANKDEVICE_H

