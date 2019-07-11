
#include "Assertions.h"
#include "NativeHandlerPublic.h"
#include "NativeHandlerCustomer.h"
#include "NativeHandlerAssertions.h"

#include "Message.h"
#include "MessageTypes.h"
#include "MessageValueSystem.h"
#include "MessageValueNetwork.h"
#include "MessageValueDebug.h"
#include "MonitorConfig.h"
#include "mgmtModule.h"

#include "BrowserAgent.h"
#include "BrowserEventQueue.h"
#include "PowerOffDialog.h"
#include "KeyTableParser.h"
#include "KeyDispatcher.h"
#include "UpgradeManager.h"
#include "UltraPlayer.h"
#include "UltraPlayerWebPage.h"
#include "TerminalControl.h"

#include "UpgradeManager.h"
#include "StringData.h"

#include "SystemManager.h"
#include "StandardScreen.h"
#include "Setting/SettingModuleType.h"

#include "customer.h"
#include "auth.h"
#include "Tr069.h"

#include "libzebra.h"

#include "config.h"
#include "config/webpageConfig.h"

#include "AppSetting.h"
#include "SysSetting.h"
#include "app/app_reminderlist.h"
#include "app/app_heartbit.h"

#include "app/app_epg_para.h"

#include "mid_fpanel.h"


#include "sys_msg.h"
#include "app_sys.h"
#include "sys_key_deal.h"
#include "sys_basic_macro.h"

#include "disk/disk_info.h"
#include "TAKIN_browser.h"
#if defined(PAY_SHELL)
#include "Hippo_CTCUBankDevice.h"
#include "PayShell.h"
#endif

#if defined(SQM_INCLUDED)
#include "sqm_port.h"
#endif
#include "IPTVMiddleware.h"

extern "C" {
#if defined(ANDROID)
int ygp_layer_setDisplaySurface(int);
#endif

void Jvm_Main_Close(void);
#ifdef INCLUDE_DLNA
void mid_dlna_GetEvent(int type);
#endif
}

#include <utmp.h> //ssh users
#ifdef ANDROID
extern "C" void endutent(void);
#endif
#ifndef ANDROID
extern "C" int monitorUnInsertUDisk();
#endif

static int mBrowserFocusOnEditBox = 0;
static int sBrowserTimer = 0;

