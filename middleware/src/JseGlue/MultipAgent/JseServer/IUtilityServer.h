#ifndef IUTILITYAGENTSERVER_H
#define IUTILITYAGENTSERVER_H

#include "IIPCServer.h"
#include "IUtilityCell.h"

class IUtilityServer : public IIPCServer, public IUtilityCell {
public:
    virtual int attachServer();
    ~IUtilityServer();
private:
    IUtilityServer();
    IUtilityServer(const IUtilityServer&);
    IUtilityServer& operator = (const IUtilityServer&);

    static int IstartLocalCfg(void*);
    static int IgetEvent(void*);
    static int IsendVendorSpecificCommand(void*);
    static int IgetValueByName(void*);
    static int IsetValueByName(void*);

    friend IIPCServer* IcreateIUtility(const char*);
};

extern IIPCServer* IcreateIUtility(const char*);

#endif //IUTILITYAGENTSERVER_H

