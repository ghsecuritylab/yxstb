
#if defined(INCLUDE_IMS)

#include "JseHWIMS.h"

#include "JseRoot.h"
#include "JseFunctionCall.h"
#include "JseAssertions.h"

#include "pathConfig.h"
#include "AddrBook/JseHWAddrBook.h"
#include "Camera/JseHWCamera.h"
#include "Friends/JseHWFriends.h"
#include "FriendTV/JseHWFriendTV.h"
#include "Msg/JseHWMsg.h"
#include "TempData/JseHWTempData.h"
#include "Call/JseHWCall.h"

#include <stdio.h>
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


static int JseRegisterWrite(const char* param, char* value, int len)
{
    YX_IMS_JSON_PARSE jsonParse;
    YX_IMS_JSON_STR *pJsonNameValue = NULL;
    IMSServerInfo serverInfo;
    int i = 0;
    int ret = 0;
    memset(&serverInfo, 0, sizeof(IMSServerInfo));
    ims_json_parse_string(value, &jsonParse);
    pJsonNameValue = jsonParse.jsonStr;
    for(i = 0; i < jsonParse.count; i++) {
        if(NULL == pJsonNameValue) {
            YIMS_ERR();
            break;
        }
        if(!strcmp(pJsonNameValue->name, "ServerUrl")) {
            if(!strcmp(pJsonNameValue->value, "")) {
                char tempValue[64] = {0};

                ret = STB_config_read("userInfo", "IMS.ServerUrl", tempValue, 64);
                if(0 == ret) {
                    strncpy((serverInfo.szIMSServerURL), tempValue, strlen(tempValue));
                }
            } else {
                strncpy((serverInfo.szIMSServerURL), pJsonNameValue->value, strlen(pJsonNameValue->value));
                STB_config_write("userInfo", "IMS.ServerUrl", serverInfo.szIMSServerURL);
            }

        } else if(!strcmp(pJsonNameValue->name, "Domain")) {
            if(!strcmp(pJsonNameValue->value, "")) {
                char tempValue[64] = {0};
                ret = STB_config_read("userInfo", "IMS.Domain", tempValue, 64);
                if(0 == ret) {
                    strncpy((serverInfo.szDomain), tempValue, strlen(tempValue));
                }
            } else {
                strncpy((serverInfo.szDomain), pJsonNameValue->value, strlen(pJsonNameValue->value));
                STB_config_write("userInfo", "IMS.Domain", serverInfo.szDomain);
            }
        } else if(!strcmp(pJsonNameValue->name, "Account")) {

            if(!strcmp(pJsonNameValue->value, "")) {
                char tempValue[64] = {0};
                ret = STB_config_read("userInfo", "IMS.Account", tempValue, 64);
                if(0 == ret) {
                    strncpy((serverInfo.szIMPIAccount), tempValue, strlen(tempValue));
                    strncpy((serverInfo.szIMPUAccount), tempValue, strlen(tempValue));
                }
            } else {
                strncpy((serverInfo.szIMPIAccount), pJsonNameValue->value, ACCOUNT_LENGTH);
                strncpy((serverInfo.szIMPUAccount), pJsonNameValue->value, ACCOUNT_LENGTH);
                char tempBuf[128] = {0};
                int ret = 0;
                ret = STB_config_read("userInfo", "IMS.Account", tempBuf, 128);
                if(ret) {
                    if(strcmp(tempBuf, serverInfo.szIMPUAccount)) {
                        STB_config_remove("IMS", "URI");
                    }
                }
                STB_config_write("userInfo", "IMS.Account", serverInfo.szIMPUAccount);
            }
            if(strcmp(serverInfo.szDomain, "")) { /*if not get domain, get domail from account*/
                char *ptr = NULL;
                ptr = strchr(serverInfo.szIMPUAccount, '@');
                if(NULL != ptr) {
                    sprintf(serverInfo.szDomain, "%s", ptr + 1);
                    STB_config_write("userInfo", "IMS.Domain", serverInfo.szDomain);
                }
            }


        } else if(!strcmp(pJsonNameValue->name, "Pwd")) {
            if(!strcmp(pJsonNameValue->value, "")) {
                char tempValue[64] = {0};
                ret = STB_config_read("userInfo", "IMS.Pwd", tempValue, 64);
                if(0 == ret) {
                    strncpy((serverInfo.szIMPIPassword), tempValue, strlen(tempValue));
                    strncpy((serverInfo.szIMPUPassword), tempValue, strlen(tempValue));
                }
            } else {
                strncpy((serverInfo.szIMPIPassword), pJsonNameValue->value, PASSWORD_LENGTH);
                strncpy((serverInfo.szIMPUPassword), pJsonNameValue->value, PASSWORD_LENGTH);
                STB_config_write("userInfo", "IMS.Pwd", serverInfo.szIMPUPassword);
            }
        } else {
            YIMS_ERR();
        }
        pJsonNameValue = pJsonNameValue->next;
    }
    ims_json_parse_free(&jsonParse);



    ret = STB_config_read("userInfo", "IMS.PGMServer", serverInfo.szPGMServerURL, 256);
    if(ret) {
        YIMS_ERR();
    }
    /*before register , compare the register account with the account that cbb saved,*/
    /*if different, delete the account cbb saved*/
    STB_INT8 preAccount[128] = {0};
    ret = STB_config_read("IMS", "URI", preAccount, 128);
    if((0 == ret) && (!strcmp(preAccount, ""))) { /*read URI ok, and URI is not BLANK*/
        if(!strcmp(preAccount, serverInfo.szIMPUAccount)) {
            STB_config_remove("IMS", "URI");
        }
    }
    YX_IMS_register(&serverInfo);

    return 0;
}

