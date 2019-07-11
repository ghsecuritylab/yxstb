
#include "JseHWCA.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"

#if defined(INCLUDE_VMCA)
#include "Verimatrix/JseHWVerimatrix.h"
#endif

#include "ICAS/JseHWICAS.h"
#include "Irdeto/JseHWIrdeto.h"
#include "Nagra/JseHWNagra.h"
#include "SecureMedia/JseHWSecureMedia.h"

//TODO
static int JseCaidRead(const char* param, char* value, int len)
{
    return 0;
}

//TODO
static int JseCaidWrite(const char* param, char* value, int len)
{
    return 0;
}

/*************************************************
Description: ��ʼ����ΪCAģ�����ö���Ľӿڣ���JseModules.cpp����
Input: ��
Return: ��
 *************************************************/
int JseHWCAInit()
{
    JseCall* call;

    //C20 regist
    call = new JseFunctionCall("caid", JseCaidRead, JseCaidWrite);
    JseRootRegist(call->name(), call);

#if defined(INCLUDE_VMCA)
    //C20 regist
    call = new JseHWVerimatrix();
    JseRootRegist(call->name(), call);
#endif

    //�����ĸ�ģ��ע��Ķ�û��ʵ�֡�
    JseHWICASInit();
    JseHWIrdetoInit();
    JseHWNagraInit();
    JseHWSecureMediaInit();
    return 0;
}

