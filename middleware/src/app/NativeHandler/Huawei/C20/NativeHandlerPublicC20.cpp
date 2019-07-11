
#include "NativeHandlerPublicC20.h"
#include "NativeHandlerAssertions.h"
#include "LogModuleHuawei.h"

#include "Message.h"
#include "MessageTypes.h"
#include "MessageValueSystem.h"

#include "MessageValueNetwork.h"
#include "BrowserEventQueue.h"

#include "BrowserAgent.h"

#include "KeyDispatcher.h"
#include "PowerOffDialog.h"
#include "SystemManager.h"
#include "NetworkFunctions.h"

#include <iostream>

#include <hippo_module_config.h>
#include <Hippo_OS.h>
#include "Hippo_Context.h"
#include "Session.h"
#include "Business.h"

#include "codec.h"
#include "config.h"
#include "sys_key_deal.h"
#include "config/webpageConfig.h"
#include "mid_stream.h"
#include "mid_record.h"

#include "SysSetting.h"
#include "app/app_reminderlist.h"
#include "app/app_heartbit.h"
#include "app/app_epg_para.h"
#include "app/app_http.h"
#include "app_sys.h"

#include "middle/mid_fpanel.h"
#include "mid/mid_timer.h"
#include "mid/mid_http.h"
#include "middle/mid_task.h"
#include "ntp/mid_ntp.h"


#include "sys_msg.h"
#include "porting/tvms/tvms.h"
#include "Verimatrix.h"

#include "tr069/tr069_api.h"

#ifdef INCLUDE_cPVR
#include "Tstv.h"
#include "CpvrJsCall.h"
#endif
#if defined(INCLUDE_DVBS)
#include "DvbModule.h"
#endif
#include "app_c20_init.h"

#include "TAKIN_browser.h"

#if defined(BROWSER_INDEPENDENCE)
extern "C" int KillBrowserForPowerOff();
#endif


