#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "libzebra.h"
#include "sdk/yxsdk_func.h"

#include "config/webpageConfig.h"
#include "takin.h"
#include "Assertions.h"
#include "MessageTypes.h"
#include "Business.h"

#include "mid/mid_time.h"

#include "NativeHandlerPublic.h"
#include "NativeHandlerAssertions.h"
#ifdef INCLUDE_DMR
#include "DMRPlayerHuawei.h"
#include "DMRPlayerHuaweiC30.h"
#endif

#include "YX_codec_porting.h"

#ifdef HUAWEI_C20
#include "app_c20_init.h"
#endif //HUAWEI_C20

#include "sys_msg.h"
#include "SysSetting.h"

#include "browser_event.h"
#include "BrowserAgent.h"
#include "TAKIN_setting_type.h"
#include "TAKIN_interface.h"
#include "TAKIN_mid_porting.h"

#include "SystemManager.h"
#include "BrowserAgent.h"
#include "UltraPlayer.h"
#include "Huawei/UltraPlayerWebPage.h"
#include "auth.h"
#include "customer.h"
#include "Keyboard/Keyboard.h"
#include "Tr069.h"

#define KEY_UP_FOR_JVM // add for jvm key up message  2010.8.6


static const int EDS_REQUEST_TIMES = 1;
//static int last_is_upgrade_page = 0;
static int TakinEdsFlag = 0;
static char TakinErrorUrl[1028] = { 0 };
static int  EPGDomianBackUpFlag = 0;
static int  EPGDomianBackUpTwoFlag = 0;
static int HTTPFailNumbers = 0;
static char HTTPFailInfo[10][256] = {{0}, {0}}; // STATISTIC_INFO_FAID_NUM = 10, STATISTIC_INFO_FAID_SIZE = 256, here comes from tr069
static int HTTPReqNumbers = 0;

static void BrowserStatisticHttpFailInfoSet(char *url, int err_no);
static int s_Takin_Vkeyboard_Handle(YX_INPUTEVENT *yx_event);
static int s_TaKin_Get_Errorpage(int doctype, int ErrorCode, char *g_lastURL, char **ErrorUrl);
static void s_HttpErrorPrintf(char *FialedUrl, int ErrorCode);


extern "C" void TAKIN_browser_removeAllCookies(BrowserHandle handle);
#ifdef TVMS_OPEN
extern "C" void tvms_send_error_event(char *errorurl);
#endif
extern "C" int NativeHandlerSetState(int state);
extern "C" int TAKIN_browser_fireEvent(TAKIN_browser *, YX_INPUTEVENT *);


