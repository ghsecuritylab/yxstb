#include "JseGlueAssertions.h"
#include "IiPanelCell.h"
#include "Hippo_Context.h"
#include <Hippo_HString.h>

#include <stdio.h>
#include <string.h>

using Hippo::HippoContext;
using Hippo::HString;

IiPanelCell::IiPanelCell()
{
    LogJseGlueDebug("new\n");
}

IiPanelCell::~IiPanelCell()
{
    LogJseGlueDebug("delete\n");
}

int IiPanelCell::ioctlRead(const char* name, char* value)
{
    HString fieldName = name;
    HString fieldValue;

    int ret = HippoContext::getContextInstance()->ioctlRead(fieldName, fieldValue);
    strncpy(value, fieldValue.c_str(), IJSEBUFFSIZE);
    value[IJSEBUFFSIZE] = 0;

    LogJseGlueDebug("<%s , %s>\n", name, value);
    return ret;
}

int IiPanelCell::ioctlWrite(const char* name, const char* value)
{
    LogJseGlueDebug("<%s, %s>\n", name, value);

    HString fieldName = name;
    HString fieldValue = value;
    return HippoContext::getContextInstance()->ioctlWrite(fieldName, fieldValue);
}

int IiPanelCell::debug()
{
    return 0;
}

//export
IiPanel* createIiPanel(const char* name)
{
    LogJseGlueDebug("name<%s>\n", name);
    return new IiPanelCell;
}

