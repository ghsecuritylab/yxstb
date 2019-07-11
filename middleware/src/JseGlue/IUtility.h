#ifndef IUTILITY_H
#define IUTILITY_H

#include "JseGlueMacro.h"

class IUtility {
public:
    virtual int startLocalCfg() = 0;
    virtual int getEvent(char*) = 0;
    virtual int sendVendorSpecificCommand(const char*, const char*) = 0;
    virtual int getValueByName(const char*, char*) = 0;
    virtual int setValueByName(const char*, const char*) = 0;
    virtual ~IUtility() { }
};

extern IUtility* createIUtility(const char* name);

#endif //IUTILITY_H