extern "C" void TAKIN_Proc_Key(void *handle, int msg, unsigned int p1, unsigned int p2)
{
    YX_INPUTEVENT event;
    int x = 0, y = 0, w = 0, h = 0;
#ifdef KEY_UP_FOR_JVM
    static YX_INPUTEVENT previousEvent;
#endif

    if(-100 == msg) {
#ifdef KEY_UP_FOR_JVM
        if(epgBrowserAgentGetJvmStatus() == 1) {
            previousEvent.eventkind = YX_EVENT_KEYUP;
            TAKIN_browser_fireEvent((TAKIN_browser *)(epgBrowserAgentGetHandle()), &previousEvent);
        }
#endif
        return ;
    }

    if(handle == NULL) {
        return;
    }
    event.eventkind = YX_EVENT_KEYDOWN;
    event.keyvalue = 0;
    event.unicode = 0;

	int changeVideoMode = 0;

    sysSettingGetInt("changevideomode", &changeVideoMode, 0);
    if(changeVideoMode) {
        bool OnTimeshift = false;
#if defined(HUBEI_HD) || (defined(Jiangsu) && defined(EC1308H))
        Hippo::SystemManager &sysManager = Hippo::systemManager();
        Hippo::UltraPlayer *player = NULL;

        player = sysManager.obtainMainPlayer();
#ifndef Jiangsu
        if(player) {
            if(STRM_STATE_IPTV != player->mCurrentStatus) {
                OnTimeshift = true;
            }
        }
        sysManager.releaseMainPlayer(player);
#endif
#endif

#if defined(Jiangsu) && defined(EC1308H)
        if (player && (EIS_IRKEY_FASTFORWARD == p1 || EIS_IRKEY_RIGHT == p1)
            && STRM_STATE_IPTV == player->mCurrentStatus
            && player->mDisplayMode)
            return;
#endif
        if( (EIS_IRKEY_BACK == p1 && !OnTimeshift)
            || EIS_IRKEY_STOP == p1
            || EIS_IRKEY_BTV == p1
            || EIS_IRKEY_VOD == p1
            || EIS_IRKEY_TVOD == p1
            || EIS_IRKEY_NVOD == p1
            || EIS_IRKEY_RED == p1
            || EIS_IRKEY_GREEN == p1
            || EIS_IRKEY_YELLOW == p1
            || EIS_IRKEY_BLUE == p1
#if defined(Liaoning) && defined(hi3716m)
            || EIS_IRKEY_INFO == p1
#endif
        ) {
            Hippo::UltraPlayer::setVideoClearFlag(1);
        } else if(p1 == EIS_IRKEY_SELECT
                  || p1 == EIS_IRKEY_NUM0
                  || p1 == EIS_IRKEY_NUM1
                  || p1 == EIS_IRKEY_NUM2
                  || p1 == EIS_IRKEY_NUM3
                  || p1 == EIS_IRKEY_NUM4
                  || p1 == EIS_IRKEY_NUM5
                  || p1 == EIS_IRKEY_NUM6
                  || p1 == EIS_IRKEY_NUM7
                  || p1 == EIS_IRKEY_NUM8
                  || p1 == EIS_IRKEY_NUM9) {
            int tX = 0, tY = 0, tWidth = 0, tHeight = 0;

            YX_SDK_codec_rect_get(&tX, &tY, &tWidth, &tHeight);
            if(tX != 0 && tY != 0) {
                Hippo::UltraPlayer::setVideoClearFlag(1);
            }
        }
    }
    switch(p1) {
    case EIS_IRKEY_NUM0: {
        event.vkey = VK_0;
        event.unicode = 0x30;
        break;
    }
    case EIS_IRKEY_NUM1: {
        event.vkey = VK_1;
        event.unicode = 0x31;
        break;
    }
    case EIS_IRKEY_NUM2: {
        event.vkey = VK_2;
        event.unicode = 0x32;
        break;
    }
    case EIS_IRKEY_NUM3: {
        event.vkey = VK_3;
        event.unicode = 0x33;
        break;
    }
    case EIS_IRKEY_NUM4: {
        event.vkey = VK_4;
        event.unicode = 0x34;
        break;
    }
    case EIS_IRKEY_NUM5: {
        event.vkey = VK_5;
        event.unicode = 0x35;
        break;
    }
    case EIS_IRKEY_NUM6: {
        event.vkey = VK_6;
        event.unicode = 0x36;
        break;
    }
    case EIS_IRKEY_NUM7: {
        event.vkey = VK_7;
        event.unicode = 0x37;
        break;
    }
    case EIS_IRKEY_NUM8: {
        event.vkey = VK_8;
        event.unicode = 0x38;
        break;
    }
    case EIS_IRKEY_NUM9: {
        event.vkey = VK_9;
        event.unicode = 0x39;
        break;
    }
    case EIS_IRKEY_UP: {
        event.vkey = VK_UP;
        break;
    }
    case EIS_IRKEY_DOWN: {
        event.vkey = VK_DOWN;
        break;
    }
    case EIS_IRKEY_LEFT: {
        event.vkey = VK_LEFT;
        break;
    }
    case EIS_IRKEY_RIGHT: {
        event.vkey = VK_RIGHT;
        break;
    }
    case EIS_IRKEY_SELECT: {
#ifdef ANDROID
        // Fixme:
        // ANDROID要求按确认键才弹出输入法。
        // 但是感觉这样做不太合适。
        // 应该是先让EPG把确认键给处理了然后再判断是否存在输入框来决定是否打开输入法

        if (TAKIN_browser_editingEnabled((TAKIN_browser *)(epgBrowserAgentGetHandle()), &x, &y, &w, &h)) {
            usleep(500000);
            Hippo::Rect r;
            r.set(x, y, x + w, y + h);
            Hippo::Keyboard::GetInstance()->Open(r);
            break;
        }
#endif
        event.vkey = VK_RETURN;
        break;
    }
    case EIS_IRKEY_VOLUME_UP: {
        event.vkey = 0x0103;
        break;
    }
    case EIS_IRKEY_VOLUME_DOWN: {
        event.vkey = 0x0104;
        break;
    }
    case EIS_IRKEY_VOLUME_MUTE: {
        event.vkey = 0x0105;
        break;
    }
    case EIS_IRKEY_AUDIO:
    case EIS_IRKEY_AUDIO_MODE: {
        event.vkey = 0x0106;
        break;
    }
    case EIS_IRKEY_BACK: {
        event.vkey = VK_BACK;
        break;
    }
    // case EIS_IRKEY_DELETE:
    case EIS_IRKEY_CLEAR: {
        if(epgBrowserAgentGetJvmStatus() == 1)
            return;
        event.vkey = VK_BACK;
        break;
    }
    case EIS_IRKEY_HELP: {
        event.vkey = 0x011C;
        break;
    }
    case EIS_IRKEY_VK_F10: {
        event.vkey = 0x011D;
        break;
    }
    case EIS_IRKEY_GOTO: //浙江定义为定位按键，功能同pause
    case EIS_IRKEY_PLAY:
    case EIS_IRKEY_PAUSE: {
        event.vkey = 0x0107;
        break;
    }
    case EIS_IRKEY_STOP: {
        event.vkey = 0x010E;
        break;
    }
    case EIS_IRKEY_FASTFORWARD: {
        event.vkey = 0x0108;
        break;
    }
    case EIS_IRKEY_REWIND: {
        event.vkey = 0x0109;
        break;
    }
    case EIS_IRKEY_PAGE_UP: {
        event.vkey = VK_PRIOR;
        break;
    }
    case EIS_IRKEY_PAGE_DOWN: {
        event.vkey = VK_NEXT;
        break;
    }
    //case EIS_IRKEY_FPANELPOWER:
    case EIS_IRKEY_POWER: {
        event.vkey = 0x0100;
        break;
    }
    case EIS_IRKEY_CHANNEL_UP: {
        event.vkey = 0x0101;
        break;
    }
    case EIS_IRKEY_CHANNEL_DOWN: {
        event.vkey = 0x0102;
        break;
    }
    case EIS_IRKEY_INTERX: {
        event.vkey = 0x010D;
        break;
    }
    case EIS_IRKEY_FAVORITE: {
        if (TAKIN_browser_editingEnabled((TAKIN_browser *)(epgBrowserAgentGetHandle()), &x, &y, &w, &h))
			return;
        event.unicode = 0x0119;
        break;
    }
    case EIS_IRKEY_POS:{
        if (TAKIN_browser_editingEnabled((TAKIN_browser *)(epgBrowserAgentGetHandle()), &x, &y, &w, &h)){
            Hippo::Rect r;
            r.set(x, y, x + w, y + h);
            Hippo::Keyboard::GetInstance()->Open(r);
		    break;
		}
		unsigned int cus = sys_msg_ir_cus_get();
		if ((cus == 0xDD22) || (cus == 0xfd02)) {
            event.vkey = 0x010F;
		} else {
            event.unicode = 0x010F;
		}
        break;
    }
    /*case EIS_IRKEY_BOOKMARK:{
                event.unicode = 0x011A;
                break;
    }
    case EIS_IRKEY_RECALL:{  //application
        //event.keyvalue = 0x0117;
        if(epgBrowserAgentGetJvmStatus() == 1)
            return;
        break;
    } */
    case EIS_IRKEY_CHANNEL_POS: {
        event.unicode = 0x011B;
        break;
    }
    case EIS_IRKEY_IME: {
#ifdef ANDROID
        event.vkey = 105;
        break;
#endif
        Hippo::Rect r;
        r.setEmpty();

#ifdef VIETTEL_HD
        unsigned int cus = sys_msg_ir_cus_get();
        event.vkey = 0x006A;
        if((0x4cb3 == cus) || (0x4db2 == cus)) { // 2012华为为新款遥控器，*和软键盘复用
            Hippo::Keyboard::GetInstance()->Open(r);
            break;
        }
        if (!Hippo::Keyboard::GetInstance()->Open(r))
            event.vkey = 0x006A;
        break;
#endif
#if defined(SHANGHAI_HD) || defined(SHANGHAI_SD)
        event.vkey = 0x0069;
#endif
#ifndef _lint
        if (!Hippo::Keyboard::GetInstance()->Open(r))
            event.vkey = 105;
#endif
        break;
    }
    case 0x300: { //media event
        event.vkey = p1;
        break;
    }
    case EIS_IRKEY_VK_F2: { //local set
//      a_Browser_open_config();
        break;
    }
    case EIS_IRKEY_TRACK: { //audio track
        event.vkey = 0x106;
        break;
    }
//  case EIS_IRKEY_PORTAL:
    case EIS_IRKEY_MENU: {
        if(epgBrowserAgentGetJvmStatus() == 1) {
            return;
        }
        event.vkey = 0x0110;
        break;
    }
    case EIS_IRKEY_RED: {
        if(epgBrowserAgentGetJvmStatus() == 1)
            return;
        event.vkey = 0x0113;
        break;
    }
    case EIS_IRKEY_GREEN: {
        if(epgBrowserAgentGetJvmStatus() == 1)
            return;
        event.vkey = 0x0114;
        break;
    }
    case EIS_IRKEY_YELLOW: {
        if(epgBrowserAgentGetJvmStatus() == 1)
            return;
        event.vkey = 0x0115;
        break;
    }
    case EIS_IRKEY_BLUE: {
        if(epgBrowserAgentGetJvmStatus() == 1)
            return;
        event.vkey = 0x0116;
        break;
    }
    case EIS_IRKEY_NVOD:
    case EIS_IRKEY_INFO: {
#if defined(Liaoning) && defined(hi3716m)
        event.vkey = 0x0116;
        break;
#endif
#if defined(Gansu)
        if(epgBrowserAgentGetJvmStatus() == 1)
            return;
        event.vkey = 0x0116;
        break;
#endif
        event.vkey = 0x458;
        break;
    }
    case EIS_IRKEY_INFO_BLUETOOTH: {
        event.vkey = 0x0601;
        break;
    }
    case EIS_IRKEY_SWITCH: {
        unsigned int cus = sys_msg_ir_cus_get();

        if(cus == 0xff00) { // 0xff00 越南用的那种遥控器,右下角switch键与del键复用.
            if(TAKIN_browser_editingEnabled((TAKIN_browser *)(epgBrowserAgentGetHandle()), &x, &y, &w, &h))
                event.vkey = VK_BACK;
            else
                event.vkey = p1;
            break;
        }
#if defined(SHANGHAI_HD) || defined(SHANGHAI_SD) || defined(VNPT_HD)
        event.vkey = 0x117;
#else
        event.vkey = 0x118;
#endif
        break;
    }
    case EIS_IRKEY_GREY: {
#if defined(SHANGHAI_HD) || defined(SHANGHAI_SD) || defined(VNPT_HD)
        event.vkey = 0x0117;
#else
        event.vkey = p1;
#endif
        break;
    }
    case EIS_IRKEY_COMM: {
        event.vkey = 0x0457;
        break;
    }
    case EIS_IRKEY_BTV: {
        event.vkey = 0x0454;
        break;
    }
    case EIS_IRKEY_VOD: {
        event.vkey = 0x0455;
        break;
    }
    case EIS_IRKEY_TVOD: {
        event.vkey = 0x0456;
        break;
    }
    case EIS_IRKEY_STAR: {
#ifdef ANDROID
        event.vkey = 105;
        break;
#endif

#ifdef VIETTEL_HD
        event.vkey = 105;
        break;
#endif
        Hippo::Rect r;
        r.setEmpty();
        unsigned int cus = sys_msg_ir_cus_get();

        event.vkey = 0x006A;
        if((0x4cb3 == cus) || (0x4db2 == cus)) { // 2012华为为新款遥控器，*和软键盘复用
            Hippo::Keyboard::GetInstance()->Open(r);
#ifdef HUAWEI_C20
            break;
#endif
            return;
        }
#if defined(SHANGHAI_HD) || defined(SHANGHAI_SD)
        Hippo::Keyboard::GetInstance()->Open(r);
#endif
        break;
    }
    case KEY_OPTIONS: {
        event.vkey = 0x0460;
        break;
    }
    case EIS_IRKEY_LAST_CHANNEL: {
        event.vkey = 0x045E;
        break;
    }
    default: {
        if(p1 == 0)
            event.vkey = 0; //init vkey
        else
            event.vkey = p1;
        break;
    }
    }

    LogUserOperDebug("HandleKey:0x%x, to Browser 0x%x\n", p1, (event.vkey != 0? event.vkey : event.unicode));

#ifdef ANDROID
    // Android版本要求不需要这个东西了。
#else
    if (Hippo::Keyboard::GetInstance()->HandleKey(&event)) {
        return;
    }
#endif
    TAKIN_browser_fireEvent((TAKIN_browser *)handle, &event);
#ifdef KEY_UP_FOR_JVM
    memcpy(&previousEvent, &event, sizeof(YX_INPUTEVENT));
#endif
    // 关输入法的动作拿到这里来。。逻辑上来讲应该是在浏览器处理完了方向键值之后再判断是否还在输入框内。
    // 放在之前的地方先判断再处理按键有点不妥，会造成从输入框离开的时候不能即时判断。
    if(!TAKIN_browser_editingEnabled((TAKIN_browser *)(epgBrowserAgentGetHandle()), &x, &y, &w, &h)) {
        Hippo::Keyboard::GetInstance()->Close();
    }

    return ;
}

