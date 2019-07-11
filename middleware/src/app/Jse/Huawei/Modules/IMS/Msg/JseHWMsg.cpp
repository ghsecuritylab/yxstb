
#if defined(INCLUDE_IMS)
#include "JseHWMsg.h"
#include "JseFunctionCall.h"
#include "JseAssertions.h"

#include <string.h>

//由于不知道下面函数中调用的接口具体包含哪些头文件，因此将可能包含的头文件全部列出
#include"FriendBasic.h"
#include"Friends.h"
#include"FriendTV.h"
#include"Group.h"
#include"GroupMember.h"
#include"IMSRegister.h"
#include"InfoPush.h"
#include"platform_types.h"
#include"yx_ims_init.h"
#include"dev_flash.h"
#include"YX_IMS_porting.h"
#include"TMW_Camera.h"
#include"TMW_Media.h"

static int JseGetCountRead(const char* param, char* value, int len)
{
    char* ptr = NULL;
    int ret = 0;

    ptr = strchr(func, ',');
    if(NULL != ptr) {
        ret = TMW_friends_IMGetCount(ptr + 2);
        if(ret >= 0) {
            sprintf(value, "%d", ret);
        } else {
            YIMS_ERR();
        }
    } else {
        YIMS_ERR();
    }

    return 0;
}


static int JseGetInfoRead(const char* param, char* value, int len)
{
    char* ptr1 = NULL;
    char* ptr2 = NULL;

    const MessageInfo* pMsgInfo = NULL;
    char url[256] = {
        0
    };
    int index = 0;

    ptr1 = strchr(func, ',');
    ptr2 = strchr(ptr1 + 1, ',');
    if(NULL != ptr1 && NULL != ptr2) {
        memcpy(url, ptr1 + 1, (ptr2 - ptr1 - 1));
        index = strtol(ptr2 + 1, NULL, 10);
        pMsgInfo = TMW_friends_IMInfo(url, index);
        if(NULL != pMsgInfo) {
            sprintf(value,
                    "{\"type\":\"%d\",\"uri\":\"%s\",\"text\":\"%s\",\"time\":\"%s\"}",
                    \
                    pMsgInfo->eType,
                    pMsgInfo->szURI,
                    pMsgInfo->szText,
                    pMsgInfo->szSendTime);
        } else {
            YIMS_ERR();
        }
    } else {
        YIMS_ERR();
    }

    return 0;
}

static int JseSendMsgWrite(const char* param, char* value, int len)
{
    YX_IMS_JSON_PARSE jsonParse;
    YX_IMS_JSON_STR* pJsonNameValue = NULL;
    char url[128] = { 0 };
    char text[512] = { 0 };
    int ret = 0;

    ims_json_parse_string(value, &jsonParse);
    pJsonNameValue = jsonParse.jsonStr;
    if(NULL != pJsonNameValue) {
        strncpy(url, pJsonNameValue->value, 128);
        pJsonNameValue = pJsonNameValue->next;
        if(NULL != pJsonNameValue) {
            strncpy(text, pJsonNameValue->value, 512);
        } else {
            YIMS_ERR();
            ret = -1;
        }
    } else {
        YIMS_ERR();
        ret = -1;
    }
    ims_json_parse_free(&jsonParse);

    if(0 == ret) {
        ret = TMW_friends_IMSend(url, text);
        if(ret) {
            YIMS_ERR();
        }
    }

    return 0;
}

/*************************************************
Description: 初始化并注册华为定义的接口 <Msg.***>  该接口是IMS
             系统的子功能接口。
接口相关说明见 《IPTV STB V100R002C10&C20 IMS业务STB与EPG接口说明书》
Input: 无
Return: 无
 *************************************************/
JseHWMsg::JseHWMsg()
	: JseGroupCall("Msg")
{
    JseCall* Call;
	
    Call = new JseFunctionCall("GetCount", JseGetCountRead, 0);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("GetInfo", JseGetInfoRead, 0);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("SendMsg", 0, JseSendMsgWrite);
    regist(Call->name(), Call);	

}

JseHWMsg::~JseHWMsg()
{
}

#endif
