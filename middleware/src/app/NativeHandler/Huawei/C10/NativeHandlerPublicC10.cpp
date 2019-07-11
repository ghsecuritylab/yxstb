
#include <iostream>

#include "NativeHandlerPublicC10.h"
#include "NativeHandlerAssertions.h"

#include "Message.h"
#include "MessageTypes.h"
#include "MessageValueSystem.h"
#include "MessageValueNetwork.h"
#include "MessageValueMaintenancePage.h"

#include "BrowserAgent.h"

#include "UltraPlayerWebPage.h"
#include "config/webpageConfig.h"

#include "KeyDispatcher.h"
#include "PowerOffDialog.h"
#include "DMRDialog.h"
#include "UpgradeDialog.h"
#include "RestartDialog.h"
#include "SystemManager.h"
#include "UpgradeManager.h"

#include "Hippo_Context.h"
#include "LogModuleHuawei.h"
#include "BootImagesShow.h"
#include "StringData.h"
#include "Verimatrix.h"

#include "Tr069.h"

#if defined(ANDROID)
#include "IPTVMiddleware.h"
#endif

#include "VisualizationDialog.h"
#include "auth.h"
#include "concatenator_hw_logout.h"

#include "config.h"
#include "sys_key_deal.h"
#include "sys_msg.h"
#include "mid_sys.h"

#include "AppSetting.h"
#include "SysSetting.h"
#include "app_sys.h"
#include "app/app_heartbit.h"
#include "Session.h"

#include "mid_fpanel.h"
#include "mid/mid_http.h"
#include "mid/mid_time.h"

#include "porting/tvms/tvms.h"
#if defined(SQM_INCLUDED)
#include "sqm_port.h"
#endif

#ifdef INCLUDE_DLNA
#include "mid_dlna_ex.h"
#endif

#if defined(CYBERCLOUD)
#include "cloud_api.h"
#endif

#include "NetworkFunctions.h"
#include "customer.h"
#if defined(XMPP)
#include "XmppService.h"
#endif
extern char* global_cookies;