namespace Hippo {

Dialog *NativeHandlerPublic::mDialog = 0;
Dialog *NativeHandlerPublic::mProgressBarDialog = 0;

NativeHandlerPublic::NativeHandlerPublic()
    : m_isConnect(true)
    , mErrorCodeOfConnect(0)
    , mNICStatus(NICStatus_on)
    , mWaitUpgradeUrl(false)
    , mPPPoEAccountIsModify(false)
    , mPPPoEPwdIsModify(false)
    , mDoNetworkMask(0)
{
}

NativeHandlerPublic::~NativeHandlerPublic()
{
}

bool
NativeHandlerPublic::handleMessage(Message *msg)
{
    if(MessageType_Repaint != msg->what) {
        NATIVEHANDLER_LOG("message[0x%x]/[0x%x], type:[0x%x]\n", msg->arg1, msg->arg2, msg->what);
    }

    if(msg->what == MessageType_KeyDown) {
        if(mDialog) {  //5E=
            bool status = mDialog->handleMessage(msg);
            if(status)
                return status;
            else if(msg->arg1 == EIS_IRKEY_SELECT) {
                delete mDialog;
                mDialog = 0;
                return true;
            }
#ifdef ENABLE_VISUALINFO
            else if (EIS_IRKEY_POWER == msg->arg1 || EIS_IRKEY_VK_F10 == msg->arg1) {
                delete mDialog;
                mDialog = 0;
            }
#endif
        }


        switch(msg->arg1) {
        case EIS_IRKEY_POWER: {
            int flag = mid_fpanel_standby_get();
#if defined(Jiangsu) || defined(Anhui)
            if (epgBrowserAgentGetJvmStatus())
                return true;
#endif
            NATIVEHANDLER_LOG("IR power key standby status(%d)\n", flag);
            if (flag == 1) {
                return doPowerOn();
            } else {
                if (msg->arg2 != 1) {
                    return onPowerOff();
                }
            }
            return true;
        }
        case EIS_IRKEY_FPANELPOWER: {
			NATIVEHANDLER_LOG("Fpanel power key standby status(%d)\n", mid_fpanel_standby_get());
            if(mid_fpanel_standby_get() == 1)
                return doPowerOn();
            else
                return doPowerOff();
        }
        case EIS_IRKEY_MENU: {
            return doMenu();
        }
        case EIS_IRKEY_TVOD:
        case EIS_IRKEY_BTV:
        case EIS_IRKEY_VOD: {
            return doShortCut(msg);
        }
        case EIS_IRKEY_VK_F10: {
            return doConfig(msg);
        }
        case EIS_IRKEY_VK_F11: {
            return doOpenLoginPage(msg);
        }
        case EIS_IRKEY_IME: {
            return doIME(msg);
        }
        case EIS_IRKEY_CLEAR: {
            return doClear(msg);
        }
        case EIS_IRKEY_SWITCH: {
            return doSwitch();
        }
        case EIS_IRKEY_DESKTOP: {
            return doDeskTop();
        }
#ifdef Liaoning
        case EIS_IRKEY_BACK:{
            return doBack();
        }
#endif
        default: {
            return false;
        }
        }
    } else if(msg->what == MessageType_Play) {
        return true;
    } else if (msg->what == MessageType_SaveToFlash) {
        switch (msg->arg1) {
        case eSettingModule_Sys:
            break;
        case eSettingModule_App:
            break;
        case eSettingModule_Dlna:
            //TODO
            break;
        case eSettingModule_Dvb:
            //TODO
            break;
        case eSettingModule_Network:
            //TODO
            break;
        case eSettingModule_Player:
            //TODO
            break;
        case eSettingModule_Tr069:
            //TODO
            break;
        case eSettingModule_Upgrade:
            //TODO
            break;
        default:
                ;
        }
    }
    else if (msg->what == MessageType_Android) {
        switch (msg->arg1) {
            case eAndroidOpenUrl:
                doAndroidOpenUrl(msg);
                msg->obj->safeUnref();
                return true;
            case eAndroidSetSurface:
                return doAndroidSetSurface(msg->arg2);
            case eAndroidStop:
                return doAndroidStop();
            case eAndroidRestart:
                return doAndroidRestart();
            default:
                return true;
        }
    }
#ifdef INCLUDE_TR069
	else if(msg->what == MessageType_Tr069) {
        switch(msg->arg1) {
        case TR069_MSG_START: {
            return doTr069Start();
        }
        case TR069_MSG_STOP: {
            return doTr069Stop();
        }
        case TR069_REQUEST_REBOOT: {
            return doRequestReboot();
        }
        case TR069_UPGRADE_BOOT_REQUEST: {
            return doUpgradeTr069BootRequest();
        }
        case TR069_UPGRADE_REQUEST: {
            return doUpgradeTr069Request();
        }
        case TR069_NET_CONNECT_OK: {
            return doTr069NetConnectOk();
        }
        case TR069_NET_CONNECT_ERROR: {
            return doTr069NetConnectError();
        }
        case TR069_NET_SPEED_TEST:{
#ifdef NW_DIAGNOSE_OPEN
        systemManager().destoryAllPlayer();
        std::string url = Customer().AuthInfo().AvailableEpgUrlWithoutPath();
        if (!url.empty()) {
            url += std::string("EPG/jsp/Speed.jsp");
            epgBrowserAgentOpenUrl(url.c_str());
        } else {
            LogRunOperError("open network speed test url error!\n");
        }
#endif
            return true;
        }
        }
    }
#endif
	else if(msg->what == MessageType_Repaint) {
#if defined(hi3560e)
        systemManager().mixer().display();
#else
        systemManager().mixer().refresh(msg->arg1);
#endif
        return true;
    } else if (MessageType_Network == msg->what) {
        //TODO not finished, don't fix me.
        char obj[URL_LEN] = { 0 };
        if (msg->obj)
            strncpy(obj, (static_cast<StringData*>(msg->obj)->getData()), URL_LEN);
        switch (msg->arg1) {
        case MV_Network_ProtocolUp:
            return doNetworkProtocolUp(msg->arg2, obj);
        case MV_Network_ProtocolDown:
            return doNetworkProtocolDown(msg->arg2, obj);
        case MV_Network_UnlinkMonitor:
            return doNetworkUnlinkMonitor(msg->arg2, obj);
        case MV_Network_WifiJoinFail:
            return doNetworkWifiJoinFail(msg->arg2, obj);
        case MV_Network_WifiJoinSuccess:
            return doNetworkWifiJoinSuccess(msg->arg2, obj);
        case MV_Network_WifiCheckSignal:
            return doNetworkWifiCheckSignal(msg->arg2, obj);
        case MV_Network_ProtocolConflict:
            return doNetworkProtocolConflict(msg->arg2, obj);
        case MV_Network_ConnectOk:
            return doNetworkConnectOk(msg->arg2, obj);
        case MV_Network_DhcpError:
            return doNetworkDhcpError(msg->arg2, obj);
        case MV_Network_PppoeError:
            return doNetworkPppoeError(msg->arg2, obj);
        default:
            break;
        }
    } else if(msg->what == MessageType_Upgrade) {
        if (msg->arg1 == MV_System_OpenUpgradePage)
            return doOpenUpgradePage();
        if (msg->arg1 ==  USB_UPGRADE_OK) {
            LogUserOperDebug("run here! clean Browser Cache!\n");
            TAKIN_browser_cleanCache();
            return doUSBUpgrade(msg);
        }
        if (msg->arg1 == MV_System_Propmt)
            return onUpgrade();

        return false;

    } else if (msg->what == MessageType_Debug) {
        switch (msg->arg1) {
        case MV_Debug_WatchLoggedUsers:
            return onWatchLoggedUsers(msg->arg2);
        case MV_Debug_ShowBrowser:
        case MV_Debug_HideBrowser:
            return onDebug(msg->arg1);
        default:
            return false;
        }
#if defined(Huawei_v5)
    } else if (msg->what == MessageType_HDMI) {
        LogSysOperDebug("MessageType_HDMI :0x%02x/%d\n", msg->arg1, msg->arg1);
        switch (msg->arg1) {
        case YX_HDMI_EVENT_HOTPLUG: //0x11
            return doHdmiInsert(msg->arg2);
        case YX_HDMI_EVENT_NO_PLUG:
            return doHdmiUnInsert(msg->arg2);
        case YX_HDMI_EVENT_EDID_FAIL:
            return false; //TODO
        case YX_HDMI_EVENT_HDCP_FAIL:
            return false; //TODO
        case YX_HDMI_EVENT_HDCP_SUCCESS:
            return false; //TODO
        case YX_HDMI_EVENT_RSEN_CONNECT:
            return doHdmiConnect(msg->arg2);
        case YX_HDMI_EVENT_RSEN_DISCONNECT:
            return doHdmiDisconnect(msg->arg2);
        case YX_HDMI_EVENT_HDCP_USERSETTING: //0x18
            return false; //TODO
        default:
            return false;
        }
#endif
#if defined(PAY_SHELL)
    }else if (msg->what == MessageType_PayShell) {
        LogSysOperDebug("MessageType_PayShell : %d<->%d\n", msg->arg1, msg->arg1);
        switch (msg->arg1) {
        case PAY_SHELL_EVENT_ERROR:
            return doPayShellError(msg->arg2);
        default:
            Hippo::CTCUBankDevice::onEvent(msg->arg1, "", pay_shell_getHandle());
            return true;
        }
#endif
    }else if (msg->what == MessageType_DLNA) {
       //printf("dinglei  sendMessageToNativeHandler(MessageType_KeyDown, EIS_IRKEY_POWER, 0, 0);\n");
	   return onDLNADialog();
    }

    switch(msg->arg1) {
    case MV_System_OpenMenuPage: {
        return doOpenMenuPage();
    }
    case MV_System_OpenStandbyPage: {
        return doOpenStandbyPage();
    }
    case MV_System_OpenTransparentPage: {
        return doOpenTransparentPage();
    }
    case MV_System_OpenBootPage: {
        return doOpenBootPage();
    }
    case MV_System_OpenConfigPage: {
        return doOpenConfigPage();
    }
    case HEART_USER_INVALID:
    case MV_System_OpenTimeoutPage: {
        return doOpenTimeoutPage();
    }
    case MV_System_OpenErrorPage: {
        return doOpenErrorPage();
    }
    case MV_System_OpenUnzippingPage: {
        return doOpenUnzipRunningPage();
    }
    case MV_System_OpenUnzipErrorPage: {
        return doOpenUnzipErrorPage();
    }

#if defined(Gansu)
    case MV_System_UnzipErr2Restart:{
        return doUdiskUnzipErrReboot();
    }
#endif

    case MV_System_OpenDVBPage: {
        return doOpenDVBPage();
    }
    case MV_System_ModifyPPPoEAccount: {
        return doModifyPPPoEAccount();
    }
    case MV_System_ModifyPPPoEPwd: {
        return doModifyPPPoEPwd();
    }
    case MV_System_OpenCheckPPPoEAccountPage: {
        return doOpenCheckPPPoEAccountPage();
    }
    case EIS_IRKEY_SHOWSTANDBY: {
        return onPowerOff();
    }
    case KEYTABLE_UPDATE: {         //~GO%,m'fC
        return doUpdateGolbalKeyTable();
    }
    case HEART_BIT_RUN: {           //G
        return doHeartBit();
    }
    case CHANNEL_PPV_OK: {          //PPV z0W
        return doPPVChannelSync();
    }
    case CHANNEL_PPV_ERROR: {
        return doPPVChannelError();
    }
    case CHANNEL_LIST_ERROR: {
        return doChannelListError();
    }
    case LOCAL_TS_START:
    case LOCAL_TS_DISK_FULL:
    case LOCAL_TS_DATA_DAMAGE:
    case LOCAL_TS_PVR_CONFLICT:
    case LOCAL_TS_DISK_ERROR:
    case LOCAL_TS_MSG_ERROR:
    case LOCAL_TS_DISK_DETACHED: {
        return doLocalTS(msg);
    }
    case REMINDER_LIST_OK: {        // Reminder list
        return doReminderListSync();
    }
    case NTP_DNS_SERVER_NOTFOUND: {
        LogRunOperDebug("NTP DNS server not found\n");
        return doDNSServerNotfound();
    }
    case NTP_DNS_RESOLVE_ERROR: {
        LogRunOperDebug("NTP DNS resolve error\n");
        return doDNSResolveError();
    }
    case NTP_SYNC_OK: {
        LogRunOperDebug("NTP sync ok\n");
        return doNtpSyncOk();
    }
    case NTP_SYNC_ERROR: {
        LogRunOperDebug("NTP sync error\n");
        return doNtpSyncError();
    }
    case RESTART_AUTH: {
        return doEpgAuth();
    }
    case EIS_IRKEY_USB_INSERT: {
        return doUSBInsert(msg);
    }
    case EIS_IRKEY_USB_UNINSERT: {
        return doUSBUninsert(msg);
    }
    case USB_CONFIG_OK: {
        return doUSBConfig(msg);
    }
    default: {
        return false;
    }
    }
    return true;
}

bool
NativeHandlerPublic::onPowerOff()
{
    return doPowerOff();
}

bool
NativeHandlerPublic::doPowerOff()
{
    NATIVEHANDLER_LOG("NativeHandlerPublic::doPowerOff \n");
    return false;
}

bool
NativeHandlerPublic::onUpgrade()
{
    return true;
}

bool
NativeHandlerPublic::onWatchLoggedUsers(int lusrcnt)
{
    //NATIVEHANDLER_LOG("last time user count : %d\n", lusrcnt);
    int usrs = 0;
    char path[256] = { 0 };
    Message* msg = 0;
    #ifdef ANDROID
    // android ÔÚAPKÊµÏÖ
    #else
    if (getMonitorState()) {
        //usrs = getMonitorUsersCount();
        // getMonitorUsersCountÓÐÎÊÌâ£¬
        usrs = 1;
    }
    #endif
    if (getSSHState()) {
        struct utmp* u = 0;
        setutent(); //For 3560E
        while ((u = getutent())) {
            if(u->ut_type == USER_PROCESS && strlen(u->ut_host)) {
                //For 3560E
                snprintf(path, 255, "/dev/%s", u->ut_line);
                if (access(path, F_OK))
                    continue;
                usrs++;
                //NO<1> LoginUser:[huawei] LineTerm[pts/1] Host:[113.1.1.145]
                NATIVEHANDLER_LOG("NO<%d> LoginUser:[%s] LineTerm[%s] Host:[%s]\n", usrs, u->ut_user, u->ut_line, u->ut_host);
            }
        }
        endutent();
    }

    if (usrs > 0) {
        if (!lusrcnt) {
            msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptDebug, 1);
            epgBrowserAgent().sendMessage(msg);
        }
    } else {
        if (lusrcnt) {
            msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptDebug, 0);
            epgBrowserAgent().sendMessage(msg);
        }
    }
    msg = defNativeHandler().obtainMessage(MessageType_Debug, MV_Debug_WatchLoggedUsers, usrs, 0);
    defNativeHandler().sendMessageDelayed(msg, 3000);
    return true;
}

