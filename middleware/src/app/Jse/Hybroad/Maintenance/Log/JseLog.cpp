
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
Description: ��ʼ������Log���ö���Ľӿڣ���JseMaintenance.cpp����
Input: ��
Return: ��
 *************************************************/
int JseLogInit()
{
    JseCall* call;

    call = new JseFunctionCall("printf", 0, JsePrintfWrite);
    JseRootRegist(call->name(), call);
    return 0;
}