namespace Hippo {

#if defined(Jiangsu)
int NativeHandlerPublicC10::networkPhysicalDown = 0;
#endif


int NativeHandlerPublicC10::mNtpsyncerror = 0;

NativeHandlerPublicC10::NativeHandlerPublicC10()
    : mNtpDnsError(0)
    , mDHCPErrorCount(0)
    , mPppoeErrorCount(0)
    , mKeyTimes(0)
    , mLastKeyDownPressed(0)
    , timeFirsePressed(0)
{
}

NativeHandlerPublicC10::~NativeHandlerPublicC10()
{
}

bool
NativeHandlerPublicC10::handleMessage(Message *msg)
{
    if(MessageType_Repaint != msg->what)
        NATIVEHANDLER_LOG("what(0x%x), message(0x%x)\n", msg->what, msg->arg1);

    if(MessageType_Network == msg->what) {
        if (MV_Network_DhcpError == msg->arg1) {
            NATIVEHANDLER_LOG_ERROR("DHCP Connect Error event\n");
            if (DHCP_CONNECT_ERROR == msg->arg2) {
                mDHCPErrorCount++;
                NATIVEHANDLER_LOG_ERROR("DHCP ConnectError Count increased\n");
            }
        }

        if( MV_Network_PppoeError == msg->arg1) {
            NATIVEHANDLER_LOG_ERROR("PPPoE Connect Error event\n");
            if(PPPOE_CONNECT_ERROR == msg->arg2) {
                mPppoeErrorCount += 1;
                NATIVEHANDLER_LOG_ERROR("PPPoE ConnectError Count increased\n");
            }
        }
#if defined(Jiangsu)
        switch(msg->arg1) {
            case MV_Network_PhysicalDown_OnOpenError: {
                LogRunOperDebug("Network MV_Network_PhysicalDown_jiangsu Down\n");
                if (0 == networkPhysicalDown) {
                    LogRunOperDebug("I will send zhe MV_Network_PhysicalDown message\n");
                    sendMessageToNativeHandler(MessageType_Network, MV_Network_PhysicalDown_DoOpenError, 0, 45000);
                    networkPhysicalDown = 1;
                }
                return true;
            }
            case MV_Network_PhysicalDown_DoOpenError: {
                LogRunOperDebug("Network MV_Network_PhysicalDown_DoOpenError Down\n");
                networkPhysicalDown = 0;
                char devname[USER_LEN] = { 0 };
                network_default_devname(devname, USER_LEN);
                if (0 == network_device_link_state(devname)) {
                    LogRunOperDebug("Network MV_Network_PhysicalDown Down\n");
                    epgBrowserAgent().openUrl(LOCAL_WEBPAGE_PATH_ERROR"1901");
                    defNativeHandler().setState(NativeHandler::Error);
                }
                return true;
            }
        }
#endif
    }

#if defined(Jiangsu)
    if (MessageType_ClearAuthLogo == msg->what) {
        LogRunOperDebug("Clear authlogo!!\n");
        BootImagesShowAuthLogo(0);
        return true;
    }
    if (MessageType_Tr069 == msg->what) {
        switch(msg->arg1) {
            case ITMS_OPEN_SERIAL_NUM_BOX:
                doOpenSerialNumberPaga();
                break;
            default:
                break;
       }
    }
#endif

#if defined(CYBERCLOUD)
	if (MessageType_CyberCloud == msg->what) {
		//printf("cloudExitUrlbackURL =%s\n", (msg->arg2 != NULL) ? msg->arg2:"NONE");
		char* back_url = (char*)msg->arg2;
		switch(msg->arg1){
		case CloudKey_Exit:
		case CloudKey_Menu:
			CStb_CyberCloudTaskStop();
			sendMessageToKeyDispatcher(MessageType_KeyDown, EIS_IRKEY_MENU, 0, 0);
		#if 0
			if(strlen(back_url) && !strncmp(back_url, "http://", 7)){
				epgBrowserAgent().openUrl(back_url);
			}
		#endif
			break;
		default:
			break;
		}

		return true;
	}
#endif
    if (MessageType_KeyDown == msg->what) {
        switch(msg->arg1) {
        case MV_Maintenance_openMaintenancePage_clear:
            doCloseMaintenancePage();
            break;
        case EIS_IRKEY_IME:
            doDealIMEKey();
            break;
        case MV_Maintenance_stopCollectDebugInfo:
            mid_sys_setStartCollectDebugInfo(0);
            sendMessageToEPGBrowser(MessageType_Prompt, Hippo::BrowserAgent::PromptVisualizationDebug, 0, 0);
            break;
        default:
            NATIVEHANDLER_LOG("do nothing, VisualizationDialog deal them\n");
            break;
        }
    }
    switch (msg->arg1) {
        case EIS_IRKEY_RED:
        case EIS_IRKEY_BLUE:
        case EIS_IRKEY_YELLOW:
        case EIS_IRKEY_GREEN:
        case EIS_IRKEY_NVOD:{
            return doShortCut(msg);
        }
    }

    return NativeHandlerPublic::handleMessage(msg);
}
bool
NativeHandlerPublicC10::onDLNADialog()
{
    NATIVEHANDLER_LOG("NativeHandlerPublicC10::onDLNADialog \n");
    if (mDialog == 0) {

    	keyDispatcher().setEnabled(false);

        mDialog = new DlnaDialog();
        ((DlnaDialog *)mDialog)->setHandler(this);
        mDialog->draw();
    }
    return true;
}

bool
NativeHandlerPublicC10::onPowerOff()
{
    NATIVEHANDLER_LOG("NativeHandlerPublicC10::onPowerOff \n");
#if defined(SHANGHAI_HD) || defined(SHANGHAI_SD) || defined(NEIMENGGU_HD) || defined(ANDROID)
    doPowerOff();
#else
    if (mDialog == 0) {
    	/* ÔÝÊ±½ûÖ¹¼üÖµ±í¹¦ÄÜ */
    	keyDispatcher().setEnabled(false);
        /* ´´½¨¶Ô»°¿ò */
        mDialog = new PowerOffDialog();
        ((PowerOffDialog *)mDialog)->setHandler(this);
        mDialog->draw();
    }
#endif
    return true;
}

bool
NativeHandlerPublicC10::doPowerOff()
{
    char mid_url[1024] = {0};
    Message *msg = NULL;
	int type = 0;
	struct timeval tv = {0, 0};
    tv.tv_sec = 2;

    NATIVEHANDLER_LOG("NativeHandlerPublicC10::doPowerOff \n");
    mNtpsyncerror = 0;
#if defined(XMPP)
    xmppTaskStop();
#endif
#if defined(INCLUDE_TR069)
	sysSettingGetInt("tr069_type", &type, 0);
    if (1 == type) {
        TR069_API_SETVALUE((char*)"Event.Shutdown", NULL, 1);
    } else if (2 == type) {
        char value[16] = {0};
        uint size = 16;
        TR069_PORT_GETVALUE((char*)"Device.DeviceInfo.DeviceStatus", value, size);
        if(strcmp(value,(char*)"OFFLINE")){
            TR069_PORT_SETVALUE((char*)"Device.DeviceInfo.DeviceStatus", (char*)"OFFLINE", 0);
            TR069_API_SETVALUE((char*)"Event.ValueChange", (char*)"Device.DeviceInfo.DeviceStatus", 0);
        }
    }
    itms_stop();
#endif
    systemManager().destoryAllPlayer();

    msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptClearAll, 0);
    epgBrowserAgent().sendMessage(msg);

#if defined(ANDROID)
    sys_appmode_set(APPMODE_STANDBY);
    defNativeHandler().setState(NativeHandler::Standby);
    strcpy(mid_url, LOCAL_WEBPAGE_PATH_PREFIX"/boot.html");
    epgBrowserAgent().openUrl(mid_url);
    yhw_board_unInitVideoPlayer();
    return true;
#endif

#if defined(hi3560e)
    yx_drv_videoout_set(1, 0);
#elif defined(hi3716m)
    yhw_vout_setMute(1);
#endif
    mid_fpanel_powerled(0);
    mid_fpanel_netled(0);
#ifdef INCLUDE_DLNA
    mid_dlna_stop();
#endif
    httpHeartClr();
    ConcatenatorHwLogout cl;
    char lastepgLogout[4096] = {0};
    appSettingGetString("epg", lastepgLogout, sizeof(lastepgLogout), 0);
    std::string url = cl(std::string(lastepgLogout));

#if defined(SHANGHAI_HD) || defined(SHANGHAI_SD)
    ctc_http_send_GETmessage(url.c_str(), "Z112|webkit|13|Hybroad|2.2.1");
#elif defined(GUANGDONG)
    ; //Please don't use mixed encoded text in source code
#elif defined(Sichuan)
    ctc_http_send_GETmessage(url.c_str(), NULL);
#else
    mid_http_call(url.c_str(), (mid_http_f)httpDefaultFunc, 0, NULL, 0, global_cookies);
#endif
    /*This select do nothing,  only let this thread wait 2s, so that other thread(such as tr069) get enough time to send messages to server!*/
    select(1, NULL, NULL, NULL, &tv);

#if (defined( SQM_VERSION_C21) || defined( SQM_VERSION_C22 ) || defined(SQM_VERSION_C23) ||defined(SQM_VERSION_C26) || defined(SQM_VERSION_C28)||defined(SQM_VERSION_ANDROID))
    sqm_port_msg_write(MSG_STOP);
#endif

