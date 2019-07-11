
#include "NativeHandlerErrorC10.h"
#include "NativeHandlerAssertions.h"

#include "BrowserAgent.h"

#include "SystemManager.h"

#include "MessageTypes.h"

#include "config/webpageConfig.h"
#include "BootImagesShow.h"

#include "NetworkFunctions.h"

#if defined(Gansu)
#include "SysSetting.h"
#endif

#include "app_sys.h"
#include "mid_fpanel.h"

namespace Hippo {

NativeHandlerErrorC10::NativeHandlerErrorC10()
{
}

NativeHandlerErrorC10::~NativeHandlerErrorC10()
{
}

void
NativeHandlerErrorC10::onActive()
{
    Message *msg = NULL;

    if(defNativeHandler().getOldState() == NativeHandler::Running) {
        systemManager().destoryAllPlayer();
    }

    // When errorpage is open, then ,close video bootlogo and authlogo.
    BootImagesShowAuthLogo(0);

    // Don't display network disconnect error code in the error state
    msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptDisconnectAlert, 0);
    epgBrowserAgent().sendMessage(msg);
    msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptWifiDisconnectAlert, 0);
    epgBrowserAgent().sendMessage(msg);

    //Network mask set
    DONT_SET(NMB_SHOW_BOTTOM_ICON);
    DONT_SET(NMB_CONNECTOK_DOMENU);
    DONT_SET(NMB_REJOIN_WIRELESS);
}

void
NativeHandlerErrorC10::onUnactive()
{
    Message *msg = NULL;

    // Display network disconnect error code when leave the error state
    msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptDisconnectAlert, 1);
    epgBrowserAgent().sendMessage(msg);
    msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptWifiDisconnectAlert, 1);
    epgBrowserAgent().sendMessage(msg);

    //Network mask clear
    DONT_ZERO();
}

bool
NativeHandlerErrorC10::handleMessage(Message *msg)
{
    Message *tempMsg = NULL;

    if(MessageType_Repaint != msg->what)
        NATIVEHANDLER_LOG("what(0x%x), info(0x%x & 0x%x)\n", msg->what, msg->arg1, msg->arg2);

    tempMsg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptDisconnectAlert, 0);
    epgBrowserAgent().sendMessage(tempMsg);

    return NativeHandlerPublicC10::handleMessage(msg);
}

bool
NativeHandlerErrorC10::doShortCut(Message *)
{
    return false;
}

bool
NativeHandlerErrorC10::doMenu()
{
    std::string mid_url = "";

    systemManager().destoryAllPlayer();
    if(mid_sys_boot_get())
    {
        mid_url = LOCAL_WEBPAGE_PATH_PREFIX LOCAL_WEBPAGE_PATH_BOOT;
        defNativeHandler().setState(NativeHandler::Boot);
        epgBrowserAgent().openUrl(mid_url.c_str());
#if defined(Gansu)
        int connectType = 0;
        sysSettingGetInt("connecttype", &connectType, 0);
        char devname[USER_LEN] = { 0 };
        network_default_devname(devname, USER_LEN);
        NATIVEHANDLER_LOG("mid_net_link_state=%d, connectType=%d!\n", network_device_link_state(devname), connectType);
        if ((1 == network_device_link_state(ifname)) && (connectType == 1))
             epgBrowserAgent().openUrl(LOCAL_WEBPAGE_PATH_PREFIX"/check_pppoeaccount.html");
         else
             epgBrowserAgent().openUrl(mid_url.c_str());
 #endif

    } else {
        NativeHandlerPublicC10::doMenu();
    }
	return true;
}

} // namespace Hippo
