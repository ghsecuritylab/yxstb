
#include "JseAnalog.h"
#include "JseFunctionCall.h"
#include "JseRoot.h"

#include "AppSetting.h"

#include <stdio.h>
#include <stdlib.h>


static int JseMacroRead(const char* param, char* value, int len)
{
    int macrovision = 0;
    appSettingGetInt("macrovision", &macrovision, 0);
    sprintf(value, "%d", macrovision);
    return 0;
}

static int JseMacroWrite(const char* param, char* value, int len)
{
    appSettingSetInt("macrovision", atoi(value));
    return 0;
}

/*************************************************
Description: ��ʼ������ģ������˿����ö���Ľӿڣ���JseIO.cpp����
Input: ��
Return: ��
**************************************************/
int JseAnalogInit()
{
    JseCall* call;

    //C20 regist
    call = new JseFunctionCall("yx_para_macro", JseMacroRead, JseMacroWrite);
    JseRootRegist(call->name(), call);
    return 0;
}

