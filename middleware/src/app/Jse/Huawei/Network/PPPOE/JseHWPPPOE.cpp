
#include "JseHWPPPOE.h"
#include "JseRoot.h"
#include "JseCall.h"
#include "JseFunctionCall.h"
#include "JseAssertions.h"

#include "SysSetting.h"
#include "SettingModuleNetwork.h"

#include "NativeHandler.h"
#include "MessageValueSystem.h"
#include "MessageTypes.h"
#include "NetworkFunctions.h"
#include "sys_basic_macro.h"

static int JseNetUserIDRead(const char *param, char *value, int len)
{
    sysSettingGetString("netuser", value, len, 0);
    return 0;
}

static int JseNetUserIDWrite(const char *param, char *value, int len)
{
    sysSettingSetString("netuser", value);
    if (NativeHandlerGetState() == 2) {//Boot 0 Config 1 Running 2
        char ifname[USER_LEN] = { 0 };
        if (network_connecttype_get(network_default_ifname(ifname, USER_LEN)) == NETTYPE_PPPOE) {
            LogJseDebug("PPPoE change Account\n");
            sendMessageToNativeHandler(MessageType_System, MV_System_ModifyPPPoEAccount, 0, 0);
        }
    }
    return 0;
}

static int JseNetPwdRead(const char *param, char *value, int len)
{
    sysSettingGetString("netAESpasswd", value, len, 0);
    return 0;
}

static int JseNetPwdWrite(const char *param, char *value, int len)
{
    if (NativeHandlerGetState() == 2) {//Boot 0 Config 1 Running 2
        char ifname[USER_LEN] = { 0 };
        if (network_connecttype_get(network_default_ifname(ifname, USER_LEN)) == NETTYPE_PPPOE) {
            LogJseDebug("PPPoE change Pwd\n");
            sendMessageToNativeHandler(MessageType_System, MV_System_ModifyPPPoEPwd, 0, 0);
        }
    }
    sysSettingSetString("netAESpasswd", value);
    return 0;
}

/*************************************************
Description: 初始化华为网络中的PPPOE配置定义的接口，由JseHWNetwork.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseHWPPPOEInit()
{
    JseCall* call;

    //C10 regist
    call = new JseFunctionCall("NetUserID", JseNetUserIDRead, JseNetUserIDWrite);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("NetPwd", JseNetPwdRead, JseNetPwdWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("netuseraccount", JseNetUserIDRead, JseNetUserIDWrite);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("netuserpassword", JseNetPwdRead, JseNetPwdWrite);
    JseRootRegist(call->name(), call);

    // zte regist
    call = new JseFunctionCall("AccessUserName", JseNetUserIDRead, NULL);
    JseRootRegist(call->name(), call);

    return 0;
}

