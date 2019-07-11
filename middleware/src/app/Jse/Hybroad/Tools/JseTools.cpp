
#include "JseTools.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"
#include "JseAssertions.h"

#include "AppSetting.h"
#include "Setting.h"
#include "app_epg_para.h"
#include "BrowserAgent.h"
#include "NativeHandler.h"
#include "ind_mem.h"
#include "MessageTypes.h"
#include "mid_fpanel.h"
#include "openssl/md5.h"

#ifdef TVMS_OPEN
#include "tvms_setting.h"
#endif

#include <stdlib.h>
#include <string.h>
extern "C" int yhw_board_setRunlevel(int runlevel);

using namespace Hippo;

static int createPasswdDigest(unsigned char* tag, unsigned char* passwd, int passwdLen, unsigned char* outBuf, int outBufLen )
{
    MD5_CTX ctx;

    if( ( NULL == tag) || ( NULL == passwd )||(passwdLen <= 0)){
        PRINTF("ERR ( NULL == tag) || ( NULL == passwd )\n");
        return -1;
    }
    if(outBufLen < 16 ){
        PRINTF("ERR outBufLen < 16 \n");
        return -1;
    }
    MD5_Init(&ctx);
    MD5_Update(&ctx, tag, strlen((const char*)tag));
    MD5_Update(&ctx, passwd, passwdLen );
    MD5_Final(outBuf, &ctx);

    return 0;
}

/*-1 err, 0 fail, 1 pass*/
static int landPasswdVerify(unsigned char* tag, unsigned char* passwd, int passwdLen)
{
    if ((NULL == tag) || (NULL == passwd) || (passwdLen <= 0)) {
        LogSafeOperDebug("ERR ( NULL == tag) || ( NULL == passwd )\n");
        return -1;
    }
#ifdef ENABLE_HIGH_SECURITY
    unsigned char outBuf[16] = { 0 };
    unsigned char outBuf2[16] = { 0 };
    char tmpValue[16] = { 0 };

    createPasswdDigest(tag, passwd, passwdLen, outBuf, 16);

    if (!memcmp(tag, "pass_yuxing", 11)) {
        createPasswdDigest(tag, "989464", strlen("989464"), outBuf2, 16);
        appSettingGetString("pass_yuxing", tmpValue, 16, 0);
        if ((memcmp(outBuf, tmpValue, 16) == 0) || (memcmp(outBuf, outBuf2, 16) == 0)) {
            return 1;
        }
    } else if (!memcmp(tag, "pass_huawei1", 12)) {
        appSettingGetString("pass_huawei1", tmpValue, 16, 0);
        createPasswdDigest(tag, "2878", strlen("2878"), outBuf2, 16);
        if ((memcmp(outBuf, tmpValue, 16) == 0) || (memcmp(outBuf, outBuf2, 16) == 0)) {
            return 1;
        }
    } else if (!memcmp(tag, "pass_huaweiQtel", 15)) {
        appSettingGetString("pass_huaweiQtel", tmpValue, 16, 0);
        createPasswdDigest(tag, "8888", strlen("8888"), outBuf2, 16);
        if ((memcmp(outBuf, tmpValue, 16) == 0) || (memcmp(outBuf, outBuf2, 16) == 0)) {
            return 1;
        }
    } else if (!memcmp(tag, "pass_huawei", 11)) {
        appSettingGetString("pass_huawei", tmpValue, 16, 0);
        createPasswdDigest(tag, "8288", strlen("8288"), outBuf2, 16);
        if ((memcmp(outBuf, tmpValue, 16) == 0) || (memcmp(outBuf, outBuf2, 16) == 0)) {
            return 1;
        }
    } else if (!memcmp(tag, "pass_read", 10)) {
        appSettingGetString("pass_read", tmpValue, 16, 0);
        createPasswdDigest(tag, "123456", strlen("123456"), outBuf2, 16);
        if ((memcmp(outBuf, tmpValue, 16) == 0) || (memcmp(outBuf, outBuf2, 16) == 0)) {
            return 1;
        }
    }
    return 0;

#else

    if (!memcmp(tag, "pass_yuxing", 11)) {
        if(memcmp(passwd, "989464", passwdLen) == 0) {
            return 1;
        }
    } else if (!memcmp(tag, "pass_huawei", 11)) {
        if(memcmp(passwd, "8288", passwdLen) == 0) {
            return 1;
        }
    } else if (!memcmp(tag, "pass_huawei1", 11)) {
        if(memcmp(passwd, "2878", passwdLen) == 0) {
            return 1;
        }
    } else if (!memcmp(tag, "pass_huaweiQtel", 15)) {
        if(memcmp(passwd, "8888", passwdLen) == 0) {
            return 1;
        }
    } else if (!memcmp(tag, "pass_read", 10)) {
        if(memcmp(passwd, "123456", passwdLen) == 0) {
            return 1;
        }
    }
    return 0;
#endif
}

