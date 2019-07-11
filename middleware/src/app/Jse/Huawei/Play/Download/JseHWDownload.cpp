
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
Description: ��ʼ����ע�ỪΪ����Ľӿ� <Download.***>
�ӿ����˵���� ��IPTV ����汾STB��EPG�ӿ��ĵ� Download(webkit) V1.1.doc��
Input: ��
Return: ��
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