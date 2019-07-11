
#include "JseLogo.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"

#include "AppSetting.h"

#include <stdio.h>
#include <unistd.h>

#ifdef Jiangsu
static int JseBootPicExistRead(const char* param, char* value, int len)
{
    if(access(BOOT_PIC_PATH, R_OK | F_OK) < 0) // the second pic
        snprintf( value, len, "%d", 0);
    else
        snprintf( value, len, "%d", 1);

    return 0;
}

static int JseBootPicShowEnableRead(const char* param, char* value, int len)
{
    int bootPicShowEnable = 0;
	appSettingGetInt("bootPicEnableFlag", &bootPicShowEnable, 0);
    snprintf( value, len, "%d", bootPicShowEnable); // 2 is start pic
    return 0;
}
#endif

/*************************************************
Description: ��ʼ������ҵ��ͼ�����ö���Ľӿ�,��JseBusiness.cpp����
Input: ��
Return: ��
 *************************************************/
int JseLogoInit()
{
    JseCall* call;

#ifdef Jiangsu
    call = new JseFunctionCall("isBootPicExist", JseBootPicExistRead, 0);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("bootPicShowEnable", JseBootPicShowEnableRead, 0);
    JseRootRegist(call->name(), call);
#endif
    return 0;
}
