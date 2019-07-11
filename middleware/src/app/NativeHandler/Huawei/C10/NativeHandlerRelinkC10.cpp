
#include <iostream>

#include "NativeHandlerRelinkC10.h"
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

void
NativeHandlerRelinkC10::onActive()
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
NativeHandlerRelinkC10::onUnactive()
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
NativeHandlerRelinkC10::handleMessage(Message *msg)
{
    if(MessageType_Repaint != msg->what)
        NATIVEHANDLER_LOG("what(0x%x), info(0x%x & 0x%x)\n", msg->what, msg->arg1, msg->arg2);
        
    if (MessageType_KeyDown == msg->what) {
        switch(msg->arg1) {
            case EIS_IRKEY_MENU:
                NATIVEHANDLER_LOG("Network is connecting\n");
                return true;
            default:
                NATIVEHANDLER_LOG("do nothing!\n");
            break;
        }
    }

    return NativeHandlerPublicC10::handleMessage(msg);
}
} // namespace Hippo
