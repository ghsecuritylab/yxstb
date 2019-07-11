
#include "JseNetDiagnoseTool.h"
#include "JseFunctionCall.h"
#include "JseRoot.h"
#include "JseAssertions.h"

#include "mid/mid_tools.h"
#include "BrowserAgent.h"
#include "sys_basic_macro.h"

#include "SysSetting.h"
#include "AppSetting.h"

#include "NetworkFunctions.h"
#include "NetworkDiagnose.h"
#include "NativeHandler.h"

#include <stdio.h>
#include <stdlib.h>

static int JseEnterNetDiagnoseWrite( const char* param, char* value, int len )
{
    NetworkDiagnose* netdiag = networkDiagnose();
    if (!netdiag) 
        NETWORK_LOG_ERR("networkdiagnose error\n");
    Hippo::defNativeHandler().setState(Hippo::NativeHandler::NetworkDiagnose);
    return 0;
}

static int JseNetCheckMulticastAddRead( const char* param, char* value, int len )
{
    char tempString[1024] = {0};
    appSettingGetString("netCheckMulticastAdd", tempString, sizeof(tempString), 0);
    snprintf(value, len, "%s", tempString);
    return 0;
}

static int JseNetCheckMulticastAddWrite( const char* param, char* value, int len )
{
    appSettingSetString("netCheckMulticastAdd", value);
    settingManagerSave();
    return 0;
}

static int JseNetCheckMulticastPortRead( const char* param, char* value, int len )
{
    int tempData = 0;
    appSettingGetInt("netCheckMulticastPort", &tempData, 0);
    snprintf(value, len, "%d", tempData);
    return 0;
}

static int JseNetCheckMulticastPortWrite( const char* param, char* value, int len )
{
    appSettingSetInt("netCheckMulticastPort", atoi(value));
    settingManagerSave();
    return 0;
}

static int JseNetCheckMulticastSourceAddRead( const char* param, char* value, int len )
{
    char tempString[1024] = {0};
    appSettingGetString("netCheckMulticastSourceAdd", tempString, sizeof(tempString), 0);
    snprintf(value, len, "%s", tempString);
    return 0;
}

static int JseNetCheckMulticastSourceAddWrite( const char* param, char* value, int len )
{
    appSettingSetString("netCheckMulticastSourceAdd", value);
    settingManagerSave();
    return 0;
}

static int JseNetCheckDomainRead( const char* param, char* value, int len )
{
    char tempString[1024] = {0};
    appSettingGetString("netCheckDomain", tempString, sizeof(tempString), 0);
    snprintf(value, len, "%s", tempString);
    return 0;
}

static int JseNetCheckDomainWrite( const char* param, char* value, int len )
{
    appSettingSetString("netCheckDomain", value);
    settingManagerSave();
    return 0;
}

static int JsePingTestDataSizeRead( const char* param, char* value, int len )
{
    int tempData = 0;
    appSettingGetInt("PingTestDataSize", &tempData, 0);
    snprintf(value, len, "%d", tempData);
    return 0;
}

static int JsePingTestDataSizeWrite( const char* param, char* value, int len )
{
    appSettingSetInt("PingTestDataSize", atoi(value));
    settingManagerSave();
    return 0;
}

static int JsePingTestTimeoutRead( const char* param, char* value, int len )
{
    int tempData = 0;
    appSettingGetInt("PingTestTimeout", &tempData, 0);
    snprintf(value, len, "%d", tempData);
    return 0;
}

static int JsePingTestTimeoutWrite( const char* param, char* value, int len )
{
    appSettingSetInt("PingTestTimeout", atoi(value));
    settingManagerSave();
    return 0;
}

static int JsePingTestCountRead( const char* param, char* value, int len )
{
    int tempData = 0;
    appSettingGetInt("PingTestCount", &tempData, 0);
    snprintf(value, len, "%d", tempData);
    return 0;
}