bool
NativeHandlerPublic::onDebug(int command)
{
    switch (command) {
        case MV_Debug_ShowBrowser:
        systemManager().mixer().middleLayer()->show();
        break;
        case MV_Debug_HideBrowser:
        systemManager().mixer().middleLayer()->hide();
        break;
    }
    return true;
}

bool
NativeHandlerPublic::doShortCut(Message *msg)
{
    char mid_url[URL_MAX_LEN + 4] = {0};
    KeyDispatcherPolicy *keyPolocy = NULL;
    keyPolocy = keyDispatcher().getPolicy(msg->arg1);
    //app_ctc_FuncKeyUrl_by_KeyValue(EIS_IRKEY_MENU, mid_url);

    if(keyPolocy && keyPolocy->mKeyPolicy == KeyDispatcherPolicy::OpenUrl){
        NATIVEHANDLER_LOG("NativeHandlerPublicC20::doShortCut get menu url(%s)\n", keyPolocy->mKeyUrl.c_str());
        strncpy(mid_url, keyPolocy->mKeyUrl.c_str(), URL_MAX_LEN);
    }
    NATIVEHANDLER_LOG("NativeHandlerPublicC20::doShortCut get global url(%s)!\n", mid_url);

    if (0 != mid_url[0]) {
    	defNativeHandler().setState(NativeHandler::Running);
        epgBrowserAgent().openUrl(mid_url);
        return true;
    }
    else
        return false;
}

bool
NativeHandlerPublic::doLocalPage(Message *msg)
{
    Message *epgmsg = epgBrowserAgent().obtainMessage(MessageType_KeyDown, msg->arg2, 0);
    epgBrowserAgent().sendMessage(epgmsg); // ITMS_OK
    return true;
}

bool
NativeHandlerPublic::doOpenMenuPage()
{
    systemManager().destoryAllPlayer();

    // Õâ¸öº¯Êý×÷Îª´¥·¢EDSµ÷¶ÈµÄ½Ó¿Ú£¬Ó¦¸Ãµ±ÈÊ²»ÈÃµØÖ±½Ó»Øµ½eds0¡£
    Customer().AuthInfo().eds().Reset();

    std::string url = Customer().AuthInfo().eds().Current();
    if (url.empty()) {
        url = LOCAL_WEBPAGE_PATH_ERROR;
        TR069_API_SETVALUE("ErrorCode", "", 10071);
        defNativeHandler().setState(NativeHandler::Error);
    } else {
        defNativeHandler().setState(NativeHandler::Running);
    }

    webchannelFlagSet(0);
    NATIVEHANDLER_LOG("URL=[%s]\n", url.c_str());
    epgBrowserAgent().openUrl(url.c_str());
    sys_appmode_set(APPMODE_IPTV);
    return true;
}

bool
NativeHandlerPublic::doOpenTransparentPage()
{
#ifdef ANDROID
    std::string mid_url = "/system/net_manager" LOCAL_WEBPAGE_PATH_TRANSPARENT;
#else
    std::string mid_url = LOCAL_WEBPAGE_PATH_PREFIX LOCAL_WEBPAGE_PATH_TRANSPARENT;
#endif

    epgBrowserAgent().openUrl(mid_url.c_str());

    return true;
}

