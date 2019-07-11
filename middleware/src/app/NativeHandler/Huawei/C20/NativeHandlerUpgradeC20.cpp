
#include <iostream>

#include "NativeHandlerUpgradeC20.h"
#include "NativeHandlerAssertions.h"

#include "Message.h"
#include "MessageTypes.h"
#include "MessageValueSystem.h"

#include "Tr069.h"



namespace Hippo {

NativeHandlerUpgradeC20::NativeHandlerUpgradeC20()
{
}

NativeHandlerUpgradeC20::~NativeHandlerUpgradeC20()
{
}

void
NativeHandlerUpgradeC20::onActive()
{
    DONT_SET(NMB_SHOW_ERROR_CODE);
    DONT_SET(NMB_SHOW_BOTTOM_ICON);
    //DONT_SET(NMB_CONNECTOK_DOMENU);
    DONT_SET(NMB_REJOIN_WIRELESS);
}

void
NativeHandlerUpgradeC20::onUnactive()
{
    DONT_ZERO();
}


bool
NativeHandlerUpgradeC20::handleMessage(Message *msg)
{
    NATIVEHANDLER_LOG("NativeHandlerUpgrade::handleMessage: what(0x%x), info(0x%x & 0x%x)\n", msg->what, msg->arg1, msg->arg2);
    if(MessageType_KeyDown == msg->what)
        return true;

    switch (msg->arg1) {
        case EIS_IRKEY_POWER: // power
        case EIS_IRKEY_FPANELPOWER:
        case MV_System_OpenConfigPage:
            break;
        default:break;
    }

    if (NativeHandlerPublicC20::handleMessage(msg))
        return true;

    return true;
}

bool
NativeHandlerUpgradeC20::onUpgradeTr069Request()
{
    //STB在接收到网管的升级命令时，先判断当前是否正在升级，如果是上报错
    TR069_API_SETVALUE((char*)"Event.Upgraded", NULL, UPGRADE_IS_RUNNING_ALREADY);
    return true;
}

} // namespace Hippo