void TAKIN_browser_setFocusByJS(const char *name)
{
    //TAKIN_browser_setSetting("TAKIN_FOCUSRING_COLOR", ringColor, 0);
}

char *TAKIN_browser_checkFrameData(char *url, char *data, int *len)
{
    //This function operate move to browser lib.
    return NULL;
}

int TAKIN_jse_invoke(const char *name, const char *param, char *buffer, int buffer_len)
{
    if(!param || !name || !buffer)
        return 0;
    LogUserOperDebug("TAKIN_jse_invoke: name=%s,param=%s,buffer=%s\n", name, param, buffer);
    return 0;
}

/***********************************************************
Tahin open webpage error and success deal
***********************************************************/
extern "C" void Takin_Set_EdsFlag(int flag)
{
    TakinEdsFlag = flag;
}

//hisi平台通过settimeofday把Timezone设置给了系统
//brcm平台系统使用都是UTC,设置时区函数yu_setTimezone仅在在sdk中以全局保存时区
//浏览器获取系统时间使用gettimeofday导致brcm无法正确获取时区
//由于频繁调用建议不要使用mutex
//目前都默认返回0,如测试正常可以返回正确时区
int TAKIN_browser_getTimezone(void)
{
   int TimeZone = 0;

   sysSettingGetInt("timezone", &TimeZone, 0);
#if (defined(hi3560e))
    return TimeZone;
#elif (defined(hi3716m))
#if defined(Huawei_v5)
    return TimeZone;
#else // Huawei_v5
    return 0;
#endif // Huawei_v5
#else
    return 0;
#endif
}