namespace Hippo {

int NativeHandlerPublicC20::mIpConflictFlag = 0;
int NativeHandlerPublicC20::mErrorCodeOfNTP = 0;

NativeHandlerPublicC20::NativeHandlerPublicC20()
{
}

NativeHandlerPublicC20::~NativeHandlerPublicC20()
{
}

bool
NativeHandlerPublicC20::handleMessage(Message *msg)
{
    if (msg->what == MessageType_KeyDown) {
        switch(msg->arg1) {
        case EIS_IRKEY_FPANELPOWER: {
            return doPowerOffDeep();
        default: break;
        }
        }
    }

    bool returnValue = NativeHandlerPublic::handleMessage(msg);
    if (returnValue != false)
        return returnValue;

    NATIVEHANDLER_LOG("NativeHandlerPublicC20::handleMessage what(%d), info(0x%x & 0x%x)\n", msg->what, msg->arg1, msg->arg2);

    return false;
}

bool NativeHandlerPublicC20::doPowerOff()
{
#ifdef VIETTEL_HD
    std::string mid_url = LOCAL_WEBPAGE_PATH_PREFIX LOCAL_WEBPAGE_PATH_STANDBY;
    epgBrowserAgent().openUrl(mid_url.c_str());
#endif
    systemManager().destoryAllPlayer();
#ifdef INCLUDE_PIP
    codec_lock();
#endif
    huaweiSetLogForceSend();

    mid_fpanel_standby_set(1);
    defNativeHandler().setState(NativeHandler::Standby);
    return true;
}

bool NativeHandlerPublicC20::doPowerOffDeep()
{
    huaweiSetLogForceSend();

    systemManager().destoryAllPlayer();
    settingManagerSave();
#if defined(BROWSER_INDEPENDENCE)
    KillBrowserForPowerOff();
#endif
    sleep(5);//important
    mid_fpanel_poweroffdeep();
    return true;
}

bool NativeHandlerPublicC20::doOpenStandbyPage()
{
    systemManager().destoryAllPlayer();
#ifdef INCLUDE_PIP
    codec_lock();
#endif
    huaweiSetLogForceSend();
    mid_fpanel_standby_set(1);
    defNativeHandler().setState(NativeHandler::Standby);
#ifdef VIETTEL_HD
    sendMessageToEPGBrowser(MessageType_KeyDown, EIS_IRKEY_BACK, 0, 0);
    std::string mid_url = LOCAL_WEBPAGE_PATH_PREFIX LOCAL_WEBPAGE_PATH_STANDBY;
    epgBrowserAgent().openUrl(mid_url.c_str());
	sleep(1);
#endif
    return true;
}

// NTP sync message
bool
NativeHandlerPublicC20::doNtpSyncOk()
{
    CA_VM_API_INIT();
    NATIVEHANDLER_LOG("NativeHandlerPublicC20::doNtpSyncOk\n");
    static int g_init_start = 0;
    if(g_init_start == 0) {
#ifdef TVMS_OPEN
        init_tvms_msg();
#endif
        g_init_start = 1;
        TR069_STATISTIC_PERIOD_RESTART();

#ifdef INCLUDE_cPVR
    cpvr_module_init();
    cpvrTaskResumeAfterReboot(1);
#endif
    }
#if defined(INCLUDE_DVBS)
    dvb_module_ntpsync(1);
#endif
    if(mErrorCodeOfNTP) {
		char errInfo[64];
	    snprintf(errInfo, 64, "{\"type\":\"EVENT_STB_RESTORE\",\"error_code\":\"%d\"}", mErrorCodeOfNTP);
        browserEventSend(errInfo, NULL);
        mErrorCodeOfNTP = 0;
        return true;
    }
    return false;
}

bool
NativeHandlerPublicC20::doNtpSyncError()
{
    if(APPMODE_DVBS != sys_appmode_get() && NativeHandler::Boot != state() && NativeHandler::Upgrade != state()) {
		char errInfo[64];
	    snprintf(errInfo, 64, "{\"type\":\"EVENT_STB_ERROR\",\"error_code\":\"%d\"}", ERROR_NETWORK_NTP_SYNCHRONIZE_FAILED);
        browserEventSend(errInfo, NULL);
        mErrorCodeOfNTP = ERROR_NETWORK_NTP_SYNCHRONIZE_FAILED;
        return true;
    }
    return false;
}

bool
NativeHandlerPublicC20::doDNSServerNotfound()
{
    if(APPMODE_DVBS != sys_appmode_get() && NativeHandler::Boot != state() && NativeHandler::Upgrade != state()) {
		char errInfo[64];
	    snprintf(errInfo, 64, "{\"type\":\"EVENT_STB_ERROR\",\"error_code\":\"%d\"}", ERROR_NETWORK_NTP_SERVER_DNS_CONNECET_FAILED);
        browserEventSend(errInfo, NULL);
        mErrorCodeOfNTP = ERROR_NETWORK_NTP_SERVER_DNS_CONNECET_FAILED;
        return true;
    }
    return false;
}

bool
NativeHandlerPublicC20::doDNSResolveError()
{
    if(APPMODE_DVBS != sys_appmode_get() && NativeHandler::Boot != state() && NativeHandler::Upgrade != state()) {
		char errInfo[64];
	    snprintf(errInfo, 64, "{\"type\":\"EVENT_STB_ERROR\",\"error_code\":\"%d\"}", ERROR_NETWORK_NTP_SERVER_DNS_RESOLVED_FAILED);
        browserEventSend(errInfo, NULL);
        mErrorCodeOfNTP = ERROR_NETWORK_NTP_SERVER_DNS_RESOLVED_FAILED;
        return true;
    }
    return false;
}

bool
NativeHandlerPublicC20::doLocalTS(Message *msg)
{
#ifdef INCLUDE_cPVR
    TstvDealRecordMsg(msg->arg1);
#endif
    return true;
}

bool
NativeHandlerPublicC20::doConfig(Message *msg)
{
#if defined(Cameroon_v5)
    systemManager().destoryAllPlayer();
    std::string mid_url =  LOCAL_WEBPAGE_PATH_PREFIX LOCAL_WEBPAGE_PATH_CONFIG;
    epgBrowserAgent().openUrl(mid_url.c_str());
    defNativeHandler().setState(NativeHandler::Config);
    return true;
#else
    return false;
#endif
}

bool
NativeHandlerPublicC20::doMenu()
{
    int connecttype = 0;

    NATIVEHANDLER_LOG_WARNING("//TODO Static not call mid_ntp_time_sync(). Test is ok.\n");
    char ifname[URL_LEN] = { 0 };
    NetworkInterface* iface = networkManager().getInterface(network_default_ifname(ifname, URL_LEN));
    if (iface && !iface->isActive())
        iface->connect();
    return false;
}

} // namespace Hippo

