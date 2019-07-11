
#if defined(INCLUDE_IMS)
#include "JseHWTempData.h"
#include "JseFunctionCall.h"
#include "JseAssertions.h"

#include <stdio.h>
#include <string.h>

//由于不知道下面函数中调用的接口具体包含哪些头文件，因此将可能包含的头文件全部列出
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
Description: 初始化并注册 hybroad 定义的Jse接口 <TempData.***> 
Input: 无
Return: 无
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