bool
NativeHandlerPublic::doOpenBootPage()
{
    std::string mid_url = LOCAL_WEBPAGE_PATH_PREFIX LOCAL_WEBPAGE_PATH_BOOT;

    NATIVEHANDLER_LOG("NativeHandlerPublic::doOpenBootPage url(%s)\n", mid_url.c_str());

    epgBrowserAgent().closeAllConnection();
    defNativeHandler().setState(NativeHandler::Boot);
    epgBrowserAgent().openUrl(mid_url.c_str());

    return true;
}

bool
NativeHandlerPublic::doOpenConfigPage()
{
    std::string mid_url = "";

    NATIVEHANDLER_LOG("doOpenConfigPage\n");
#if defined(C30)
    mid_url = LOCAL_WEBPAGE_PATH_PREFIX"/common/index.html";
#else
#if defined(NW_DIAGNOSE_OPEN) || defined(NET_DIAG_HWC10_OPEN)
    mid_url = LOCAL_WEBPAGE_PATH_PREFIX"/web_select.html";
#else
    mid_url = LOCAL_WEBPAGE_PATH_PREFIX LOCAL_WEBPAGE_PATH_CONFIG;
#endif
#endif
#if defined(HUAWEI_C10)
    defNativeHandler().setState(NativeHandler::Config);
#endif
    systemManager().destoryAllPlayer();

    epgBrowserAgent().openUrl(mid_url.c_str());
    return true;
}

bool
NativeHandlerPublic::doOpenLoginPage(Message *msg)
{
#if defined(SHANGHAI_HD) || defined(SHANGHAI_SD) || defined(JIANGSU_HD)
    std::string mid_url = "";
    mid_url = LOCAL_WEBPAGE_PATH_PREFIX"/login.html";
    defNativeHandler().setState(NativeHandler::Config);
    epgBrowserAgent().openUrl(mid_url.c_str());
#endif
    return true;
}

bool
NativeHandlerPublic::doOpenErrorPage()
{
    std::string mid_url = LOCAL_WEBPAGE_PATH_ERROR;

    if(NativeHandler::Config == state())
        return true;

    Hippo::defNativeHandler().setState(Hippo::NativeHandler::Error);
    defNativeHandler().setState(NativeHandler::Error);
    epgBrowserAgent().openUrl(mid_url.c_str());
    TR069_API_SETVALUE("ErrorCode", "", 10071);


    return true;
}

bool
NativeHandlerPublic::doOpenUnzipRunningPage()
{
    std::string mid_url = "";
    if (defNativeHandler().getState() == NativeHandler::Recovery) {
        Hippo::epgBrowserAgent().sendEmptyMessageDelayed(MessageType_Timer, 10);
        sBrowserTimer = 1;
    }
    mid_url = LOCAL_WEBPAGE_PATH_PREFIX LOCAL_WEBPAGE_PATH_UNZIP_RUNNING;
    NATIVEHANDLER_LOG("NativeHandlerPublic::doOpenUnzipRunningPage url(%s)\n", mid_url.c_str());
    defNativeHandler().setState(NativeHandler::UConfig);
    epgBrowserAgent().openUrl(mid_url.c_str());

    return true;
}

bool
NativeHandlerPublic::doOpenUnzipErrorPage()
{
    std::string mid_url = "";
    mid_url = LOCAL_WEBPAGE_PATH_PREFIX LOCAL_WEBPAGE_PATH_UNZIP_ERROR;

    epgBrowserAgent().openUrl(mid_url.c_str());

    return true;
}

#if defined(Gansu)
bool
NativeHandlerPublic::doUdiskUnzipErrReboot()
{
    mid_fpanel_reboot();
    return true;
}
#endif

bool
NativeHandlerPublic::doOpenDVBPage()
{
    std::string mid_url = "";

#ifdef INCLUDE_DVBS
    mid_url = LOCAL_WEBPAGE_PATH_PREFIX"/template1/play.html";
    sys_appmode_set(APPMODE_DVBS);

    Message *msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptClearNetwork, 0);
    epgBrowserAgent().sendMessage(msg);
#endif
    defNativeHandler().setState(NativeHandler::Running);
    epgBrowserAgent().openUrl(mid_url.c_str());

    return true;
}

bool
NativeHandlerPublic::doModifyPPPoEAccount()
{
    if(mPPPoEAccountIsModify == false) {
        mPPPoEAccountIsModify = true;
        return true;
    }

    return false;
}

bool
NativeHandlerPublic::doModifyPPPoEPwd()
{
    if(mPPPoEPwdIsModify == false) {
        mPPPoEPwdIsModify = true;
        return true;
    }

    return false;
}

bool
NativeHandlerPublic::doOpenCheckPPPoEAccountPage()
{
    std::string mid_url = LOCAL_WEBPAGE_PATH_PREFIX LOCAL_WEBPAGE_PATH_CHECK_PPPOEACCOUNT;

    if(mPPPoEAccountIsModify == true && mPPPoEPwdIsModify == true) {
        defNativeHandler().setState(NativeHandler::Config);
        epgBrowserAgent().openUrl(mid_url.c_str());
    }

    return true;
}

// heart bit message
bool
NativeHandlerPublic::doHeartBit()
{
    httpHeartBit(0);
    return true;
}

// PPV message
bool
NativeHandlerPublic::doPPVChannelSync()
{
    NATIVEHANDLER_LOG("doPPVChannelSync\n");
    return true;
}

bool
NativeHandlerPublic::doPPVChannelError()
{
    NATIVEHANDLER_LOG("doPPVChannelError\n");
    if(2 == (int)defNativeHandler().getState())
        return false;
    else
        return true;
}

// media message
bool
NativeHandlerPublic::doChannelListError()
{
    NATIVEHANDLER_LOG("doChannelListError\n");
    if(2 == (int)defNativeHandler().getState())
        return false;
    else
        return true;
}

bool
NativeHandlerPublic::doReminderListSync()
{
    reminder_list_sync();
    return true;
}

bool
NativeHandlerPublic::doLocalTS(Message *msg)
{
    msg->arg2 = 0xe003/*APP_MSG_LOCALTS*/;
    return true;
}

// specific IRkey message
bool
NativeHandlerPublic::doUpdateGolbalKeyTable()
{
    KeyTableUpdate();
    return true;
}

bool
NativeHandlerPublic::doIME(Message *msg)
{
    return false;
}

bool
NativeHandlerPublic::doClear(Message *msg)
{
    if(mBrowserFocusOnEditBox == 1) {
        msg->arg1 = EIS_IRKEY_BACK;
    }
    return false;
}

bool
NativeHandlerPublic::doSwitch()
{
#ifdef INCLUDE_DVBS
    std::string mid_url = "";
    mid_url = LOCAL_WEBPAGE_PATH_PREFIX"/template1/play.html";

    if(mNICStatus == NICStatus_on)	//switch to dvbs epg only when network interface card is off
        return false;

    sys_appmode_set(APPMODE_DVBS);  //lh 2010-3-27 ????!:DVBS

    defNativeHandler().setState(NativeHandler::Running);
    epgBrowserAgent().openUrl(mid_url.c_str());

    Message *msg = NULL;

    msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptClearNetwork, 0);
    epgBrowserAgent().sendMessage(msg);
    return true;
