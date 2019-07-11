#ifndef IDEVICE_H
#define IDEVICE_H

#include "JseGlueMacro.h"

class IDevice {
public:
    virtual void read(const char* fieldName, char* fieldValue) = 0;
    virtual void write(const char* fieldName, const char* fieldValue) = 0;
    virtual ~IDevice() { }
};

extern IDevice* createIDevice(const char* name);

#endif //IDEVICE_H
