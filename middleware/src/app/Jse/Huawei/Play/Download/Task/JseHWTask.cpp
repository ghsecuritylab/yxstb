
#include "JseHWTask.h"
#include "JseFunctionCall.h"

//TODO
static int JseCurrentIDRead(const char *param, char *value, int len)
{
    return 0;
}

//TODO
static int JseADDSucessWrite(const char *param, char *value, int len)
{
    return 0;
}

//TODO
static int JseGetCountRead(const char *param, char *value, int len)
{
    return 0;
}

//TODO
static int JseGetListRead(const char *param, char *value, int len)
{
    return 0;
}

//TODO
static int JseGetExistRead(const char *param, char *value, int len)
{
    return 0;
}

//TODO
static int JseStopWrite(const char *param, char *value, int len)
{
    return 0;
}


//TODO
static int JseResumeWrite(const char *param, char *value, int len)
{
    return 0;
}

//TODO
static int JseDeletetWrite(const char *param, char *value, int len)
{
    return 0;
}

//TODO
static int JseDownlaodSupportRead(const char *param, char *value, int len)
{
    return 0;
}

//TODO
static int JseGetInfoRead(const char *param, char *value, int len)
{
    return 0;
}

/*************************************************
Description: ��ʼ����ע�ỪΪ����Ľӿ� <Download.Task.***>
�ӿ����˵���� ��IPTV ����汾STB��EPG�ӿ��ĵ� Download(webkit) V1.1.doc��
Input: ��
Return: ��
 *************************************************/
JseHWTask::JseHWTask()
	: JseGroupCall("Task")
{
    JseCall* call;

    //C20 regist
    call = new JseFunctionCall("GetCurrentID", JseCurrentIDRead, 0);
    regist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("AddSuccess", 0, JseADDSucessWrite);
    regist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("GetCount", JseGetCountRead, 0);
    regist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("GetList", JseGetListRead, 0);
    regist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("GetExist", JseGetExistRead, 0);
    regist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("Stop", 0, JseStopWrite);
    regist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("Resume", 0, JseResumeWrite);
    regist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("Delete", 0, JseDeletetWrite);
    regist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("GetMetadata", JseDownlaodSupportRead, 0);
    regist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("GetInfo", JseGetInfoRead, 0);
    regist(call->name(), call);
}

JseHWTask::~JseHWTask()
{
}