#endif
    return false;
}

bool
NativeHandlerPublic::doDeskTop()
{
    return false;
}

// upgrade message
bool
NativeHandlerPublic::doOpenUpgradePage()
{
    std::string mid_url = LOCAL_WEBPAGE_PATH_PREFIX LOCAL_WEBPAGE_PATH_UPGRADE;

    LogUserOperDebug("run here! clean Browser Cache!\n");
    TAKIN_browser_cleanCache();

    if(mDialog) {
        delete mDialog;
        mDialog = 0;
    }

    if(defNativeHandler().getState() == NativeHandler::Upgrade) {
        epgBrowserAgent().openUrl(mid_url.c_str());
        return true;
    }

    // mid_timer_delete(app_openEDS2,0);
    systemManager().destoryAllPlayer();
#if defined(SQM_VERSION_C21) || defined(SQM_VERSION_C22) || defined(SQM_VERSION_C23)
    sqm_port_msg_write(MSG_STOP);
#ifdef HEILONGJIANG_SD
    yos_systemcall_runSystemCMD("killall -9 usb.elf", NULL);
    //yos_systemcall_runSystemCMD("killall -9 sqm.elf", NULL);
#else
    sleep(3); // waiting for Message processing, important
    yos_systemcall_runSystemCMD("killall -9 sqm.elf", NULL);
    yos_systemcall_runSystemCMD("killall -9 usbmounter", NULL);
#endif
#endif
#if defined(INCLUDE_DLNA)
    yos_systemcall_runSystemCMD("killall -9 FASTDMRAPP", NULL);
#endif

    if (defNativeHandler().getState() == NativeHandler::Recovery)
        Hippo::epgBrowserAgent().sendEmptyMessageDelayed(MessageType_Timer, 10);

    defNativeHandler().setState(NativeHandler::Upgrade);
    epgBrowserAgent().openUrl(mid_url.c_str());
    return true;
}

bool
NativeHandlerPublic::doAndroidOpenUrl(Message* msg)
{
    if (msg->obj)
        epgBrowserAgent().openUrl(static_cast<StringData*>(msg->obj)->getData());
    return true;
}

bool
NativeHandlerPublic::doAndroidSetSurface(int surface)
{
#ifdef ANDROID
    ygp_layer_setDisplaySurface(surface);
#endif
    return true;
}

bool NativeHandlerPublic::doAndroidStop()
{
#ifdef ANDROID
    // IPTV¸úTR069Ã»ÓÐÆô¶¯¹ØÏµ¡£¡£
// #ifdef INCLUDE_TR069
    // itms_stop();
// #endif
    Jvm_Main_Close();
    systemManager().destoryAllPlayer();
    yhw_board_unInitVideoPlayer();

    // ´ý»úµÄÊ±ºò¾ÍÇåµô°É¡£
    epgBrowserAgentCleanTakinCache();
#endif
    return true;
}

bool NativeHandlerPublic::doAndroidRestart()
{
#ifdef ANDROID
    yhw_board_InitVideoPlayer();
    epgBrowserAgentCleanTakinCache();
    epgBrowserAgent().closeAllConnection();
#endif
#if defined(SQM_VERSION_ANDROID)
    sqm_port_sqmloader_start();
#endif
    return true;
}

// tr069/TMS message
#if defined(INCLUDE_TR069)
bool
NativeHandlerPublic::doTr069Start()
{
    if(NativeHandler::Boot == state()) {
        Message *msg = epgBrowserAgent().obtainMessage(MessageType_KeyDown, ITMS_START, 0);
        epgBrowserAgent().sendMessage(msg);//ITMS_START
    }

    // µÚ¶þ¸ö²ÎÊývalue£¬"V1_CHOISE_NOAPP"£¬±¾²»»áÓÐÖµ£¬´«ÖµÖ»ÊÇÎªÔÚV1×öÑ¡ÔñÄÄ¸öº¯ÊýÁÙÊ±ÕâÑùÓÃ×Å£¬ ÔÚtr069.cppÐÞ¸ÄºóÈ¥µô
    TR069_API_SETVALUE((char*)"Task.Active", "V1_CHOISE_NOAPP", 1);

    return true;
}
#endif

#if defined(INCLUDE_TR069)
bool
NativeHandlerPublic::doTr069Stop()
{
    TR069_API_SETVALUE((char*)"Task.Suspend", NULL, 1);
    return true;
}
#endif

bool
NativeHandlerPublic::doUpgradeTms()
{
	int upgradeMode = 0;
    sysSettingGetInt("upgradeMode", &upgradeMode, 0);

    //mid_timer_delete(app_upgrade_loop,0);
    if(upgradeMode != 1) { // DVB mode or IP + DVB mode
       // UpgradeManager().upgradeCheckStop(1);
    }
    if(upgradeMode != 2) {	// IP mode or IP + DVB mode
      //  UpgradeManager().upgradeCheckStop(0);
    }
    return true;
}

bool
NativeHandlerPublic::doUpgradeAuto()
{
    if(NativeHandler::Upgrade == state())
        return false;
    // app_upgrade_dispatch(APP_UPGRADE_NORMAL, NULL, 0, UPGRADE_START);
    int tr069Upgrade = 0;
    sysSettingGetInt("tr069_upgrades", &tr069Upgrade, 0);
	int upgradeMode = 0;
    sysSettingGetInt("upgradeMode", &upgradeMode, 0);
    if(upgradeMode == 1 && 0 == tr069Upgrade ) {
        if (upgradeManager())
            upgradeManager()->touchOffUpgradeCheck(UpgradeManager::UMUT_IP_SOFTWARE, false);
    }
    // if(sys_upgradeMode_get() != 2) {	// IP mode or IP + DVB mode
        // UpgradeManager().upgradeCheckStart(0);
    // }
    return true;
}

bool
NativeHandlerPublic::doRequestReboot()
{
    mid_fpanel_reboot();
    return true;
}

#if defined(INCLUDE_TR069) || defined(INCLUDE_HMWMGMT)
bool
NativeHandlerPublic::doUpgradeTr069BootRequest()
{
    return true;
}
#endif

#if defined(INCLUDE_TR069) || defined(INCLUDE_HMWMGMT)
bool
NativeHandlerPublic::doUpgradeTr069Request()
{
    if(NativeHandler::Upgrade == state())
        return false;
    //int tr069Upgrade = 0;
    ///sysSettingGetInt("tr069_upgrades", &tr069Upgrade, 0);
    //if(1 == tr069Upgrade && !mDialog) {

    // tr069Upgrade Tr069 is to take over the upgrade, either 0 or 1 Tr069 can upgrade.
    // To 1 only tr069 upgrade, all upgrades can be upgraded to 0.
    if(!mDialog) {
        if (upgradeManager())
            upgradeManager()->touchOffUpgradeCheck(UpgradeManager::UMUT_IP_SOFTWARE, true);
        return true;
    }
#ifdef INCLUDE_TR069
    LogTr069Debug("tr069_upgrades = %d......cannot execute tms upgrade!!!!\n");
    TR069_API_SETVALUE((char*)"Event.Upgraded", NULL, -1);
#endif
    return true;
}
#endif

