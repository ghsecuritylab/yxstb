
#include "JseHWProgram.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"

static int JsePfProgramItemRead(const char* param, char* value, int len)
{
    return 0;
}

static int JseProgramItemCountRead(const char* param, char* value, int len)
{
	return 0;
}

static int JseProgramItemRead(const char* param, char* value, int len)
{
	return 0;
}

static int JseCurrentProgramPositionRead(const char* param, char* value, int len)
{
	return 0;
}

static int JsePFReviseSwitchRead(const char* param, char* value, int len)
{
	return 0;
}

static int JsePFReviseSwitchWrite(const char* param, char* value, int len)
{
	return 0;
}

/*************************************************
Description: ��ʼ����Ϊ���������е�DVBSģ��֮Program����Ľӿڣ���JseHWDVB.cpp����
Input: ��
Return: ��
 *************************************************/
int JseHWProgramInit()
{
    JseCall* call;

    //C20 regist
    call = new JseFunctionCall("dvb_get_pf_programItem", JsePfProgramItemRead, 0);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_programItem_get_count", JseProgramItemCountRead, 0);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_programItem_get", JseProgramItemRead, 0);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_get_current_program_position", JseCurrentProgramPositionRead, 0);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_pf_revise_switch", JsePFReviseSwitchRead, JsePFReviseSwitchWrite);
    JseRootRegist(call->name(), call);
    return 0;
}