static int JseUnRegisterWrite(const char* param, char* value, int len)
{
    int ret = 0;
    ret = YX_IMS_unregister();
    if(-1 == ret) { /*err*/
        YX_IMS_putEventToList(EVENT_IMS_REGISTER, "EVENT_IMS_UNREGISTER_FAILED", 201105, NULL, NULL);
    } else {
        YX_IMS_putEventToList(EVENT_IMS_REGISTER, "EVENT_IMS_UNREGISTER_SUCCESS", 201104, NULL, NULL);
    }

    return 0;
}

static int JseServerUrlRead(const char* param, char* value, int len)
{
    int ret = 0;
    char buf[512] = { 0 };

    ret = STB_config_read("userInfo", "IMS.ServerUrl", buf, 512);
    if(0 == ret) {
        strcpy(value, buf);
        return 0;
    }

    return -1;
}

static int JseServerUrlWrite(const char* param, char* value, int len)
{
    STB_config_write("userInfo", "IMS.ServerUrl", value);

    return 0;
}

static int JseDomainRead(const char* param, char* value, int len)
{
    int ret = 0;
    char  buf[512] = {
        0
    };

    ret = STB_config_read("userInfo", "IMS.Domain", buf, 512);
    if(0 == ret) {
        strcpy(value, buf);
    } else {
        //sprintf(buf, "huawei.com");
        return -1;
    }

    return 0;
}

static int JseDomainWrite(const char* param, char* value, int len)
{
    STB_config_write("userInfo", "IMS.Domain", value);

    return 0;
}

static int JseAccountRead(const char* param, char* value, int len)
{
    int ret = 0;
    char buf[512] = {0};
    ret = STB_config_read("userInfo", "IMS.Account", buf, 512);
    if(0 == ret) {
        strcpy(value, buf);
    } else {
        return -1;
        //sprintf(buf, "Test001@huawei.com");
    }

    return 0;
}

static int JseAccountWrite(const char* param, char* value, int len)
{
    STB_config_write("userInfo", "IMS.Account", value);

    return 0;
}

static int JsePwdRead(const char* param, char* value, int len)
{
    int ret = 0;
    char buf[512] = {0};
    ret = STB_config_read("userInfo", "IMS.Pwd", buf, 512);
    if(0 == ret) {
        strcpy(value, buf);
    } else {
        return -1;
    }

    return 0;
}

static int JsePwdWrite(const char* param, char* value, int len)
{
    STB_config_write("userInfo", "IMS.Pwd", value);

    return 0;
}

static int JsePGMServerRead(const char* param, char* value, int len)
{
    int ret = 0;
    char buf[512] = {0};
    ret = STB_config_read("userInfo", "IMS.PGMServer", buf, 512);
    if(0 == ret) {
        strcpy(value, buf);
    } else {
        return -1;
    }

    return 0;
}

static int JsePGMServerWrite(const char* param, char* value, int len)
{
    STB_config_write("userInfo", "IMS.PGMServer", value);
    STB_config_write("Pgm", "Pgm.RegServer", value);

    return 0;
}

static int JseSupportRead(const char* param, char* value, int len)
{
    int ret = 0;
    char buf[128] = {0};
    ret = STB_config_read("userInfo", "IMS.Support", buf, 128);
    if(0 == ret) {
        strcpy(value, buf);
    } else {
        sprintf(value, "%d", 1);
        STB_config_write("userInfo", "IMS.Support", buf);
    }

    return 0;
}

static int JseSupportWrite(const char* param, char* value, int len)
{
    STB_config_write("userInfo", "IMS.Support", buf);

    return 0;
}

/*************************************************
Description: 初始化并注册华为定义的接口 <IMS.***>
接口相关说明见 《IPTV STB V100R002C10&C20 IMS业务STB与EPG接口说明书》
Input: 无
Return: 无
 *************************************************/
JseHWIMS::JseHWIMS()
	: JseGroupCall("IMS")
{
    JseCall* Call;
	
    Call = new JseFunctionCall("Register", 0, JseRegisterWrite);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("UnRegister", 0, JseUnRegisterWrite);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("ServerUrl", JseServerUrlRead, JseServerUrlWrite);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("Domain", JseDomainRead, JseDomainWrite);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("Account", JseAccountRead, JseAccountWrite);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("Pwd", JsePwdRead, JsePwdWrite);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("PGMServer", JsePGMServerRead, JsePGMServerWrite);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("Support", JseSupportRead, JseSupportWrite);
    regist(Call->name(), Call);
	
}

JseHWIMS::~JseHWIMS()
{
}

/*************************************************
Description: 初始化华为系统配置定义的接口，由JseHuawei.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseHWIMSInit()
{
    JseCall* call;

    call = new JseHWIMS();
    JseRootRegist(call->name(), call);

    call = new JseHWCall();
    JseRootRegist(call->name(), call);

    call = new JseHWAddrBook();
    JseRootRegist(call->name(), call);

    call = new JseHWCamera();
    JseRootRegist(call->name(), call);

    call = new JseHWFriends();
    JseRootRegist(call->name(), call);
    
    call = new JseHWFriendTV();
    JseRootRegist(call->name(), call);    
    
    call = new JseHWMsg();
    JseRootRegist(call->name(), call); 
    
    call = new JseHWTempData();
    JseRootRegist(call->name(), call);     
        
    return 0;
}

#endif //INCLUDE_IMS