bool
NativeHandlerPublic::doTr069NetConnectOk()
{
    if(NativeHandler::Boot == state()) {
        Message *msg = epgBrowserAgent().obtainMessage(MessageType_KeyDown, ITMS_OK, 0);
        epgBrowserAgent().sendMessage(msg); // ITMS_OK
    }

    if(NativeHandler::Upgrade == state())
        return false;

	int tr069Upgrade = 0;
    sysSettingGetInt("tr069_upgrades", &tr069Upgrade, 0);
    if( 1 == tr069Upgrade )
        return false;

    return true;
}

bool
NativeHandlerPublic::doTr069NetConnectError()
{
    if(NativeHandler::Boot == state()) {
        Message *msg = epgBrowserAgent().obtainMessage(MessageType_KeyDown, ITMS_ERROR, 0);
        epgBrowserAgent().sendMessage(msg);//ITMS_ERROR
    }

    if(NativeHandler::Upgrade == state())
        return false;

    //upgradeManager()->touchOffUpgradeCheck(UpgradeManager::IP_SOFTWARE, false);
    //app_upgrade_dispatch(APP_UPGRADE_NORMAL, NULL, 0, UPGRADE_START);
    return true;
}

// EPG auth
bool
NativeHandlerPublic::doEpgAuth()
{
    std::string url = Customer().AuthInfo().AvailableEpgUrlWithoutPath();
    if (url.empty()) {
        url = LOCAL_WEBPAGE_PATH_ERROR;
    } else {
        url += "EPG/jsp/AuthenticationURL";
        url = Customer().AuthInfo().ConcatenateUrl(url);
    }
    defNativeHandler().setState(NativeHandler::Running);
    epgBrowserAgent().openUrl(url.c_str());
    return true;
}

//usb plugin message
bool
NativeHandlerPublic::doUSBInsert(Message *msg)
{
    /** YX_USBPLUGIN event depended on sdk version**/
#ifdef INCLUDE_DLNA
	mid_dlna_GetEvent(msg->arg1);
#endif
#ifdef USE_DISK
    storage_in_out_event(YX_EVENT_HOTPLUG_ADD, msg->arg2);
#endif
    return true;
}

bool
NativeHandlerPublic::doUSBUninsert(Message *msg)
{
    /** YX_USBPLUGOUT event depended on sdk version**/
#ifdef INCLUDE_DLNA
	mid_dlna_GetEvent(msg->arg1);
#endif
#ifdef USE_DISK
    storage_in_out_event(YX_EVENT_HOTPLUG_REMOVE, msg->arg2);
#endif
#ifndef ANDROID
    monitorUnInsertUDisk();
#endif
    return true;
}

bool
NativeHandlerPublic::doUSBConfig(Message *msg)
{
    std::string mid_url = "";

    mid_url = LOCAL_WEBPAGE_PATH_PREFIX LOCAL_WEBPAGE_PATH_USB_CONFIG;
    NATIVEHANDLER_LOG("url(%s)\n", mid_url.c_str());
    defNativeHandler().setState(NativeHandler::UConfig);
    epgBrowserAgent().openUrl(mid_url.c_str());
    return true;
}

bool
NativeHandlerPublic::doUSBUpgrade(Message *msg)
{
    if (defNativeHandler().getState() == NativeHandler::Recovery && 0 == sBrowserTimer)
        Hippo::epgBrowserAgent().sendEmptyMessageDelayed(MessageType_Timer, 10);

    std::string mid_url = "";
    mid_url = LOCAL_WEBPAGE_PATH_PREFIX LOCAL_WEBPAGE_PATH_USB_UPGRADE;
    NATIVEHANDLER_LOG("url(%s)\n", mid_url.c_str());
    defNativeHandler().setState(NativeHandler::Upgrade);
    epgBrowserAgent().openUrl(mid_url.c_str());
    return true;
}

#if defined(Huawei_v5)
bool
NativeHandlerPublic::doHdmiInsert(int arg)
{
    int eFlag;
    appSettingGetInt("HDCPEnableDefault", &eFlag, 0);
    if (eFlag)
        yhw_vout_setHdcpMode(1);
    return true;
}

bool
NativeHandlerPublic::doHdmiUnInsert(int arg)
{
    yhw_vout_setHdcpMode(0);
    return true;
}

bool
NativeHandlerPublic::doHdmiConnect(int arg)
{
    return true;
}

bool
NativeHandlerPublic::doHdmiDisconnect(int arg)
{
    yhw_vout_setHdcpMode(0);
    return true;
}
#endif

bool
NativeHandlerPublic::doNetworkDNSTimeout(int pIndex)
{
    return false;
}

bool
NativeHandlerPublic::doNetworkDNSError(int pIndex)
{
    return false;
}
#ifdef Liaoning
bool
NativeHandlerPublic::doBack(){
    return false;
}
#endif

#if defined(PAY_SHELL)
bool
NativeHandlerPublic::doPayShellError(int ErrorId)
{
    switch(ErrorId) {
    case PAY_SHELL_ERROR_DOWNLOAD: // CTC 0030
        epgBrowserAgent().openUrl(LOCAL_WEBPAGE_PATH_ERROR"0030");
        defNativeHandler().setState(NativeHandler::Error);
        break;
    case PAY_SHELL_ERROR_AUTH: // CTC 0031
        epgBrowserAgent().openUrl(LOCAL_WEBPAGE_PATH_ERROR"0031");
        defNativeHandler().setState(NativeHandler::Error);
        break;
    case PAY_SHELL_ERROR_LOAD: // CTC 0032
        epgBrowserAgent().openUrl(LOCAL_WEBPAGE_PATH_ERROR"0032");
        defNativeHandler().setState(NativeHandler::Error);
        break;
    case PAY_SHELL_ERROR_MSG2BROWSER: // CTC 0033
        epgBrowserAgent().openUrl(LOCAL_WEBPAGE_PATH_ERROR"0033");
        defNativeHandler().setState(NativeHandler::Error);
        break;
    default:
        break;
    }
    return true;
}
#endif

bool
NativeHandlerPublic::doNetworkRefresh()
{
    NATIVEHANDLER_LOG("//TODO This log can delete. do network refresh.\n");
    NetworkCard* device = networkManager().getActiveDevice();
    if (!device)
        return false;
    return NativeHandlerPublic::doNetworkReconnect(0, device->devname());
}