/***********************************************************
*	Input params:
*		int DocType
*   		1:DOC_TYPE_MAINFRAME		主页面
*   		2:DOC_TYPE_OVERLAYFRAME		overlay 页面，tvms
*		int ErrorCode
*			http standard error code
*		char *g_lastURL
*			The last open url
*		char *ErrorUrl
*			return error page url
*	Return
*		0:	success
*		1:	fail
***********************************************************/
static int s_TaKin_Get_Errorpage(int doctype, int ErrorCode, char *g_lastURL, char **ErrorUrl)
{
    char *p = NULL;

    LogUserOperDebug("Takin doctype(%d) errcode(%d)\n", doctype, ErrorCode);
    if(doctype == 2) {
        LogUserOperDebug("Takin this is a tvms ovelay timeout(%s)\n", g_lastURL);
#ifdef TVMS_OPEN
        tvms_send_error_event(g_lastURL);
#endif
        ErrorUrl = NULL;
        return 1;
    }

    BrowserStatisticHttpFailInfoSet(g_lastURL , ErrorCode);
    if(webchannelFlagGet()) {
        sleep(1);
		//app_a2_setEvttype(EVENT_MEDIA_ERROR);
	    //app_a2_setErrcode(ERR_RTSP_WEBCHANNEL_ERROR);
	    //"{\"type\":\"EVENT_MEDIA_ERROR\",\"instance_id\":%d,\"entry_id\":\"%s\",\"error_code\":%d,\"error_message\":\"%s\",\"media_code\":\"%s\"}", ERR_RTSP_WEBCHANNEL_ERROR,
        //sendMessageToKeyDispatcher(MessageType_Unknow, EVENT_MEDIA_ERROR, 0, 0);
        ErrorUrl = NULL;
        return 1;
    }

    // 加强调度
    // TODO: 拼接Url.
    std::string current = Hippo::Customer().AuthInfo().eds().Current();
    std::string::size_type pos = std::string(g_lastURL).find(current);
    if (pos != std::string::npos && strstr(g_lastURL, "Action=Login")) {
        Hippo::Customer().AuthInfo().eds().Next();
        std::string url = Hippo::Customer().AuthInfo().eds().Current();
        LogUserOperDebug("next url=[%s] \n\n", url.c_str());
        if (url.empty()) {
            sprintf(TakinErrorUrl, LOCAL_WEBPAGE_PATH_ERROR"%d", ErrorCode);
        } else {
            snprintf(TakinErrorUrl, sizeof(TakinErrorUrl), "%s", url.c_str());
        }
        *ErrorUrl = TakinErrorUrl;
        epgBrowserAgentSetCurrentUrl(TakinErrorUrl);
    } else if (strstr(g_lastURL, "action=PortalEPGURL")) {
        TAKIN_browser_removeAllCookies((TAKIN_browser *) epgBrowserAgentGetHandle());
        business().setEDSJoinFlag(1);

        Hippo::Customer().AuthInfo().eds().Reset();
        std::string url = Hippo::Customer().AuthInfo().eds().Current().c_str();

        snprintf(TakinErrorUrl, sizeof(TakinErrorUrl), "%s", url.c_str());
        *ErrorUrl = TakinErrorUrl;
        epgBrowserAgentSetCurrentUrl(TakinErrorUrl);
    } else {
        sprintf(TakinErrorUrl, LOCAL_WEBPAGE_PATH_ERROR"%d", ErrorCode);
        *ErrorUrl = TakinErrorUrl;
        epgBrowserAgentSetCurrentUrl(TakinErrorUrl);
        NativeHandlerSetState(10);

        // TODO: 为何要这样做？
        Hippo::SystemManager &sysManager = Hippo::systemManager();
        Hippo::UltraPlayer *player = NULL;
        player = sysManager.obtainMainPlayer();
        if(3 == doctype && player && 0 != player->mDisplayMode) {
            Hippo::epgBrowserAgent().openUrl(TakinErrorUrl);
            sysManager.releaseMainPlayer(player);
            return 1;
        }
        sysManager.releaseMainPlayer(player);
    }
    return 0;
}

