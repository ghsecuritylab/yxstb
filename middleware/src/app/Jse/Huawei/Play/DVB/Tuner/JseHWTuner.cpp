
#include "JseHWTuner.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"

static int JseTunerLockStatusRead(const char* param, char* value, int len)
{
	return 0;
}

static int JseDvbTuneWrite(const char* param, char* value, int len)
{
    return 0;
}

static int JseTunerLnbConnectTypeRead(const char* param, char* value, int len)
{
	return 0;
}

static int JseTunerLnbConnectTypeWrite(const char* param, char* value, int len)
{
	return 0;
}

static int JseTunerNumRead(const char* param, char* value, int len)
{
	return 0;
}

/*************************************************
Description: 初始化华为播放流控中的DVBS模块之Tuner定义的接口，由JseHWDVB.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseHWTunerInit()
{
    JseCall* call;

    //C20 regist
    call = new JseFunctionCall("dvb_get_tuner_lock_status", JseTunerLockStatusRead, 0);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_tune", 0, JseDvbTuneWrite);
    JseRootRegist(call->name(), call);

     //C20 regist
    call = new JseFunctionCall("dvb_get_tuner_lnb_connect_type", JseTunerLnbConnectTypeRead, 0);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_set_tuner_lnb_connect_type", 0, JseTunerLnbConnectTypeWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_get_tuner_num", JseTunerNumRead, 0);
    JseRootRegist(call->name(), call);
    return 0;
}

