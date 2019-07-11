
#include <iostream>

#include "NativeHandlerUpgradeC10.h"
#include "NativeHandlerAssertions.h"

#include "Message.h"
#include "MessageTypes.h"
#include "MessageValueSystem.h"
#include "BrowserAgent.h"

#include "Tr069.h"

#include "mid_fpanel.h"

namespace Hippo {

NativeHandlerUpgradeC10::NativeHandlerUpgradeC10()
{
}

NativeHandlerUpgradeC10::~NativeHandlerUpgradeC10()
{
}

bool
NativeHandlerUpgradeC10::handleMessage(Message *msg)
{
    if(MessageType_Repaint != msg->what)
        NATIVEHANDLER_LOG("what(0x%x), info(0x%x & 0x%x)\n", msg->what, msg->arg1, msg->arg2);

    if(MessageType_KeyDown == msg->what){
        switch(msg->arg1){
            case EIS_IRKEY_LEFT:
            case EIS_IRKEY_RIGHT:
            case EIS_IRKEY_SELECT:
                return false;
            default:
                return true;
        }
    }

    if (NativeHandlerPublicC10::handleMessage(msg))
        return true;

    switch (msg->arg1) {
        case EIS_IRKEY_POWER:	//power
        case EIS_IRKEY_VK_F10:
        case MV_System_OpenConfigPage:
            return true;
        default:
            return false;
    }
}

#ifdef INCLUDE_TR069
bool
NativeHandlerUpgradeC10::onUpgradeTr069Request()
{
    //STB在接收到网管的升级命令时，先判断当前是否正在升级，如果是上报错
    TR069_API_SETVALUE((char*)"Event.Upgraded", NULL, UPGRADE_IS_RUNNING_ALREADY);
    return true;
}
#endif

void
NativeHandlerUpgradeC10::onActive(void)
{
    Message *msg = NULL;

    // Don't display network disconnect error code in the upgrade state
    msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptDisconnectAlert, 0);
    epgBrowserAgent().sendMessage(msg);
    msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptWifiDisconnectAlert, 0);
    epgBrowserAgent().sendMessage(msg);

    //Network mask set
    DONT_SET(NMB_SHOW_ERROR_CODE);
    DONT_SET(NMB_SHOW_BOTTOM_ICON);
}

void
NativeHandlerUpgradeC10::onUnactive(void)
{
    Message *msg = NULL;

    // Display network disconnect error code when leave the upgrade state
    msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptDisconnectAlert, 1);
    epgBrowserAgent().sendMessage(msg);
    msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptWifiDisconnectAlert, 1);
    epgBrowserAgent().sendMessage(msg);

    //Network mask clear
    DONT_ZERO();
}

} // namespace Hippo
