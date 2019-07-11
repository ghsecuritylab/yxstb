
#include "JseHWDHCP.h"
#include "JseRoot.h"
#include "JseCall.h"
#include "JseFunctionCall.h"

//TODO
static int JseDHCPUserIDRead(const char * param, char * value, int len)
{
    return 0;
}

//TODO
static int JseDHCPUserIDWrite(const char * param, char * value, int len)
{
    return 0;
}

//TODO
static int JseDHCPPwdRead(const char * param, char * value, int len)
{
    return 0;
}

//TODO
static int JseDHCPPwdWrite(const char * param, char * value, int len)
{
    return 0;
}

/*************************************************
Description: 初始化华为网络DHCP配置定义的接口，由JseHWNetwork.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseHWDHCPInit()
{
#ifdef HUAWEI_C10
    JseCall* call;

    call = new JseFunctionCall("DHCPUserID", JseDHCPUserIDRead, JseDHCPUserIDWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("DHCPPwd", JseDHCPPwdRead, JseDHCPPwdWrite);
    JseRootRegist(call->name(), call);
#endif
    return 0;
}

