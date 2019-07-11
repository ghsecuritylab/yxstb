
#include "NativeHandlerConfigC10.h"
#include "NativeHandlerAssertions.h"

#include "Message.h"
#include "MessageTypes.h"
#include "BrowserAgent.h"

#include "SystemManager.h"

#include "UltraPlayerWebPage.h"
#include "KeyDispatcher.h"
#include "browser_event.h"
#include "SysSetting.h"

#include "config/webpageConfig.h"
#include "app_sys.h"

#include "MessageTypes.h"
#include "MessageValueNetwork.h"
#include "NetworkFunctions.h"


namespace Hippo {

NativeHandlerConfigC10::NativeHandlerConfigC10()
{
}

NativeHandlerConfigC10::~NativeHandlerConfigC10()
{
}

void
NativeHandlerConfigC10::onActive()
{
    Message *msg = NULL;

    // Don't display network disconnect error code in the config state
    msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptDisconnectAlert, 0);
    epgBrowserAgent().sendMessage(msg);
    msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptWifiDisconnectAlert, 0);
    epgBrowserAgent().sendMessage(msg);

    //Network mask set
    DONT_SET(NMB_SHOW_BOTTOM_ICON);
}

void
NativeHandlerConfigC10::onUnactive()
{
    Message *msg = NULL;

    // Display network disconnect error code when leave the config state
    msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptDisconnectAlert, 1);
    epgBrowserAgent().sendMessage(msg);
    msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptWifiDisconnectAlert, 1);
    epgBrowserAgent().sendMessage(msg);

    //Network mask clear
    DONT_ZERO();
}

bool
NativeHandlerConfigC10::handleMessage(Message *msg)
{
    if(MessageType_Repaint != msg->what)
        NATIVEHANDLER_LOG("what(0x%x), info(0x%x & 0x%x)\n", msg->what, msg->arg1, msg->arg2);
    if((msg->what == MessageType_NetworkDiagnose) && (msg->arg1 == 0x9201))
        Hippo::defNativeHandler().setState((Hippo::NativeHandler::State)msg->arg2);

#if defined(Jiangsu)
    if(MessageType_Network == msg->what) {
        switch (msg->arg1) {
            case MV_Network_PhysicalDown:
                sendMessageToNativeHandler(MessageType_Network, MV_Network_PhysicalDown_OnOpenError, 0, 0);
                break;
            default:
                break;
        }
    }
#endif

    return NativeHandlerPublicC10::handleMessage(msg);
}


bool
NativeHandlerConfigC10::doMenu()
{
    std::string mid_url = "";

    systemManager().destoryAllPlayer();

#if defined(SHANGHAI_HD) || defined(SHANGHAI_SD)
    mid_url = app_EPGDomian_get();
    defNativeHandler().setState(NativeHandler::Running);
    epgBrowserAgent().openUrl(mid_url.c_str());
#else
    NATIVEHANDLER_LOG("Open menu page from config, boot flag is %d\n", mid_sys_boot_get());
    if (!mid_sys_boot_get()) {
        /* webpage boot.html is already loaded (network already init), so only open menu page. add. by Michael 2012/10/11 */


#ifdef Liaoning// ÁÉÄþNativeHandlerPublicC10::doMenu();Ö±½Óreturn false
        webchannelFlagSet(0);
        KeyDispatcherPolicy *keyPolocy = NULL;
        keyPolocy = keyDispatcher().getPolicy(EIS_IRKEY_MENU);
        if (keyPolocy) {
            NATIVEHANDLER_LOG("Get menu url(%s)\n", keyPolocy->mKeyUrl.c_str());
            mid_url = keyPolocy->mKeyUrl;
        }
        if (0 == strlen(mid_url.c_str())){
            char url[URL_MAX_LEN + 4] = { 0 };

            app_mainUrl_get(url);
            mid_url=url;
        }
        defNativeHandler().setState(NativeHandler::Running);
        epgBrowserAgent().openUrl(mid_url.c_str());
#else
        NativeHandlerPublicC10::doMenu();
#endif
    } else {
        mid_url = LOCAL_WEBPAGE_PATH_PREFIX LOCAL_WEBPAGE_PATH_BOOT;
        defNativeHandler().setState(NativeHandler::Boot);
        epgBrowserAgent().openUrl(mid_url.c_str());
    }
#endif

    return true;
}

} // namespace Hippo

