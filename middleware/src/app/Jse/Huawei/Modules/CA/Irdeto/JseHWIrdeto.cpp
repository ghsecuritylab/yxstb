
#include "JseHWIrdeto.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"

//TODO
static int JseCAEmmSrcipWrite(const char* param, char* value, int len)
{
    return 0;
}

//TODO
static int JseDelIrdetoSoftClientFileWrite(const char* param, char* value, int len)
{
    return 0;
}

//TODO
static int JseServiceaddressWrite(const char* param, char* value, int len)
{
    return 0;
}

//TODO
static int JseCAEmmUpdateWrite(const char* param, char* value, int len)
{
    return 0;
}

/*************************************************
Description: ��ʼ����ΪIrdetoģ�����ö���Ľӿڣ���JseHWCA.cpp����
Input: ��
Return: ��
 *************************************************/
int JseHWIrdetoInit()
{
    JseCall* call;

    //����ȫΪC20ע��
    call = new JseFunctionCall("CA_EMM_SRCIP", 0, JseCAEmmSrcipWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("DelIrdetoSoftClientFile", 0, JseDelIrdetoSoftClientFileWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("serviceAddress", 0, JseServiceaddressWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("caEmmUpdate", 0, JseCAEmmUpdateWrite);
    JseRootRegist(call->name(), call);
    return 0;
}

