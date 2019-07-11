
#include "JseLog.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"
#include "JseAssertions.h"
// #include "iptv_logging.h"

static int JsePrintfWrite(const char* param, char* value, int len)
{
    LogJseDebug("Webpage debug: %s\n", value);
    // ILOG() << TERMC_BLUE << value;
    return 0;
}

/*************************************************
Description: 初始化海博Log配置定义的接口，由JseMaintenance.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseLogInit()
{
    JseCall* call;

    call = new JseFunctionCall("printf", 0, JsePrintfWrite);
    JseRootRegist(call->name(), call);
    return 0;
}