#ifdef HUAWEI_C20
static int deal_httpsError(int doctype, int ErrorCode)
{
    if((ErrorCode == 35)
       || (ErrorCode == 51)
       || (ErrorCode == 53)
       || (ErrorCode == 54)
       || (ErrorCode == 58)
       || (ErrorCode == 59)
       || (ErrorCode == 60)
       || (ErrorCode == 64)
       || (ErrorCode == 66)
       || (ErrorCode == 77)
       || (ErrorCode == 82)
       || (ErrorCode == 83)) {
        char *caAddress = getCadownloadAddress();
        if(strlen(caAddress) > 0 && strlen(caAddress) < 1024) {  //WZW modified to fix pc-lint Error 58
            strcpy(TakinErrorUrl, caAddress);
        } else {
            printf("Error (strlen(caAddress) > 0 && strlen(caAddress <1024)\n");
        }
        return 0;
    }
    return -1;
}
#endif

/* 当第一个参数为TAKIN_LOADER_ERROR时，浏览器报告错误信息。返回0时，浏览器再次尝试打开buffer中的url。
   1、如果希望浏览器尝试新的url，则把url赋值到buffer，并返回0。
   2、如果浏览器传出的参数有问题，返回-1。
   3、如果中间件自身出错，或url长度buffer存不下，返回1，浏览器打开setting设置的默认错误地址。*/