    appSettingSetInt("mute", 0);
    yos_systemcall_runSystemCMD((char*)"rm -rf /var/takincookies", NULL);
    huaweiSetLogForceSend();

#ifdef TVMS_OPEN
    clear_tvms_msg();
#endif

    Hippo::Customer().AuthInfo().eds().Reset();

    NATIVEHANDLER_LOG("//TODO lost mid_ntp_time_contrl()!\n");
    char ifname[USER_LEN] = { 0 };
    NetworkInterface* iface = networkManager().getInterface(network_default_ifname(ifname, USER_LEN));
    if (iface) {
        iface->disconnect();
        if (NetworkCard::NT_WIRELESS == iface->device()->getType())
            iface->device()->linkDown();
        networkManager().refresh();
    }

    sys_appmode_set(APPMODE_STANDBY);
    defNativeHandler().setState(NativeHandler::Standby);

    // epgBrowserAgent().setTakinEDSFlag(0);
    Hippo::Customer().AuthInfo().eds().Reset();

    strcpy(mid_url, LOCAL_WEBPAGE_PATH_PREFIX"/standby.html");
    epgBrowserAgent().openUrl(mid_url);

    NATIVEHANDLER_LOG("enter standby mode!\n");
    return false;
}

bool
NativeHandlerPublicC10::onUpgrade()
{
    if (!mDialog) {
        keyDispatcher().setEnabled(false);
        mDialog = new UpgradeDialog();
        mDialog->draw();
    }
    return true;
}

