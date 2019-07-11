
#include "JseHWDownload.h"
#include "JseFunctionCall.h"
#include "Task/JseHWTask.h"

//TODO
static int JseDownlaodSupportRead(const char *param, char *value, int len)
{
    return 0;
}

//TODO
static int JseDownloadStartWrite(const char *param, char *value, int len)
{
    return 0;
}

/*************************************************
Description: 初始化并注册华为定义的接口 <Download.***>
接口相关说明见 《IPTV 海外版本STB与EPG接口文档 Download(webkit) V1.1.doc》
Input: 无
Return: 无
 *************************************************/
JseHWDownload::JseHWDownload()
	: JseGroupCall("Download")
{
    JseCall* call;

    call = new JseHWTask();
    regist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("Support", JseDownlaodSupportRead, 0);
    regist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("Start", 0, JseDownloadStartWrite);
    regist(call->name(), call);
}

JseHWDownload::~JseHWDownload()
{
}