extern "C" int TAKIN_porting_load_notification(TAKIN_SETTING_TYPE name, char *buffer, int buffer_len,int handler)
{
    int len = 0;

    if(name == TAKIN_LOADER_ERROR) {
        sendMessageToEPGBrowser(MessageType_WaitClock, 2, 0, 0);
        char *Url = NULL;
        char *ErrUrl = NULL;
        int docType = 0;
        int ErrorCode = 0;
#ifdef INCLUDE_DMR
        if (Hippo::defNativeHandler().getState() == Hippo::NativeHandler::Local) {
            Hippo::setIsepgfinish_huawei(0);
        }else{
            Hippo::setIsepgfinish(0);
        }
#endif

        if(strstr(buffer, "mainFrame") != NULL) { //mainFrame ErrorCode url
            docType = 1;
            ErrorCode = atoi(buffer + 10);
            ErrUrl = strchr((buffer + 11), ' ');
        } else if(strstr(buffer, "iFrame") != NULL || strstr(buffer, "iframe") != NULL) { //iFrame ErrorCode url
            docType = 3;
            ErrorCode = atoi(buffer + 7);
            ErrUrl = strchr((buffer + 8), ' ');
        } else if(strstr(buffer, "ajax") != NULL) { //ajax err
            docType = 2;
            ErrorCode = atoi(buffer + 4);
            ErrUrl = strchr((buffer + 5), ' ');
        } else {
            LogUserOperError("Error buffer [%s]\n", buffer);
            return -1;
        }

#ifdef HUAWEI_C20
        if(deal_httpsError(docType,  ErrorCode) == 0) { //deal https error
            return 1;
        }
#endif
        LogUserOperDebug("Open url failed, jump to timeout page, fail_url = %s, code = %d\n", buffer, ErrorCode);
#ifdef HUAWEI_C10
        s_HttpErrorPrintf(buffer, ErrorCode);
#endif

        s_TaKin_Get_Errorpage(docType, ErrorCode, buffer, &Url);

        memset(buffer, 0, buffer_len);
        if(ErrUrl != NULL) {
            ErrUrl += 1;
            HTTPReqNumbers++;
        }

        if(docType != 2) {
            if(Url != NULL)
                len = strlen(Url);
            if(len > buffer_len || len == 0) {
                sprintf(buffer, LOCAL_WEBPAGE_PATH_ERROR"%d", ErrorCode);
                Hippo::SystemManager &sysManager = Hippo::systemManager();
                Hippo::UltraPlayer *player = NULL;

                player = sysManager.obtainMainPlayer();
                if(3 == docType && player && 0 != player->mDisplayMode) {
                    Hippo::epgBrowserAgent().openUrl(buffer);
                    sysManager.releaseMainPlayer(player);
                    buffer = NULL;
                    return -1;
                }
                sysManager.releaseMainPlayer(player);
                return 0;
            }
            buffer_len = len;
            strcpy(buffer, Url);
        }
    } else if(name == TAKIN_OPEN_URL_BEGIN) {
        LogUserOperDebug("Open url(%s) begin!\n", buffer);
#ifdef INCLUDE_DMR
        if (Hippo::defNativeHandler().getState() == Hippo::NativeHandler::Local) {
            Hippo::setIsepgfinish_huawei(-1);
        }else{
            Hippo::setIsepgfinish(-1);
        }
#endif
        sendMessageToEPGBrowser(MessageType_WaitClock, 0, 0, 0);
        HTTPReqNumbers++;
    } else if(name == TAKIN_OPEN_URL_END) {
        LogUserOperDebug("Open url(%s) end!\n", buffer);
        sendMessageToEPGBrowser(MessageType_WaitClock, 1, 0, 0);
#ifdef INCLUDE_DMR
        if (Hippo::defNativeHandler().getState() == Hippo::NativeHandler::Local) {
            Hippo::setIsepgfinish(1);
        }else{
            Hippo::setIsepgfinish_huawei(1);
        }
#endif
    HTTPReqNumbers++;
    }
    return 0;
}

