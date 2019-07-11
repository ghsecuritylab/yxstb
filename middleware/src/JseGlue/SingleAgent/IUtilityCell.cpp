#include "JseGlueAssertions.h"
#include "IUtilityCell.h"
#include "Hippo_Context.h"
#include <Hippo_HString.h>

#include <stdio.h>
#include <string.h>

using Hippo::HippoContext;
using Hippo::HString;

IUtilityCell::IUtilityCell()
{
    LogJseGlueDebug("new");
}

int 
IUtilityCell::startLocalCfg()
{
    HString fieldName("StartLocalCfg");
    HString fieldValue;

    int ret = HippoContext::getContextInstance()->ioctlWrite(fieldName, fieldValue);

    LogJseGlueDebug("ret<%d>\n", ret);
    return ret;
}

int 
IUtilityCell::getEvent(char* event)
{
    HString fieldName("getEvent");
    HString fieldValue;

    int ret = HippoContext::getContextInstance()->ioctlRead(fieldName, fieldValue);
    strncpy(event, fieldValue.c_str(), IJSEBUFFSIZE);
    event[IJSEBUFFSIZE] = '\0';

    LogJseGlueDebug("Event<%s>\n", event);
    return ret;
}

int 
IUtilityCell::sendVendorSpecificCommand(const char* name, const char* value)
{
    LogJseGlueDebug("<%s , %s>\n", name, value);

    HString fieldName = name;
    HString fieldValue = value;

    return HippoContext::getContextInstance()->ioctlWrite(fieldName, fieldValue);
}

int 
IUtilityCell::getValueByName(const char* name, char* value)
{
    HString fieldName = name;
    HString fieldValue;

    int ret = HippoContext::getContextInstance()->ioctlRead(fieldName, fieldValue);
    strncpy(value, fieldValue.c_str(), IJSEBUFFSIZE);
    value[IJSEBUFFSIZE] = '\0';

    LogJseGlueDebug("<%s , %s>\n", name, value);
    return ret;
}

int 
IUtilityCell::setValueByName(const char* name, const char* value)
{		
    LogJseGlueDebug("<%s , %s>\n", name, value);

    HString fieldName = name;
    HString fieldValue = value;

    int ret = HippoContext::getContextInstance()->ioctlWrite(fieldName, fieldValue);

    return ret;
}

//export
IUtility* createIUtility(const char* name)
{
    LogJseGlueDebug("name<%s>\n", name);
    return new IUtilityCell;
}