bool
NativeHandlerPublic::doNetworkReconnect(int force, const char* devname)
{
    NATIVEHANDLER_LOG("Force[%d], DeviceName[%s]\n", force, devname);
    NetworkCard* device = networkManager().getDevice(devname);
    if (!device)
        return false;

    if (NL_FLG_RUNNING != device->linkStatus())
        return true;

    bool bConn = false;
    NetworkInterface* iface = 0;
    std::vector<std::string> ifnames = device->getIfnames();

    if (NetworkCard::NT_WIRELESS == device->getType()) {
        WirelessNetworkCard* wifi = static_cast<WirelessNetworkCard*>(device);
        if (!wifi->joinConnStatus())
            bConn = wifi->reJoinAP(); //wireless need join AP.
    }

    for (size_t i = 0; i < ifnames.size(); ++i) {
        iface = networkManager().getInterface(ifnames[i].c_str());
        if (iface && (!iface->isActive() || bConn || force))
            iface->connect();
    }
    networkManager().refresh();
    return true;
}

bool
NativeHandlerPublic::doNetworkErrorDisplay(int errcode, const char* errmsg)
{
    NetworkErrorCode* errnode = networkManager().getErrorCode(errcode);
    if (!errnode)
        return false;
    TR069_API_SETVALUE("ErrorCode", "",errnode->getCodeId());
    Message* msg = 0;
    char errpage[URL_LEN] = { 0 };
    char errinfo[URL_LEN] = { 0 };
    switch (errnode->getShowMode()) {
    case OPEN_ERROR_PAGE: //C10
        NATIVEHANDLER_LOG("ErrorCode[%d]\n", errnode->getCodeId());
        snprintf(errpage, URL_LEN, "%s%d", LOCAL_WEBPAGE_PATH_ERROR, errnode->getCodeId());
        defNativeHandler().setState(NativeHandler::Error);
        epgBrowserAgent().openUrl(errpage);
        break;
    case SHOW_MINI_ALERT:
        NATIVEHANDLER_LOG("PromptId[%d]\n", errnode->getPromptId());
        msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, errnode->getPromptId(), 1);
        epgBrowserAgent().sendMessage(msg);
        break;
    case SEND_ERROR_EVENT: //C20
        snprintf(errinfo, URL_LEN, "{\"type\":\"EVENT_STB_ERROR\",\"error_code\":\"%d\"}", errnode->getCodeId());
        browserEventSend(errinfo, NULL);
    default:
        ;
    }
    if (errmsg) //Huawei syslog needed.
        NATIVEHANDLER_LOG_ERROR("%s %s\n", errnode->getMessage(), errmsg);
    else
        NATIVEHANDLER_LOG_ERROR("%s\n", errnode->getMessage());
    return true;
}

bool
NativeHandlerPublic::doNetworkProtocolUp(int errcode, const char* devname)
{
    NATIVEHANDLER_LOG("ErrorCode[%d], DeviceName[%s]\n", errcode, devname);

    epgBrowserAgent().closeAllConnection();

    //TODO Jiangsu is different, and wifi is not finished.

    //bool bReconnect = false;
    Message* msg = 0;
    NetworkInterface* iface = 0;
    NetworkCard* device = networkManager().getDevice(devname);
    if (!device)
        return false;

    if (DONT_ISSET(NMB_SEND_WEBPAGE_KEY)) {
        msg = epgBrowserAgent().obtainMessage(MessageType_KeyDown, MV_Network_PhysicalUp/*0x9000*/, 0);
        epgBrowserAgent().sendMessageAtFrontOfQueue(msg);
    }

    if (NetworkCard::NT_ETHERNET == device->getType()) {
        //ethernet
        msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptDisconnect, 0);
        epgBrowserAgent().sendMessage(msg);
        msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptDisconnectAlert, 0);
        epgBrowserAgent().sendMessage(msg);
        mid_fpanel_netled(device->linkStatus());
        //bReconnect = true;
    } else {
        //wireless
        msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptWifiLowQuality, 0);
        epgBrowserAgent().sendMessage(msg); // clean the wifi low quality icon
        msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptWifiDisconnect, 0);
        epgBrowserAgent().sendMessage(msg); // clean the wifi disconnect icon
        msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptWifiDisconnectAlert, 0);
        epgBrowserAgent().sendMessage(msg); // clean the wifi disconnect alert errcode.
    }
    defNativeHandler().removeMessages(MessageType_System);
    //Reconnect all network interface
    return doNetworkReconnect(0, devname);
}

bool
NativeHandlerPublic::doNetworkProtocolDown(int errcode, const char* devname)
{
    NATIVEHANDLER_LOG("ErrorCode[%d], DeviceName[%s]\n", errcode, devname);

    Message* msg = 0;
    NetworkInterface* iface = 0;
    NetworkCard* device = networkManager().getDevice(devname);
    if (!device)
        return false;
    mid_fpanel_netled(0);

    if (NetworkCard::NT_ETHERNET == device->getType()) {
        //ethernet
        if (DONT_ISSET(NMB_SHOW_BOTTOM_ICON)) {
            msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptDisconnect, 1);
            epgBrowserAgent().sendMessage(msg);
        }
    } else {
        //wireless
        if (DONT_ISSET(NMB_SHOW_BOTTOM_ICON)) {
            msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptWifiLowQuality, 0);
            epgBrowserAgent().sendMessage(msg); // clean the wifi low quality icon
            msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptWifiDisconnect, 1);
            epgBrowserAgent().sendMessage(msg); // display the wifi disconnect icon
        }
        networkManager().refresh();
    }
    defNativeHandler().removeMessages(MessageType_System);
    defNativeHandler().removeMessages(MessageType_Network, MV_Network_UnlinkMonitor, errcode);
    return doNetworkUnlinkMonitor(errcode, devname);
}

bool
NativeHandlerPublic::doNetworkUnlinkMonitor(int errcode, const char* devname)
{
    NATIVEHANDLER_LOG("ErrorCode[%d], DeviceName[%s]\n", errcode, devname);
    NetworkCard* device = networkManager().getDevice(devname);
    if (!device)
        return false;
    int status = device->linkStatus();

    if (NL_FLG_RUNNING == status)
        return true;

    if (DONT_ISSET(NMB_SHOW_ERROR_CODE))
        doNetworkErrorDisplay(errcode, 0);

    StringData* data = new StringData(devname);
    Message* msg = defNativeHandler().obtainMessage(MessageType_Network, MV_Network_UnlinkMonitor, errcode, data);
    data->safeUnref();
    defNativeHandler().sendMessageDelayed(msg, 1000);

    if (NL_FLG_DOWN == status) //If wireless need linkup manually
        device->linkUp();
    return true;
}

bool
NativeHandlerPublic::doNetworkWifiJoinFail(int errcode, const char* devname)
{
    NATIVEHANDLER_LOG("ErrorCode[%d], DeviceName[%s]\n", errcode, devname);

    Message* msg = 0;
    if (DONT_ISSET(NMB_SHOW_ERROR_CODE))
        doNetworkErrorDisplay(errcode, 0);
    if (DONT_ISSET(NMB_SHOW_BOTTOM_ICON)) {
        msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptWifiNoSignal, 1);
        epgBrowserAgent().sendMessage(msg);
    }
    mid_fpanel_netled(0);
    return true;
}