static void s_HttpErrorPrintf(char *FialedUrl, int ErrorCode)
{
    switch(ErrorCode) {
    case 400: {
        LogUserOperError("Error code:HTTP %d, Description:Bad, Http Error URL:%s\n", ErrorCode, FialedUrl);
        break;
    }
    case 401: {
        LogUserOperError("Error code:HTTP %d, Description:Unauthorized, Http Error URL:%s\n", ErrorCode, FialedUrl);
        break;
    }
    case 402: {
        LogUserOperError("Error code:HTTP %d, Description:Payment Required, Http Error URL:%s\n", ErrorCode, FialedUrl);
        break;
    }
    case 403: {
        LogUserOperError("Error code:HTTP %d, Description:Forbidden, Http Error URL:%s\n", ErrorCode, FialedUrl);
        break;
    }
    case 404: {
        LogUserOperError("Error code:HTTP %d, Description:Not Found, Http Error URL:%s\n", ErrorCode, FialedUrl);
        break;
    }
    case 405: {
        LogUserOperError("Error code:HTTP %d, Description:Method Not Allowed, Http Error URL:%s\n", ErrorCode, FialedUrl);
        break;
    }
    case 406: {
        LogUserOperError("Error code:HTTP %d, Description:Not Acceptable, Http Error URL:%s\n", ErrorCode, FialedUrl);
        break;
    }
    case 407: {
        LogUserOperError("Error code:HTTP %d, Description:Proxy Authentication Required, Http Error URL:%s\n", ErrorCode, FialedUrl);
        break;
    }
    case 408: {
        LogUserOperError("Error code:HTTP %d, Description:Request Timeout, Http Error URL:%s\n", ErrorCode, FialedUrl);
        break;
    }
    case 410: {
        LogUserOperError("Error code:HTTP %d, Description:Gone, Http Error URL:%s\n", ErrorCode, FialedUrl);
        break;
    }
    case 411: {
        LogUserOperError("Error code:HTTP %d, Description:Length Required, Http Error URL:%s\n", ErrorCode, FialedUrl);
        break;
    }
    case 412: {
        LogUserOperError("Error code:HTTP %d, Description:Precondition Failed, Http Error URL:%s\n", ErrorCode, FialedUrl);
        break;
    }
    case 413: {
        LogUserOperError("Error code:HTTP %d, Description:Request Entity Too Large, Http Error URL:%s\n", ErrorCode, FialedUrl);
        break;
    }
    case 414: {
        LogUserOperError("Error code:HTTP %d, Description:Request-URI Too Long, Http Error URL:%s\n", ErrorCode, FialedUrl);
        break;
    }
    case 415: {
        LogUserOperError("Error code:HTTP %d, Description:Unsupported Media Type, Http Error URL:%s\n", ErrorCode, FialedUrl);
        break;
    }
    case 416: {
        LogUserOperError("Error code:HTTP %d, Description:Requested Range Not Satisfiable, Http Error URL:%s\n", ErrorCode, FialedUrl);
        break;
    }
    case 417: {
        LogUserOperError("Error code:HTTP %d, Description:Expectation Failed, Http Error URL:%s\n", ErrorCode, FialedUrl);
        break;
    }
    case 500: {
        LogUserOperError("Error code:HTTP %d, Description:Internal Server Error, Http Error URL:%s\n", ErrorCode, FialedUrl);
        break;
    }
    case 501: {
        LogUserOperError("Error code:HTTP %d, Description:Not Implemented, Http Error URL:%s\n", ErrorCode, FialedUrl);
        break;
    }
    case 502: {
        LogUserOperError("Error code:HTTP %d, Description:Bad Gateway, Http Error URL:%s\n", ErrorCode, FialedUrl);
        break;
    }
    case 503: {
        LogUserOperError("Error code:HTTP %d, Description:Service Unavailable, Http Error URL:%s\n", ErrorCode, FialedUrl);
        break;
    }
    case 504: {
        LogUserOperError("Error code:HTTP %d, Description:Gateway Timeout, Http Error URL:%s\n", ErrorCode, FialedUrl);
        break;
    }
    case 505: {
        LogUserOperError("Error code:HTTP %d, Description:HTTP Version Not Supported, Http Error URL:%s\n", ErrorCode, FialedUrl);
        break;
    }
    default: {
        LogUserOperError("Error code:HTTP 10071，Description:EPG page access without response timeout. Http Timeout URL:%s\n", FialedUrl);
        break;
    }
        }
#ifdef INCLUDE_TR069
        if(ErrorCode >= 200) //takin 给的非标准错误码都小于200
		tr069_api_setValue("ErrorCode", "", ErrorCode);
        else
		tr069_api_setValue("ErrorCode", "", 10071);
#if defined(Sichuan)
        app_report_epg_access_alarm( );
#endif
#endif
       return;
}

