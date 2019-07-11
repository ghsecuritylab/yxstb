
#ifdef INCLUDE_IMS
#include "JseHWCall.h"
#include "JseFunctionCall.h"
#include "JseAssertions.h"

#include "JseHWVoice.h"
#include "JseHWVideo.h"

#include <stdio.h>

//由于不知道下面函数中调用的接口具体包含哪些头文件，因此将可能包含的头文件全部列出
#include"platform_types.h"
#include"yx_ims_init.h"
#include"dev_flash.h"
#include"YX_IMS_porting.h"
#include"TMW_Camera.h"
#include"TMW_Media.h"

static int JseGetStateRead(const char* param, char* value, int len)
{
    return 0;
}

static int JseTransferFlagRead(const char* param, char* value, int len)
{
    char buf[8] = {0};
    int ret = 0;
    ret = STB_config_read("userInfo", "Call.TransferFlag", buf, 8);
    if(0 == ret) {
        sprintf(value, "%s", buf);
    } else {
        sprintf(value, "%d", 0);/*default value ,not support call transfer*/
        STB_config_write("userInfo", "Call.TransferFlag", "0");
    }

    return 0;
}

static int JseTransferFlagWrite(const char* param, char* value, int len)
{
    STB_config_write("userInfo", "Call.TransferFlag", value);

    return 0;
}

static int JseTransferPhoneRead(const char* param, char* value, int len)
{
    char buf[128] = {0};
    int ret = 0;

    ret = STB_config_read("userInfo", "Call.TransferPhone", buf, 128);
    if(0 == ret) {
        sprintf(value, "%s", buf);
    } else {
        YIMS_ERR();
    }

    return 0;
}

static int JseTransferPhoneWrite(const char* param, char* value, int len)
{
    char buf[128] = {0};
    int ret = 0;

    ret = STB_config_read("userInfo", "Call.TransferPhone", buf, 128);
    if(0 == ret) {
        sprintf(value, "%s", buf);
    } else {
        YIMS_ERR();
    }

    return 0;
}

JseHWCall::JseHWCall()
	: JseGroupCall("Call")
{
    JseCall* Call;

    Call = new JseFunctionCall("GetState", JseGetStateRead, 0);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("TransferFlag", JseTransferFlagRead, JseTransferFlagWrite);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("TransferPhone", JseTransferPhoneRead, JseTransferPhoneWrite);
    regist(Call->name(), Call);	
	
    Call = new JseHWVoice();
    regist(Call->name(), Call);	
    Call = new JseHWVideo();
    regist(Call->name(), Call);	
    
}

JseHWCall::~JseHWCall()
{
}

#endif
