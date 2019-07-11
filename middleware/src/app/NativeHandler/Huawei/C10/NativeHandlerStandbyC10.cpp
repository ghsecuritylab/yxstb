
#include <iostream>
#include <unistd.h> //for usleep

#include "NativeHandlerStandbyC10.h"
#include "NativeHandlerAssertions.h"
#include "BootImagesShow.h"

#include "Message.h"
#include "MessageTypes.h"
#include "MessageValueSystem.h"
#include "MessageValueNetwork.h"
#include "BrowserEventQueue.h"

#include "SystemManager.h"

#if defined(ANDROID)
#include "IPTVMiddleware.h"
#include "libzebra.h"
#endif

#include "middle/mid_fpanel.h"

#include "AppSetting.h"
#include "config.h"
#include "BrowserAgent.h"


namespace Hippo {

NativeHandlerStandbyC10::NativeHandlerStandbyC10()
{
}

NativeHandlerStandbyC10::~NativeHandlerStandbyC10()
{
}

void
NativeHandlerStandbyC10::onActive(void)
{
}

void
NativeHandlerStandbyC10::onUnactive(void)
{
}

extern "C" int Hybroad_Inconsistent_getIRtype();
bool
NativeHandlerStandbyC10::handleMessage(Message *msg)
{
    if(MessageType_Repaint != msg->what)
        NATIVEHANDLER_LOG("NativeHandlerStandby::handleMessage what(0x%x), info(0x%x & 0x%x)\n", msg->what, msg->arg1, msg->arg2);

    if(msg->what == MessageType_Repaint) {
#if defined(hi3560e)
        systemManager().mixer().display();
#else
        systemManager().mixer().refresh();
#endif
        return true;
    }
    else if(msg->what == MessageType_Unknow) { /**  300 event send to epg **/
        return false;
    }
    else if(msg->what == MessageType_System && msg->arg1 == HM_CLOSEDIALOG) {
        return NativeHandlerPublic::handleMessage(msg);
    }
    switch(msg->arg1) {
    case EIS_IRKEY_POWER:
    case EIS_IRKEY_FPANELPOWER:
#if !defined(ANDROID)
        if (Hybroad_Inconsistent_getIRtype() == 1)
            return true;
#endif
        return doPowerOn();
    case MV_System_OpenBootPage:
        return doOpenBootPage();

    case HEART_BIT_RUN:
        return doHeartBit();

    default:
        return true;
    }

}

bool
NativeHandlerStandbyC10::doPowerOn()
{
    NATIVEHANDLER_LOG("---------------doPowerOn------------\n");
#if defined(ANDROID)
    int lOldState = NativeHandlerGetOldState();
	defNativeHandler().setState(NativeHandler::State(lOldState));
    yhw_board_InitVideoPlayer();
    return true;
#endif
#if (defined(HUAWEI_C10) && defined(hi3716m))
    usleep(5000000);
    if (mid_real_standbyFlag_get())
        return true;
#endif
    epgBrowserAgentCleanTakinCache();

	mid_fpanel_standby_set(0);
	std::string tA2event = "{\"type\":\"EVENT_POWEROFF_WAKEUP\",\"last_channel_id\":\"";
	int lastChannelKey = 0;
    appSettingGetInt("lastChannelID", &lastChannelKey, 0);
	tA2event += ( lastChannelKey +"\"}");
	browserEventSend(tA2event.c_str(), NULL);
	return false;
}

} // namespace Hippo
