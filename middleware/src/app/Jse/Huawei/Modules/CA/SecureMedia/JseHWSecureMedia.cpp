
#include "JseHWSecureMedia.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"

//TODO
static int JseSMRegisterWrite(const char* param, char* value, int len)
{
    return 0;
}

//TODO
static int JseSMSyncrightsWrite(const char* param, char* value, int len)
{
    return 0;
}

//TODO
static int JseSMReqServiceWrite(const char* param, char* value, int len)
{
    return 0;
}

/*************************************************
Description: ��ʼ����ΪSecureMediaģ�����ö���Ľӿڣ���JseHWCA.cpp����
Input: ��
Return: ��
 *************************************************/
int JseHWSecureMediaInit()
{
    JseCall* call;

    //����ȫΪC20ע��
    call = new JseFunctionCall("hw_op_smregister", 0, JseSMRegisterWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("hw_op_smsyncrights", 0, JseSMSyncrightsWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("hw_op_smreqservice", 0, JseSMReqServiceWrite);
    JseRootRegist(call->name(), call);
    return 0;
}

