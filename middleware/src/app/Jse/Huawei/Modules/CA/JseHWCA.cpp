
#include "JseHWCA.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"

#if defined(INCLUDE_VMCA)
#include "Verimatrix/JseHWVerimatrix.h"
#endif

#include "ICAS/JseHWICAS.h"
#include "Irdeto/JseHWIrdeto.h"
#include "Nagra/JseHWNagra.h"
#include "SecureMedia/JseHWSecureMedia.h"

//TODO
static int JseCaidRead(const char* param, char* value, int len)
{
    return 0;
}

//TODO
static int JseCaidWrite(const char* param, char* value, int len)
{
    return 0;
}

/*************************************************
Description: 初始化华为CA模块配置定义的接口，由JseModules.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseHWCAInit()
{
    JseCall* call;

    //C20 regist
    call = new JseFunctionCall("caid", JseCaidRead, JseCaidWrite);
    JseRootRegist(call->name(), call);

#if defined(INCLUDE_VMCA)
    //C20 regist
    call = new JseHWVerimatrix();
    JseRootRegist(call->name(), call);
#endif

    //以下四个模块注册的都没有实现。
    JseHWICASInit();
    JseHWIrdetoInit();
    JseHWNagraInit();
    JseHWSecureMediaInit();
    return 0;
}

