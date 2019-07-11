
#include "JseHWNetDiagnoseTool.h"
#include "JseFunctionCall.h"
#include "JseRoot.h"

#include "JseAssertions.h"
#include "IPTVMiddleware.h"
#include "AppSetting.h"

#include <map>
#include <stdio.h>
#include <stdlib.h>

// using namespace Hippo;

// Keep
static int JseDiagnoseModeWrite( const char* param, char* value, int len )
{
#ifdef NEW_ANDROID_SETTING
    appSettingSetString("check_type", value);
#else
    IPTVMiddleware_SettingSetStr("check_type", value);
#endif
    return 0;
}


// keep
static int JseMulticastTestInfoWrite( const char* param, char* value, int len )
{
#ifdef NEW_ANDROID_SETTING
    appSettingSetString("mul_type", value);
#else
    IPTVMiddleware_SettingSetStr("mul_type", value);
#endif
    return 0;
}


// keep
static int JseNetworkSpeedTestURLWrite( const char* param, char* value, int len )
{
#ifdef NEW_ANDROID_SETTING
    appSettingSetString("downloadtest_url", value);
#else
    IPTVMiddleware_SettingSetStr("downloadtest_url", value);
#endif
    return 0;
}


JseHWNetDiagnoseTool::JseHWNetDiagnoseTool()
    : JseGroupCall("NetDiagnoseTool")
{
    JseCall* call;

    call = new JseFunctionCall("DiagnoseMode", NULL, JseDiagnoseModeWrite);
    regist(call->name(), call);

    call = new JseFunctionCall("TestMulticast", NULL, JseMulticastTestInfoWrite);
    regist(call->name(), call);

    call = new JseFunctionCall("NetworkSpeedTestURL", NULL, JseNetworkSpeedTestURLWrite);
    regist(call->name(), call);
}

JseHWNetDiagnoseTool::~JseHWNetDiagnoseTool()
{
}


int JseHWNetDiagnoseToolInit()
{
    JseCall* call = new JseHWNetDiagnoseTool();
    JseRootRegist(call->name(), call);

    return 0;
}