static int JsePingTestCountWrite( const char* param, char* value, int len )
{
    appSettingSetInt("PingTestCount", atoi(value));
    settingManagerSave();
    return 0;
}

static int JsePingTestTTLRead( const char* param, char* value, int len )
{
    int tempData = 0;
    appSettingGetInt("PingTestTTL", &tempData, 0);
    snprintf(value, len, "%d", tempData);
    return 0;
}

static int JsePingTestTTLWrite( const char* param, char* value, int len )
{
    appSettingSetInt("PingTestTTL", atoi(value));
    settingManagerSave();
    return 0;
}

static int JsePingTestDomainRead( const char* param, char* value, int len )
{
    char tempString[1024] = {0};
    appSettingGetString("PingTestDomain", tempString, sizeof(tempString), 0);
    snprintf(value, len, "%s", tempString);
    return 0;
}

static int JsePingTestDomainWrite( const char* param, char* value, int len )
{
    appSettingSetString("PingTestDomain", value);
    settingManagerSave();
    return 0;
}

static int JsePingTestIpRead( const char* param, char* value, int len )
{
    char tempString[1024] = {0};
    appSettingGetString("PingTestIp", tempString, sizeof(tempString), 0);
    if (std::string(tempString).compare("127.0.0.1") == 0) {
        char devName[USER_LEN] = { 0 };
        network_default_devname(devName, USER_LEN);
        if (network_device_link_state(devName)) {
            char ifName[URL_LEN] = { 0 };
            char gateWay[URL_LEN] = { 0 };
            network_default_ifname(ifName, URL_LEN);
            network_gateway_get(ifName, gateWay, URL_LEN);
            if (std::string(gateWay).substr(0, 3).compare("0.0")) {
                snprintf(value, len, "%s", gateWay);
                return 0;
            }
        }
    }
    snprintf(value, len, "%s", tempString);
    return 0;
}

static int JsePingTestIpWrite( const char* param, char* value, int len )
{
    appSettingSetString("PingTestIp", value);
    settingManagerSave();
    return 0;
}

JseNetDiagnoseTool::JseNetDiagnoseTool()
    : JseGroupCall("setting")
{
    JseCall* call;

    call = new JseFunctionCall("enterNetDiagnose", 0, JseEnterNetDiagnoseWrite);
    regist(call->name(), call);

    call = new JseFunctionCall("netCheckMulticastAdd", JseNetCheckMulticastAddRead, JseNetCheckMulticastAddWrite);
    regist(call->name(), call);

    call = new JseFunctionCall("netCheckMulticastPort", JseNetCheckMulticastPortRead, JseNetCheckMulticastPortWrite);
    regist(call->name(), call);

    call = new JseFunctionCall("netCheckMulticastSourceAdd", JseNetCheckMulticastSourceAddRead, JseNetCheckMulticastSourceAddWrite);
    regist(call->name(), call);

    call = new JseFunctionCall("netCheckDomain", JseNetCheckDomainRead, JseNetCheckDomainWrite);
    regist(call->name(), call);

    call = new JseFunctionCall("PingTestDataSize", JsePingTestDataSizeRead, JsePingTestDataSizeWrite);
    regist(call->name(), call);

    call = new JseFunctionCall("PingTestTimeout", JsePingTestTimeoutRead, JsePingTestTimeoutWrite);
    regist(call->name(), call);

    call = new JseFunctionCall("PingTestCount", JsePingTestCountRead, JsePingTestCountWrite);
    regist(call->name(), call);

    call = new JseFunctionCall("PingTestTTL", JsePingTestTTLRead, JsePingTestTTLWrite);
    regist(call->name(), call);

    call = new JseFunctionCall("PingTestDomain", JsePingTestDomainRead, JsePingTestDomainWrite);
    regist(call->name(), call);

    call = new JseFunctionCall("PingTestIp", JsePingTestIpRead, JsePingTestIpWrite);
    regist(call->name(), call);

}

JseNetDiagnoseTool::~JseNetDiagnoseTool()
{
}

