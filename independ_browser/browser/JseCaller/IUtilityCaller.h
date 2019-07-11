#ifndef IUTILITYCALLER_H
#define IUTILITYCALLER_H

#include "IIPCCaller.h"
#include "IUtility.h"

class IUtilityCaller : public IIPCCaller, public IUtility {
public:
    virtual int startLocalCfg(); 
    virtual int getEvent(char*);
    virtual int sendVendorSpecificCommand(const char*, const char*);
    virtual int getValueByName(const char*, char*);
    virtual int setValueByName(const char*, const char*);

private:
    IUtilityCaller(const char*, IPCCaller*);
    IUtilityCaller(const IUtilityCaller&);
    IUtilityCaller& operator = (const IUtilityCaller&);

    friend class AgentHelper;
};

#endif //IUTILITYCALLER_H