static int JseCleanTakinCacheWrite(const char* param, char* value, int len)
{
    epgBrowserAgentCleanTakinCache();
    return 0;
}

static int JseYXOpSaveParamWrite(const char* param, char* value, int len)
{
    settingManagerSave();
#ifdef TVMS_OPEN
    tvms_config_save();
#endif
    return 0;
}

static int JseYXParaStbMonitorplayUrlRead(const char* param, char* value, int len)
{
    app_stbmonitor_tms_url_get(value);
    return 0;
}

static int JseEditStatusWrite(const char* param, char* value, int len)
{
    sys_key_editfocus_set(atoi(value));
    return 0;
}

static int JseLandPasswordRead(const char* param, char* value, int len)
{
    char *ptr = 0;
    char *ptr2 = 0;
    int ret = 0;
    unsigned char tag[32] = {0};
    unsigned char passwd[32] = {0};
    ptr = (char *)strchr(param + 13, ':');
    if(NULL != ptr) {
        ptr2 = strchr(ptr + 1, ':');
        if(NULL != ptr2) {
            IND_STRNCPY((char*)tag, ptr + 1, ptr2 - ptr - 1);
            IND_STRCPY((char*)passwd, ptr2 + 1);
            ret = landPasswdVerify(tag, passwd, strlen((const char*)passwd));
            if(ret < 0) {
                LogJseDebug("landPasswdVerify ERR\n");
                ret = 0;
            }
            sprintf(value, "%d", ret);
        } else {
            sprintf(value, "%d", 0);
        }
    } else {
        sprintf(value, "%d", 0);
    }
    return 0;
}

#ifdef HUAWEI_C10
static int JseDebugModeWrite(const char* param, char* value, int len)
{
    int tShow = 0;

    if (value) {
        tShow = atoi(value);
        if (tShow) {
            if(BrowserAgent::mPrompt){
                BrowserAgent::mPrompt->showDebug();
            }
        } else {
            if (BrowserAgent::mPrompt) {
                BrowserAgent::mPrompt->clearDebug();
            }
        }
    }
    return -1;
}
#endif

static int JseKeyBoardWrite(const char* param, char* value, int len)
{
    sendMessageToEPGBrowser(MessageType_KeyDown, 0x822, 0, 0);
    return 0;
}

static int JseCloseKeyboardWrite(const char* param, char* value, int len)
{
    return 0;
}

extern "C"
void appFactorySet(int flag)
{
    yhw_board_setRunlevel(4);
    mid_fpanel_reboot();
    return;
}


JseKeyBoard::JseKeyBoard()
	: JseGroupCall("browser")
{
    JseCall *call;

    call = new JseFunctionCall("showKeyboard", 0, JseKeyBoardWrite);
    regist(call->name(), call);

    call = new JseFunctionCall("closeKeyboard", 0, JseCloseKeyboardWrite);
    regist(call->name(), call);
}

JseKeyBoard::~JseKeyBoard()
{
}

/*************************************************
Description: 初始化海博Tools配置定义的接口，由JseHybroad.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseToolsInit()
{
    JseCall* call;

    call = new JseFunctionCall("CleanTakinCache", 0, JseCleanTakinCacheWrite);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseFunctionCall("yx_op_saveparam", 0, JseYXOpSaveParamWrite);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseFunctionCall("yx_para_stbmonitor_playurl", JseYXParaStbMonitorplayUrlRead, 0);
    JseRootRegist(call->name(), call);

   //C10/C20 regist
   call = new JseFunctionCall("editStatus", 0, JseEditStatusWrite);
   JseRootRegist(call->name(), call);

   //C10/C20 regist
   call = new JseFunctionCall("land_password", JseLandPasswordRead, 0);
   JseRootRegist(call->name(), call);

#ifdef HUAWEI_C10
    //C10 regist
    call = new JseFunctionCall("debugmod", 0, JseDebugModeWrite);
    JseRootRegist(call->name(), call);
#endif
    return 0;
}

