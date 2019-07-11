#ifndef IUTILITYSERVERCELL_H
#define IUTILITYSERVERCELL_H

#include "IUtility.h"

class IUtilityCell : public IUtility {
public:
    virtual int startLocalCfg();
    virtual int getEvent(char* event);
    virtual int sendVendorSpecificCommand(const char*, const char*);
    virtual int getValueByName(const char*, char*);
    virtual int setValueByName(const char*, const char*);
protected:
    IUtilityCell();
    IUtilityCell(const IUtilityCell&);
    IUtilityCell& operator = (const IUtilityCell&);

    friend IUtility* createIUtility(const char*);
};

#endif //IUTILITYSERVER_H

