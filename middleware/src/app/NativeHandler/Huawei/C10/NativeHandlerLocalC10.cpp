
#include "NativeHandlerLocalC10.h"
#include "NativeHandlerAssertions.h"

#include "Message.h"
#include "MessageTypes.h"
#include "BrowserAgent.h"

#include "SystemManager.h"

#include "NetworkFunctions.h"
#include "AppSetting.h"
#include "SysSetting.h"

#include "config/webpageConfig.h"
#include "app_sys.h"
#include "codec.h"

#ifdef HUBEI_HD
#ifdef INCLUDE_DMR
#include "mid_dlna_ex.h"
#endif
#endif


namespace Hippo {

NativeHandlerLocalC10::NativeHandlerLocalC10()
{
}

NativeHandlerLocalC10::~NativeHandlerLocalC10()
{
}

bool
NativeHandlerLocalC10::handleMessage(Message *msg)
{
    if(MessageType_Repaint != msg->what)
        NATIVEHANDLER_LOG("what(0x%x), info(0x%x & 0x%x)\n", msg->what, msg->arg1, msg->arg2);

    return NativeHandlerPublicC10::handleMessage(msg);
}

bool
NativeHandlerLocalC10::doMenu()
{
    std::string mid_url = "";

    systemManager().destoryAllPlayer();

    NATIVEHANDLER_LOG("Open menu page from localplayer, boot flag is %d\n", mid_sys_boot_get());
    if (!mid_sys_boot_get()) {
#ifdef HUBEI_HD // 湖北现网EPG为标清，但是没有设置frame size，切换会不全屏
        TAKIN_porting_set_frame_size(TAKIN_FRAME_SIZE, "640*530", strlen("640*530") + 1);
#endif
        /* webpage boot.html is already loaded (network already init), so only open menu page. add. by Michael 2012/10/11 */
        NativeHandlerPublicC10::doMenu();
    } else {
        mid_url = LOCAL_WEBPAGE_PATH_PREFIX LOCAL_WEBPAGE_PATH_BOOT;
        defNativeHandler().setState(NativeHandler::Boot);
        epgBrowserAgent().openUrl(mid_url.c_str());
    }

    return true;
}

bool
NativeHandlerLocalC10::doShortCut(Message *msg)
{
    return false;
}

void
NativeHandlerLocalC10::onActive(void)
{
    Message *msg = NULL;

#ifdef HUBEI_HD
#ifdef INCLUDE_DMR
    mid_dlna_init();
    mid_dlna_start(0);
#endif
#endif

    //Don't display network disconnect icon in the localplayer state
    msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptDisconnect, 0);
    epgBrowserAgent().sendMessage(msg);
    msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptWifiDisconnect, 0);
    epgBrowserAgent().sendMessage(msg);

    // Don't display network disconnect error code in the localplayer state
    msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptDisconnectAlert, 0);
    epgBrowserAgent().sendMessage(msg);
    msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptWifiDisconnectAlert, 0);
    epgBrowserAgent().sendMessage(msg);

    //Network mask set
    DONT_SET(NMB_SHOW_BOTTOM_ICON);
    DONT_SET(NMB_REJOIN_WIRELESS);
}

void
NativeHandlerLocalC10::onUnactive(void)
{
    Message *msg = NULL;
	int volume = 0;

#ifdef HUBEI_HD
#ifdef INCLUDE_DMR
    mid_dlna_stop();
#endif
#endif

    // Display network disconnect icon when leave the localplayer state
    msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptDisconnect, 1);
    epgBrowserAgent().sendMessage(msg);
    msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptWifiDisconnect, 1);
    epgBrowserAgent().sendMessage(msg);

    // Display network disconnect error code when leave the boot state
    msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptDisconnectAlert, 1);
    epgBrowserAgent().sendMessage(msg);
    msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptWifiDisconnectAlert, 1);
    epgBrowserAgent().sendMessage(msg);

	appSettingGetInt("volume", &volume, 0);
	if(volume < 0) {
		volume = 0;
	}
	else if(volume > AUDIO_VOLUME_MAX) {
		volume = AUDIO_VOLUME_MAX;
	}
    if(volume % 10 < 5) {
        volume = volume - volume % 10;
    } else {
        volume = volume - volume % 10 + 10;
    }
    appSettingSetInt("volume", volume);
    mid_audio_volume_set(volume);

    //Network mask clear
    DONT_ZERO();
    return ;
}

} // namespace Hippo

