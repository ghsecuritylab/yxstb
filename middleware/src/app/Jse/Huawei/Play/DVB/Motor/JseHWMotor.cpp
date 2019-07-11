
#include "JseHWMotor.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"

static int JseDriveMotorWrite(const char* param, char* value, int len)
{
    return 0;
}

static int JseStopMotorWrite(const char* param, char* value, int len)
{
	return 0;
}

static int JseSavePositionWrite(const char* param, char* value, int len)
{
	return 0;
}

static int JseDriveMotorByStepWrite(const char* param, char* value, int len)
{
    return 0;
}

static int JseGotoReferenceWrite(const char* param, char* value, int len)
{
	return 0;
}

static int JseResetPositionWrite(const char* param, char* value, int len)
{
	return 0;
}

static int JseLimitWrite(const char* param, char* value, int len)
{
	return 0;
}


/*************************************************
Description: 初始化华为播放流控中的DVBS模块之Motor定义的接口，由JseHWDVB.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseHWMotorInit()
{
    JseCall* call;

    //C20 regist
    call = new JseFunctionCall("dvb_drive_motor", 0, JseDriveMotorWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_stop_motor", 0, JseStopMotorWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_save_position", 0, JseSavePositionWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_drive_motor_by_step", 0, JseDriveMotorByStepWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_goto_reference", 0, JseGotoReferenceWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_reset_position", 0, JseResetPositionWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_set_limit", 0, JseLimitWrite);
    JseRootRegist(call->name(), call);
    return 0;
}

