
#include "JseHWSearch.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"

static int JseSearchStopWrite(const char* param, char* value, int len)
{
    return 0;
}

static int JseAntennaSetupStartWrite(const char* param, char* value, int len)
{
    return 0;
}

static int JseAntennaSetupProgressRead(const char* param, char* value, int len)
{
	return 0;
}

static int JseAntennaSetupInfoRead(const char* param, char* value, int len)
{
	return 0;
}

static int JseSatSearchStartWrite(const char* param, char* value, int len)
{
	return 0;
}

static int JseSatSearchProgressRead(const char* param, char* value, int len)
{
	return 0;
}

static int JseBlindSearchTpStartWrite(const char* param, char* value, int len)
{
	return 0;
}

static int JseBlindSearchTPProgressRead(const char* param, char* value, int len)
{
	return 0;
}

static int JseTpSearchStartWrite(const char* param, char* value, int len)
{
	return 0;
}

static int JseTpSearchProgressRead(const char* param, char* value, int len)
{
	return 0;
}

static int JseSignalQualityRead(const char* param, char* value, int len)
{
    return 0;
}

static int JseSignalStrengthRead(const char* param, char* value, int len)
{
	return 0;
}

/*************************************************
Description: 初始化华为播放流控中的DVBS模块之Search定义的接口，由JseHWDVB.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseHWSearchInit()
{
    JseCall* call;

    //C20 regist
    call = new JseFunctionCall("dvb_search_stop", 0, JseSearchStopWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_antenna_auto_setup_start", 0, JseAntennaSetupStartWrite);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("dvb_antenna_auto_setup_get_info", JseAntennaSetupInfoRead, 0);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("dvb_antenna_auto_setup_get_progress", JseAntennaSetupProgressRead, 0);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_sat_search_start", 0, JseSatSearchStartWrite);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("dvb_sat_search_get_progress", JseSatSearchProgressRead, 0);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_blind_search_tp_start", 0, JseBlindSearchTpStartWrite);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("dvb_blind_search _tp_get_progress", JseBlindSearchTPProgressRead, 0);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_tp_search_start", 0, JseTpSearchStartWrite);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("dvb_tp_search_get_progress", JseTpSearchProgressRead, 0);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_get_signal_quality", JseSignalQualityRead, 0);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("dvb_get_signal_strength", JseSignalStrengthRead, 0);
    JseRootRegist(call->name(), call);
    return 0;
}

