
#include "JseHWICAS.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"

//TODO
static int JseICASRegisterWrite(const char* param, char* value, int len)
{
    return 0;
}

//TODO
static int JseICASSyncrightsWrite(const char* param, char* value, int len)
{
    return 0;
}

//TODO
static int JseICASReqserviceWrite(const char* param, char* value, int len)
{
    return 0;
}

//TODO
static int JseDelICASSoftClientFileWrite(const char* param, char* value, int len)
{
    return 0;
}

/*************************************************
Description: ��ʼ����ΪICASģ�����ö���Ľӿڣ���JseHWCA.cpp����
Input: ��
Return: ��
 *************************************************/
int JseHWICASInit()
{
    JseCall* call;

    //����ȫΪC20ע��
    call = new JseFunctionCall("hw_op_icasregister", 0, JseICASRegisterWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("hw_op_icassyncrights", 0, JseICASSyncrightsWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("hw_op_icasreqservice", 0, JseICASReqserviceWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("DeliCASSoftClientFile", 0, JseDelICASSoftClientFileWrite);
    JseRootRegist(call->name(), call);
    return 0;
}

