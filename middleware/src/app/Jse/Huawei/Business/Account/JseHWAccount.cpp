
#include "JseHWAccount.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"
#include "JseAssertions.h"

#include "app_epg_para.h"
#include "cryptoFunc.h"
#include "charConvert.h"
#include "Account.h"

#include "customer.h"

#include "AppSetting.h"

#include <string.h>
#include <stdio.h>

/*****************************************************************
customer infomation <-> System parameters list in V1R5
(IPTV V100R005C03版本STB与EPG接口文档 基础业务(Webkit).doc 1.1.1)
*****************************************************************/
static int JseUserIDRead(const char* param, char* value, int len)
{
    appSettingGetString("ntvuser", value, len, 0);
    return 0;
}

static int JseUserIDWrite(const char* param, char* value, int len)
{
    appSettingSetString("ntvuser", value);
    return 0;
}

static int JsePwdRead(const char* param, char* value, int len)
{
    appSettingGetString("ntvAESpasswd", value, len, 0);
    return 0;
}

static int JsePwdWrite(const char* param, char* value, int len)
{
    appSettingSetString("ntvAESpasswd", value);
    settingManagerSave();
    return 0;
}


static int JseDefContAccRead(const char* param, char* value, int len)
{
    appSettingGetString("defContAcc", value, len, 0);
    return 0;
}

static int JseDefContAccWrite(const char* param, char* value, int len)
{
    appSettingSetString("defContAcc", value);
    return 0;
}

static int JseDefContPwdRead(const char* param, char* value, int len)
{
    appSettingGetString("defAESContpwd", value, len, 0);
    return 0;
}

static int JseDefContPwdWrite(const char* param, char* value, int len)
{
    appSettingSetString("defAESContpwd", value);
    return 0;
}

static int JseEncryptionTypeRead(const char* param, char* value, int len)
{
    strcpy(value, account().getEncryptionType());
    return 0;
}

static int JseEncryptionTypeWrite(const char* param, char* value, int len)
{
    account().setEncryptionType(value);
    return 0;
}

static int JseshareKeyWrite(const char* param, char* value, int len)
{
    account().setShareKey(value);
    return 0;
}

//TODO
static int EncryptTokenRead(const char* param, char* value, int len)
{
    return 0;
}

/************************************************************************
 EPG页面调用此接口通知STB对IPTV业务密码指定字符串进行加密，STB返回加密后的字符串。
 Utility.getValueByName('encyptedcontent:original=vstring')
 vstring(string 256),return(string 8)
 ************************************************************************/
static int JseencyptedcontentRead(const char* param, char* value, int len)
{
    char *p = NULL;

    p = (char *)param + strlen("original=");
#ifdef HUAWEI_C20
    md5Encypt(&p, 1, value, len, 1);
    data2Hex(value, 4, value, len);
#else
    if(!strncmp(account().getEncryptionType(), "0003", 4)){
        md5Encypt(&p, 1, value, len, 0);
        data2Hex(value, 12, value, len);
    }
    else {
        md5Encypt(&p, 1, value, len, 1);
        data2Hex(value, 4, value, len);
    }
#endif
    return 0;
}

/*************************************************
Description: 初始华为业务账号相关的Jse接口，由JseHWBusiness.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseHWAccountInit()
{
    JseCall* call;

    //UserId C10, userID C20 regist
    call = new JseFunctionCall("UserID", JseUserIDRead, JseUserIDWrite);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("userID", JseUserIDRead, JseUserIDWrite);
    JseRootRegist(call->name(), call);

    //C10 regist
    call = new JseFunctionCall("Pwd", JsePwdRead, JsePwdWrite);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseFunctionCall("defContAcc", JseDefContAccRead, JseDefContAccWrite);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("defContPwd", JseDefContPwdRead, JseDefContPwdWrite);
    JseRootRegist(call->name(), call);

    //C10 regist
    call = new JseFunctionCall("EncryptionType", JseEncryptionTypeRead, JseEncryptionTypeWrite);
    JseRootRegist(call->name(), call);

    //C10 regist
    call = new JseFunctionCall("shareKey", 0, JseshareKeyWrite);
    JseRootRegist(call->name(), call);

    //C10 regist
    call = new JseFunctionCall("EncryptToken", EncryptTokenRead, 0);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("encyptedcontent", JseencyptedcontentRead, 0);
    JseRootRegist(call->name(), call);

    ///C20 regist
    call = new JseFunctionCall("ntvuseraccount", JseUserIDRead, JseUserIDWrite);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("ntvuserpassword", JsePwdRead, JsePwdWrite);
    JseRootRegist(call->name(), call);

    return 0;
}