#if defined(BROWSER_INDEPENDENCE) //not used
int TAKIN_browser_editingEnabled(BrowserHandle handle, int *x, int *y, int *w, int *h)
{
    return -1;
}

#endif

static int stringtime(char* buf)
{
    time_t sec;
    struct tm t;

    if(buf == NULL)
        ERR_OUT("buf is null\n");

    sec = mid_time();
    gmtime_r(&sec, &t);

    sprintf(buf, "%02d%02d%02d%02d%02d%02d",
            (t.tm_year + 1900) % 100,
            t.tm_mon + 1,
            t.tm_mday,
            t.tm_hour,
            t.tm_min,
            t.tm_sec);
    buf[14] = '\0';

    return 12;
Err:
    return 0;
}

void BrowserStatisticHttpFailInfoSet(char *url, int err_no)
{
    int tIndex = 0;
    int len = 0;
    char *buf = NULL;

    if(url) {
        tIndex = HTTPFailNumbers % 10; //STATISTIC_INFO_FAID_NUM = 10
        buf = HTTPFailInfo[tIndex];

        len = stringtime(buf);
        snprintf(buf + len, 256 - 1 - len, ",%s,%d", url, err_no);
        buf[256 - 1] = 0;

        HTTPFailNumbers++;
    }
    return;
}

extern "C"
int BrowserStatisticHTTPFailNumbersGet()
{
    return HTTPFailNumbers;
}

extern "C"
void BrowserStatisticHTTPFailInfoGet(char (*buf)[256], int size)
{
    if (NULL == buf)
        return;
    memcpy(buf, HTTPFailInfo, size);
}

extern "C"
int BrowserStatisticHTTPReqNumbersGet()
{
    return HTTPReqNumbers;
}
