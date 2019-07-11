
#include <iostream>

#include "NativeHandlerStandbyC20.h"
#include "NativeHandlerAssertions.h"

#include "Message.h"
#include "MessageTypes.h"
#include "MessageValueSystem.h"
#include "MessageValueNetwork.h"
#include "BrowserEventQueue.h"
#include "UpgradeManager.h"


#include "SystemManager.h"
#include "BrowserAgent.h"
#include "KeyDispatcher.h"
#include "middle/mid_fpanel.h"

#include "AppSetting.h"
#include "config.h"

#include "codec.h"
#include "config/webpageConfig.h"

namespace Hippo {

NativeHandlerStandbyC20::NativeHandlerStandbyC20()
{
}

NativeHandlerStandbyC20::~NativeHandlerStandbyC20()
{
}

void
NativeHandlerStandbyC20::onActive()
{
    DONT_SET(NMB_SHOW_ERROR_CODE);
    //DONT_SET(NMB_SHOW_BOTTOM_ICON);
    //DONT_SET(NMB_CONNECTOK_DOMENU);
    DONT_SET(NMB_REJOIN_WIRELESS);
}

void
NativeHandlerStandbyC20::onUnactive()
{
    DONT_ZERO();
}


bool
NativeHandlerStandbyC20::handleMessage(Message *msg)
{
    NATIVEHANDLER_LOG("NativeHandlerStandby::handleMessage what(0x%x), info(0x%x & 0x%x)\n", msg->what, msg->arg1, msg->arg2);
    if(msg->what == MessageType_Repaint) {
        systemManager().mixer().refresh();
        return true;
    }
    else if (msg->what == MessageType_Unknow) {
        /**  300 event send to epg **/
        return false;
    }

    switch(msg->arg1) {
    case MV_System_OpenBootPage:
        return doOpenBootPage();

    case HEART_BIT_RUN:
        return doHeartBit();
    case EIS_IRKEY_POWER:
    case EIS_IRKEY_FPANELPOWER://front panel power can alse weak up light sleep
        return doPowerOn();
    default:
        return true;
    }
}

bool
NativeHandlerStandbyC20::doPowerOn()
{
	mid_fpanel_standby_set(0);
#ifdef INCLUDE_PIP
    codec_unlock();
#endif
	std::string a2_event = "{\"type\":\"EVENT_POWEROFF_WAKEUP\",\"last_channel_id\":\"";
	int lastChannelKey = 0;
    char buf[10] = {0};
    appSettingGetInt("lastChannelID", &lastChannelKey, 0);
    snprintf(buf, sizeof(buf), "%d", lastChannelKey);
	a2_event = a2_event + buf +"\"}";
	browserEventSend(a2_event.c_str(), NULL);
    int oldState = NativeHandlerGetOldState();
	defNativeHandler().setState(NativeHandler::State(oldState));
    upgradeManager()->touchOffUpgradeCheck(UpgradeManager::IP_SOFTWARE, false);

#ifdef VIETTEL_HD
	std::string mid_url = LOCAL_WEBPAGE_PATH_PREFIX LOCAL_WEBPAGE_PATH_BOOT;
	NATIVEHANDLER_LOG("NativeHandlerPublic::doOpenBootPage url(%s)\n", mid_url.c_str());
	epgBrowserAgent().closeAllConnection();
	defNativeHandler().setState(NativeHandler::Boot);
	epgBrowserAgent().openUrl(mid_url.c_str());
#endif
    sleep(3); //wait for HDMI Authentication
    removeSpecifiedMessageFromKeyDispatcher(MessageType_KeyDown, EIS_IRKEY_POWER, 0);
    removeSpecifiedMessageFromKeyDispatcher(MessageType_System, EIS_IRKEY_PAGE_STANDBY, 0);
	return true;
}

} // namespace Hippo
