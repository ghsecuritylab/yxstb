
#include "JseHWIrdeto.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"

//TODO
static int JseCAEmmSrcipWrite(const char* param, char* value, int len)
{
    return 0;
}

//TODO
static int JseDelIrdetoSoftClientFileWrite(const char* param, char* value, int len)
{
    return 0;
}

//TODO
static int JseServiceaddressWrite(const char* param, char* value, int len)
{
    return 0;
}

//TODO
static int JseCAEmmUpdateWrite(const char* param, char* value, int len)
{
    return 0;
}

/*************************************************
Description: 初始化华为Irdeto模块配置定义的接口，由JseHWCA.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseHWIrdetoInit()
{
    JseCall* call;

    //以下全为C20注册
    call = new JseFunctionCall("CA_EMM_SRCIP", 0, JseCAEmmSrcipWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("DelIrdetoSoftClientFile", 0, JseDelIrdetoSoftClientFileWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("serviceAddress", 0, JseServiceaddressWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("caEmmUpdate", 0, JseCAEmmUpdateWrite);
    JseRootRegist(call->name(), call);
    return 0;
}