bool
NativeHandlerPublic::doNetworkWifiJoinSuccess(int arg, const char* devname)
{
    NATIVEHANDLER_LOG("arg[%d] DeviceName[%s]\n", arg, devname);
    NetworkCard* device = networkManager().getDevice(devname);
    if (!device)
        return false;

    NetworkInterface* iface = 0;
    Message* msg = 0;
    msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptWifiNoSignal, 0);
    epgBrowserAgent().sendMessage(msg); // clean the wifi no signal icon
    msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptWifiLowQuality, 0);
    epgBrowserAgent().sendMessage(msg); // clean the wifi low quality icon

    //wireless need disconnect all network for reconnection later.
    std::vector<std::string> ifnames = device->getIfnames();
    for (size_t i = 0; i < ifnames.size(); ++i) {
        iface = networkManager().getInterface(ifnames[i].c_str());
        if (iface && iface->isActive())
            iface->disconnect();
    }
    mid_fpanel_netled(1);
    return true;
}

bool
NativeHandlerPublic::doNetworkWifiCheckSignal(int arg, const char* devname)
{
    NATIVEHANDLER_LOG("arg[%d], DeviceName[%s]\n", arg, devname);
    NetworkCard* device = networkManager().getDevice(devname);
    if (!device)
        return false;

    if (NetworkCard::NT_WIRELESS != device->getType())
        return false;

    Message* msg = 0;
    static int loopcnt = 0;
    int qual = static_cast<WirelessNetworkCard*>(device)->getSignalQuality(0, 0, 0);
    if (qual < 20) {
        loopcnt = (loopcnt % 2) + 1; //display two wifi icons alternately
        msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptWifiLowQuality, loopcnt);
        epgBrowserAgent().sendMessage(msg);
    } else {
        msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptWifiLowQuality, 0);
        epgBrowserAgent().sendMessage(msg);
    }
    return true;
}

bool
NativeHandlerPublic::doNetworkProtocolConflict(int errcode, const char* ifname)
{
    NATIVEHANDLER_LOG("ErrorCode[%d], IfaceName[%s]\n", errcode, ifname);

    if (DONT_ISSET(NMB_SHOW_ERROR_CODE))
        doNetworkErrorDisplay(errcode, 0);
    return true;
}

bool
NativeHandlerPublic::doNetworkConnectOk(int errcode, const char* ifname)
{
    NATIVEHANDLER_LOG("ErrorCode[%d], IfaceName[%s]\n", errcode, ifname);

    Message* msg = 0;
    NetworkInterface* iface = networkManager().getInterface(ifname);
    if (!iface)
        return false;

    if (NetworkInterface::PT_PPPOE != iface->getProtocolType()) {
        iface->startCheckIP();
        networkManager().refresh();
    }
    if (DONT_ISSET(NMB_SEND_WEBPAGE_KEY)) {
        switch (iface->getProtocolType()) {
        case NetworkInterface::PT_DHCP:
            msg = epgBrowserAgent().obtainMessage(MessageType_KeyDown, DHCP_CONNECT_OK/*0x9510*/, 0);
            epgBrowserAgent().sendMessage(msg);
            break;
        case NetworkInterface::PT_PPPOE:
            msg = epgBrowserAgent().obtainMessage(MessageType_KeyDown, PPPOE_CONNECT/*0x9580*/, 0);
            epgBrowserAgent().sendMessage(msg);
        default:
            ;
        }
    }
    if (DONT_ISSET(NMB_CONNECTOK_DOMENU))
        doMenu();
    return true;
}

bool
NativeHandlerPublic::doNetworkDhcpError(int errcode, const char* ifname)
{
    NATIVEHANDLER_LOG("ErrorCode[%d], IfaceName[%s]\n", errcode, ifname);

    Message* msg = 0;
    NetworkInterface* iface = networkManager().getInterface(ifname);
    if (!iface)
        return false;

    int tryTimes = iface->getDHCPSetting().getRetryTimes();
    int sumTimes = iface->getConnectionTimes();

    if (DONT_ISSET(NMB_SEND_WEBPAGE_KEY)) {
        msg = epgBrowserAgent().obtainMessage(MessageType_KeyDown, DHCP_CONNECT_ERROR/*0x9511*/, 0);
        epgBrowserAgent().sendMessage(msg);
    }

    NATIVEHANDLER_LOG("tryTimes[%d] sumTimes[%d] mod[%d]\n", tryTimes, sumTimes, sumTimes % tryTimes);
    if (sumTimes % tryTimes) {
        iface->disconnect();
        iface->connect();
        networkManager().refresh();
        return true;
    }

    //TODO Double Stack not support now.

    if (DONT_ISSET(NMB_SHOW_ERROR_CODE))
        doNetworkErrorDisplay(errcode, iface->getDHCPSetting().getUsername());

    return true;
}

bool
NativeHandlerPublic::doNetworkPppoeError(int errcode, const char* ifname)
{
    NATIVEHANDLER_LOG("ErrorCode[%d], IfaceName[%s]\n", errcode, ifname);

    Message* msg = 0;
    NetworkInterface* iface = networkManager().getInterface(ifname);
    if (!iface)
        return false;

    int tryTimes = iface->getPPPSetting().getRetryTimes();
    int sumTimes = iface->getConnectionTimes();

    if (DONT_ISSET(NMB_SEND_WEBPAGE_KEY)) {
        if (NetworkErrorCode::isAuthFail(errcode))
            msg = epgBrowserAgent().obtainMessage(MessageType_KeyDown, PPPOE_PASSWORD_ERROR/*0x9583*/, 0);
        else
            msg = epgBrowserAgent().obtainMessage(MessageType_KeyDown, PPPOE_CONNECT_ERROR/*0x9581*/, 0);
        epgBrowserAgent().sendMessage(msg);
    }

    NATIVEHANDLER_LOG("tryTimes[%d] sumTimes[%d] mod[%d]\n", tryTimes, sumTimes, sumTimes % tryTimes);
    if (sumTimes % tryTimes) {
        iface->connect();
        networkManager().refresh();
        return true;
    }

    //TODO Double Stack not support now.

    if (DONT_ISSET(NMB_SHOW_ERROR_CODE))
        doNetworkErrorDisplay(errcode, iface->getPPPSetting().getUsername());
    return true;
}

void
NativeHandlerPublic::DONT_SET(int bit)
{
    mDoNetworkMask |= bit;
}

void
NativeHandlerPublic::DONT_CLR(int bit)
{
    mDoNetworkMask &= ~bit;
}

void
NativeHandlerPublic::DONT_ZERO()
{
    mDoNetworkMask = 0;
}

bool
NativeHandlerPublic::DONT_ISSET(int bit)
{
    return 0 != (mDoNetworkMask & bit);
}

} //end namespace

extern "C" void
sys_key_editfocus_set(int pFlage) // ????,???????,?????????????
{
    mBrowserFocusOnEditBox = pFlage;
    return ;
}
