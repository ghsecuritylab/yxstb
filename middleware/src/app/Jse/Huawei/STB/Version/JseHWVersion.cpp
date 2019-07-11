
#include "JseHWVersion.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"
#include "JseAssertions.h"

#include "customer.h"

#include "VersionSetting.h"
#include "BrowserAgent.h"
#include "ind_mem.h"
#include "stbinfo/stbinfo.h"

#include <string.h>

static int JseSoftwareVersionRead(const char *param, char *value, int len)
{
    get_upgrade_version(value);
    return 0;
}

static int JseSoftwareHWversionRead(const char *param, char *value, int len)
{
    IND_STRCPY(value, StbInfo::STB::Version::HWVersionWithPrefix());
    return 0;
}

static int JseRead_BrowserTime(const char *param, char *value, int len)
{
    LogJseError("app jse JseRead_BrowserTime\n");
    return 0;
}

static int JseBrowserVersionRead(const char *param, char *value, int len)
{
    int tBrowserVersion = 0;
    char *tBrowserBuildTime = NULL;
    char *tBrowserBuilder = NULL;

    epgBrowserAgentGetTakinVersion(&tBrowserVersion, &tBrowserBuildTime, &tBrowserBuilder);
    snprintf(value, len, "%d", tBrowserVersion);
    return 0;
}

static int JseBrowserVersionWrite(const char *param, char *value, int len)
{
    LogJseError("app jse JseWrite_BrowserVersion can't support");
    return 0;
}

static int JseCompTimeRead(const char *param, char *value, int len)
{
    strcpy(value, StbInfo::STB::Version::BuildTime());
    return 0;
}

/*************************************************
Description: 初始化华为系统版本配置定义的接口，由JseHWSTB.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseHWVersionInit()
{
    JseCall* call;

    //C10 regist
    call= new JseFunctionCall("STBVersion", JseSoftwareVersionRead, 0);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call= new JseFunctionCall("SoftwareVersion", JseSoftwareVersionRead, 0);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call= new JseFunctionCall("SoftwareHWversion", JseSoftwareHWversionRead, 0);
    JseRootRegist(call->name(), call);

    //C20 regist
    call= new JseFunctionCall("BrowserTime", JseRead_BrowserTime, 0);
    JseRootRegist(call->name(), call);

    //C20 regist
    call= new JseFunctionCall("BrowserVersion", JseBrowserVersionRead, JseBrowserVersionWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call= new JseFunctionCall("CompTime", JseCompTimeRead, 0);
    JseRootRegist(call->name(), call);

    return 0;
}
