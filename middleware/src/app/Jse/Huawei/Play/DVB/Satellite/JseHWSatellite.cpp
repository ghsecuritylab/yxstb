
#include "JseHWSatellite.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"

static int JseSatCountRead(const char* param, char* value, int len)
{
    return 0;
}

static int JseSatListRead(const char* param, char* value, int len)
{
	return 0;
}

static int JseMySatCcountRead(const char* param, char* value, int len)
{
	return 0;
}

static int JseMySatListRead(const char* param, char* value, int len)
{
    return 0;
}

static int JseSatSaveParamsWrite(const char* param, char* value, int len)
{
	return 0;
}

static int JseSatAddWrite(const char* param, char* value, int len)
{
	return 0;
}

static int JseSatDeleteWrite(const char* param, char* value, int len)
{
    return 0;
}

static int JseSatListUpdateWrite(const char* param, char* value, int len)
{
	return 0;
}

/*************************************************
Description: 初始化华为播放流控中的DVBS模块之Satellite定义的接口，由JseHWDVB.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseHWSatelliteInit()
{
    JseCall* call;

    //C20 regist
    call = new JseFunctionCall("dvb_sat_get_count", JseSatCountRead, 0);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_sat_get_list", JseSatListRead, 0);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_my_sat_get_count", JseMySatCcountRead, 0);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_my_sat_get_list", JseMySatListRead, 0);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_sat_save_params", 0, JseSatSaveParamsWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_sat_add", 0, JseSatAddWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_sat_delete", 0, JseSatDeleteWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_satList_update", 0, JseSatListUpdateWrite);
    JseRootRegist(call->name(), call);
    return 0;
}

