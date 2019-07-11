
#include "JseHWTP.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"

static int JseTesetTPListWrite(const char* param, char* value, int len)
{
    return 0;
}

static int JseNewTpCountRead(const char* param, char* value, int len)
{
    return 0;
}

static int JseNewTpInfoRead(const char* param, char* value, int len)
{
	return 0;
}

static int JseTpCountRead(const char* param, char* value, int len)
{
	return 0;
}

static int JseTpLlistRead(const char* param, char* value, int len)
{
    return 0;
}

static int JseTpSaveParamsWrite(const char* param, char* value, int len)
{
	return 0;
}

static int JseTpAddWrite(const char* param, char* value, int len)
{
	return 0;
}

static int JseTpDeleteWrite(const char* param, char* value, int len)
{
    return 0;
}

static int JseDefaultTPWrite(const char* param, char* value, int len)
{
	return 0;
}

/*************************************************
Description: 初始化华为播放流控中的DVBS模块之TP定义的接口，由JseHWDVB.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseHWTPInit()
{
    JseCall* call;

    //C20 regist
    call = new JseFunctionCall("dvb_tesetTPList_update", 0, JseTesetTPListWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_new_tp_get_count", JseNewTpCountRead, 0);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_new_tp_get_info", JseNewTpInfoRead, 0);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_tp_get_count", JseTpCountRead, 0);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_tp_get_list", JseTpLlistRead, 0);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_tp_save_params", 0, JseTpSaveParamsWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_tp_add", 0, JseTpAddWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_tp_delete", 0, JseTpDeleteWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("DVBDefaultTP", 0, JseDefaultTPWrite);
    JseRootRegist(call->name(), call);
    return 0;
}

