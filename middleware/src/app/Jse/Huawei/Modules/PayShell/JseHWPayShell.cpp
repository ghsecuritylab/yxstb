
#include "JseHWPayShell.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"

#include "PayShell.h"

#ifdef HUAWEI_C10
static int JseUBankConstructionWrite(const char* param, char* value, int len)   
{
    pay_shell_input_msg(PAY_SHELL_CMD_LOAD, value);
}

static int JseUBankDestructorWrite(const char* param, char* value, int len)   
{
    pay_shell_input_msg(PAY_SHELL_CMD_UNLOAD, value);
}
#endif

/*************************************************
Description: 初始化华为PayShell模块配置定义的接口，由JseModules.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseHWPayShellInit()
{
    JseCall* call;
    
#ifdef HUAWEI_C10
    call = new JseFunctionCall("UBank_construction", 0, JseUBankConstructionWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("UBank_destructor", 0, JseUBankDestructorWrite);
    JseRootRegist(call->name(), call);
#endif
    return 0;
}

