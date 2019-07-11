
#include "JseHWPIP.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"

#include "AppSetting.h"

#include <stdio.h>

static int JseLastPipChannelNoRead(const char* param, char* value, int len)
{
    int lastPipChannelNo = 0;
    appSettingGetInt("pipchannelid", &lastPipChannelNo, 0);
    sprintf(value, "%d", lastPipChannelNo);
    return 0;
}

/*************************************************
Description: ��ʼ����ΪPIPģ�����ö���Ľӿڣ���JseModules.cpp����
Input: ��
Return: ��
 *************************************************/
int JseHWPIPInit()
{
    JseCall* call;

    call = new JseFunctionCall("lastPipChannelNo", JseLastPipChannelNoRead, 0);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("lastpipID", JseLastPipChannelNoRead, 0);
    JseRootRegist(call->name(), call);
    return 0;
}

