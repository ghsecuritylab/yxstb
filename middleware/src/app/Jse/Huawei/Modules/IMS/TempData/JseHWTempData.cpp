
#if defined(INCLUDE_IMS)
#include "JseHWTempData.h"
#include "JseFunctionCall.h"
#include "JseAssertions.h"

#include <stdio.h>
#include <string.h>

//���ڲ�֪�����溯���е��õĽӿھ��������Щͷ�ļ�����˽����ܰ�����ͷ�ļ�ȫ���г�
#include"platform_types.h"
#include"yx_ims_init.h"
#include"dev_flash.h"
#include"YX_IMS_porting.h"
#include"TMW_Camera.h"
#include"TMW_Media.h"

static int JseGetCallIDRead(const char* param, char* value, int len)
{
    int ret = 0;
    char buf[64] = {0};
    ret = STB_config_read("userInfo", "IMS.getCallID", buf, 64);
    if(0 == ret) {
        strcpy(value, buf);
    } else {
        return -1;
    }

    return 0;
}

static int JseGetCallIDWrite(const char* param, char* value, int len)
{
    int ret = 0;

    ret = STB_config_write("userInfo", "TempData.getCallID", value);
    if(ret) {
        YIMS_ERR();
    }

    return 0;
}

static int JseGetMessageEnableWrite(const char* param, char* value, int len)
{
    int ret = 0;
    char buf[64] = {0};
    ret = STB_config_read("userInfo", "TempData.getMessageEnable", buf, 64);
    if(0 == ret) {
        sprintf(value, "%s", buf);
    } else {
        YIMS_ERR();
    }

    return 0;
}

static int JseGetMessageEnableRead(const char* param, char* value, int len)
{
    int ret = 0;
    char buf[64] = {0};
    ret = STB_config_read("userInfo", "TempData.getMessageEnable", buf, 64);
    if(0 == ret) {
        sprintf(value, "%s", buf);
    } else {
        YIMS_ERR();
    }

    return 0;
}

/*************************************************
Description: ��ʼ����ע�� hybroad �����Jse�ӿ� <TempData.***> 
Input: ��
Return: ��
 *************************************************/
JseHWTempData::JseHWTempData()
	: JseGroupCall("TempData")
{
    JseCall* Call;
	
    Call = new JseFunctionCall("getCallID", JseGetCallIDRead, JseGetCallIDWrite);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("getMessageEnable", JseGetMessageEnableRead, JseGetMessageEnableWrite);
    regist(Call->name(), Call);	

}

JseHWTempData::~JseHWTempData()
{
}

#endif