bool
NativeHandlerPublicC10::doRequestReboot()
{
#if defined(NEIMENGGU_HD) || defined(Sichuan)
    mid_fpanel_reboot();
    return true;
#endif

    //no need dialog
    if (!mDialog) {
        keyDispatcher().setEnabled(false);
        mDialog = new RestartDialog();
        mDialog->draw();
    }
    return true;
}

#if  defined(Gansu)
bool
NativeHandlerPublicC10::doUdiskUnzipErrReboot()
{
    yhw_board_setRunlevel(1);
    mid_fpanel_reboot();
    return true;
}
#endif

bool
NativeHandlerPublicC10::doMenu()
{
    int connecttype = 0;
    systemManager().destoryAllPlayer();
    webchannelFlagSet(0);

    doNetworkRefresh();

#if defined (Liaoning) //TODO
    return false;
#endif

    // TODO: 直接找个地址打开，在eds调度逻辑的附近增加打开首页的逻辑。
    // 依次尝试 ServiceEntry、EPGDomain、setting::epg、eds轮循。
    std::string url;
#if defined(Chongqing) || defined(SHANGHAI_HD) || defined(SHANGHAI_SD) || defined(VNPT_HD) || defined(NEIMENGGU_HD)
    url = Customer().AuthInfo().Get(Hippo::Auth::eAuth_epgDomain);
#else
    KeyDispatcherPolicy *keyPolocy = NULL;
    keyPolocy = keyDispatcher().getPolicy(EIS_IRKEY_MENU);
    if (keyPolocy) {
        NATIVEHANDLER_LOG("Get menu url(%s)\n", keyPolocy->mKeyUrl.c_str());
        url = keyPolocy->mKeyUrl;
    }

    if (!Customer().AuthInfo().CheckUrl(url)) {
        // TODO: 中兴不是这样的逻辑！
#if defined(HUBEI_HD) || defined(Jiangsu)
        if(PLATFORM_ZTE == session().getPlatform()) {
            url = Customer().AuthInfo().Get(Hippo::Auth::eAuth_epgDomain);
        } else
#endif
        {
            url = Customer().AuthInfo().AvailableEpgAuthUrl();
        }
    }
#endif
#ifdef NEIMENGGU_HD
    TAKIN_porting_set_frame_size(TAKIN_FRAME_SIZE, "1280*720", strlen("1280*720") + 1);
    url = LOCAL_WEBPAGE_PATH_PREFIX "/AdjustWebView.html";  // NEIMENGGU use AdjustWebView to adjust page size
#endif

    defNativeHandler().setState(NativeHandler::Running);
    epgBrowserAgent().openUrl(url.c_str());
    BootImagesShowAuthLogo(0);

    return true;
}

// open local webpage message
bool
NativeHandlerPublicC10::doOpenStandbyPage()
{
    std::string mid_url = LOCAL_WEBPAGE_PATH_PREFIX LOCAL_WEBPAGE_PATH_STANDBY;

    systemManager().destoryAllPlayer();

#ifdef INCLUDE_PIP
    codec_lock();
#endif
    sys_appmode_set(APPMODE_STANDBY);
    defNativeHandler().setState(NativeHandler::Standby);

    epgBrowserAgent().openUrl(mid_url.c_str());
    return true;
}

// specific IRkey message
bool
NativeHandlerPublicC10::doConfig(Message *msg)
{
    webchannelFlagSet(0);
    doOpenConfigPage();
    BootImagesShowAuthLogo(0);
    return true;
}

// NTP sync message
bool
NativeHandlerPublicC10::doNtpSyncOk()
{
    CA_VM_API_INIT();
#ifdef TVMS_OPEN
    static int g_init_start = 0;

    if(g_init_start == 0) {
        init_tvms_msg();
        g_init_start = 1;
        TR069_STATISTIC_PERIOD_RESTART();
    }
#endif
    return true;
}

