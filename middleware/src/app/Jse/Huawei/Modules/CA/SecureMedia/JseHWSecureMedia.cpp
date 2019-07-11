
#include "JseHWSecureMedia.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"

//TODO
static int JseSMRegisterWrite(const char* param, char* value, int len)
{
    return 0;
}

//TODO
static int JseSMSyncrightsWrite(const char* param, char* value, int len)
{
    return 0;
}

//TODO
static int JseSMReqServiceWrite(const char* param, char* value, int len)
{
    return 0;
}

/*************************************************
Description: 初始化华为SecureMedia模块配置定义的接口，由JseHWCA.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseHWSecureMediaInit()
{
    JseCall* call;

    //以下全为C20注册
    call = new JseFunctionCall("hw_op_smregister", 0, JseSMRegisterWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("hw_op_smsyncrights", 0, JseSMSyncrightsWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("hw_op_smreqservice", 0, JseSMReqServiceWrite);
    JseRootRegist(call->name(), call);
    return 0;
}

