#include "JseGlueAssertions.h"
#include "IDeviceCell.h"
#include "Hippo_Context.h"
#include <Hippo_HString.h>

#include <stdio.h>

using Hippo::HippoContext;
using Hippo::HString;

IDeviceCell::IDeviceCell()
{
    LogJseGlueDebug("new\n");
}

void 
IDeviceCell::read(const char* name, char* value)
{
    HString fieldName = name;
    HString fieldValue = value;

    HippoContext::getContextInstance()->ioctlRead(fieldName, fieldValue);

    strncpy(value, fieldValue.c_str(), IJSEBUFFSIZE);
    value[IJSEBUFFSIZE] = '\0';

    LogJseGlueTest("read<%s , %s>\n", name, value);
}

void 
IDeviceCell::write(const char* name, const char* value)
{
    LogJseGlueTest("write<%s , %s>\n", name, value);

    HString fieldName = name;
    HString fieldValue = value;
    HippoContext::getContextInstance()->ioctlWrite(fieldName, fieldValue);
}

//export
IDevice* createIDevice(const char* name)
{
    LogJseGlueDebug("name<%s>\n", name);
    return new IDeviceCell;
}