bool
NativeHandlerPublicC10::doNtpSyncError()
{
    Message *msg = NULL;

    if(APPMODE_DVBS == sys_appmode_get() || NativeHandler::Upgrade == state() || NativeHandler::Boot == state())
        return false;

    if(mNtpDnsError) { // ErrorCode_NTP_DNSFail
        if(mNtpsyncerror == 0){
            msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptErrorCode10031, 1);
            epgBrowserAgent().sendMessage(msg);
            mNtpsyncerror = 1;
        }
        msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptErrorCode10031, 0);
        TR069_API_SETVALUE("ErrorCode", "", 10031);
        char ifname[USER_LEN] = { 0 };
        char dns0[USER_LEN] = { 0 };
        network_default_ifname(ifname, USER_LEN);
        LogRunOperError("Error code:10031,Description: NTP time synchronization fails because a DNS connection timeout occurs. DNS:%s\n",
            network_dns0_get(ifname, dns0, USER_LEN));
     } else { // ErrorCode_NTP_NotDNSFail
        if(mNtpsyncerror == 0){
            msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptErrorCode10030, 1);
            epgBrowserAgent().sendMessage(msg);
            mNtpsyncerror = 1;
        }
        msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptErrorCode10030, 0);
        TR069_API_SETVALUE("ErrorCode", "", 10030);
        char ntpDomain[USER_LEN] = {0};

        sysSettingGetString("ntp", ntpDomain, USER_LEN, 0);
        LogRunOperError("Error code:10030,Description: NTP server connection fails due to non-DNS causes. NTP:%s\n", ntpDomain);
    }

    mNtpDnsError = 0;
    epgBrowserAgent().sendMessageDelayed(msg, 5000);
    return true;
}

bool
NativeHandlerPublicC10::doDNSServerNotfound()
{
    mNtpDnsError = 1;
    return true;
}

bool
NativeHandlerPublicC10::doDNSResolveError()
{
    mNtpDnsError = 1;
    return true;
}

#if defined(INCLUDE_LocalPlayer)
bool
NativeHandlerPublicC10::doDeskTop()
{
    systemManager().destoryAllPlayer();
    epgBrowserAgent().openUrl(LOCAL_WEBPAGE_PATH_PREFIX"/LocalPlayer/menu.html");
    defNativeHandler().setState(NativeHandler::Local);
    return true;
}
#endif

#if defined (Liaoning) && defined (hi3560E)
bool
NativeHandlerPublicC10::doSwitch()
{
    webchannelFlagSet(0);
    doOpenConfigPage();
    return true;
}
#endif

#if defined(Jiangsu)
bool
NativeHandlerPublicC10::doOpenSerialNumberPaga()
{
    char mid_url[URL_MAX_LEN + 4] = { 0 };
    strncpy(mid_url, LOCAL_WEBPAGE_PATH_PREFIX"/serialNumInput.html", URL_MAX_LEN);
    epgBrowserAgent().openUrl(mid_url);
    return true;
}
#endif


bool
NativeHandlerPublicC10::doOpenMaintenancePage()
{
    if (mDialog == 0) {
        keyDispatcher().setEnabled(false);

        mDialog = new VisualizationDialog();
        ((VisualizationDialog *)mDialog)->setHandler(this);
        mDialog->draw();
    } else
        mDialog->draw();

    return true;
}

bool
NativeHandlerPublicC10::doCloseMaintenancePage()
{
    if(mDialog) {
        delete mDialog;
        mDialog = 0;
        keyDispatcher().setEnabled(true);
    }
    return true;
}

bool
NativeHandlerPublicC10::doDealIMEKey()
{
    if (!mKeyTimes) {
        mLastKeyDownPressed = Hippo::keyDispatcher().totalPressKeyTimes();
        mKeyTimes++;
        timeFirsePressed = mid_time();
        return true;
    }

    if (((mLastKeyDownPressed + 1) == Hippo::keyDispatcher().totalPressKeyTimes())
        && ((mid_time() - timeFirsePressed) <= 5)) {
        mKeyTimes++;
        mLastKeyDownPressed++;
        if (4 == mKeyTimes) {
            doOpenMaintenancePage();
            mKeyTimes = 0;
        }
    } else {
        mKeyTimes = 1;
        LogRunOperError("May be over 5s, actual time is %u s\n", mid_time() - timeFirsePressed);
        timeFirsePressed = mid_time();
        mLastKeyDownPressed = Hippo::keyDispatcher().totalPressKeyTimes();
    }

    return true;
}

} // namespace Hippo

