
#include <iostream>

#include "NativeHandlerUConfigC10.h"
#include "NativeHandlerAssertions.h"

#include "Message.h"
#include "MessageTypes.h"
#include "MessageValueSystem.h"

#include "SystemManager.h"
#include "StandardScreen.h"

#include "BrowserAgent.h"

#include "config.h"


extern int gUnzipStatus;

namespace Hippo {

NativeHandlerUConfigC10::NativeHandlerUConfigC10()
{
}

NativeHandlerUConfigC10::~NativeHandlerUConfigC10()
{
}

void
NativeHandlerUConfigC10::onActive()
{
    Message *msg = NULL;

    // Don't display network disconnect error code in the uconfig state
    msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptDisconnectAlert, 0);
    epgBrowserAgent().sendMessage(msg);
    msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptWifiDisconnectAlert, 0);
    epgBrowserAgent().sendMessage(msg);

    //Network mask set
    DONT_SET(NMB_SHOW_ERROR_CODE);
    DONT_SET(NMB_SHOW_BOTTOM_ICON);
}

void
NativeHandlerUConfigC10::onUnactive()
{
    Message *msg = NULL;

    // Display network disconnect error code when leave the uconfig state
    msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptDisconnectAlert, 1);
    epgBrowserAgent().sendMessage(msg);
    msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptWifiDisconnectAlert, 1);
    epgBrowserAgent().sendMessage(msg);

    //Network mask clear
    DONT_ZERO();
}

bool
NativeHandlerUConfigC10::handleMessage(Message *msg)
{
    NATIVEHANDLER_LOG("NativeHandlerUConfigC10::handleMessage what(0x%x), info(0x%x & 0x%x)\n", msg->what, msg->arg1, msg->arg2);

    if (msg->what == MessageType_Repaint) {
#if defined(hi3560e)
        systemManager().mixer().display();
#else
        systemManager().mixer().refresh();
#endif
        return true;
    }
    /*else if (msg->what == MessageType_KeyDown)*/ {
        switch (msg->arg1) {
            case EIS_IRKEY_NUM0:
            case EIS_IRKEY_NUM1:
            case EIS_IRKEY_NUM2:
            case EIS_IRKEY_NUM3:
            case EIS_IRKEY_NUM4:
            case EIS_IRKEY_NUM5:
            case EIS_IRKEY_NUM6:
            case EIS_IRKEY_NUM7:
            case EIS_IRKEY_NUM8:
            case EIS_IRKEY_NUM9:
            case EIS_IRKEY_UP:
            case EIS_IRKEY_DOWN:
            case EIS_IRKEY_LEFT:
            case EIS_IRKEY_RIGHT:
            case EIS_IRKEY_SELECT:
            case EIS_IRKEY_BACK:
            case EIS_IRKEY_IME:
            case EIS_IRKEY_STAR:
            case EIS_IRKEY_PAGE_UP:
            case EIS_IRKEY_PAGE_DOWN:
            case EIS_IRKEY_POWER:
                return false;

            case USB_UPGRADE_OK:
            case USB_CONFIG_OK:
            case MV_System_OpenUnzipErrorPage:
                return NativeHandlerPublicC10::handleMessage(msg);
                
      #if defined(Gansu)
            case MV_System_UnzipErr2Restart:
                return NativeHandlerPublicC10::handleMessage(msg);
      #endif
      
            case EIS_IRKEY_USB_UNINSERT:{
                gUnzipStatus = 3;
                return true;
            }

            default:
                return true;
        }
    }
}
} // namespace Hippo
