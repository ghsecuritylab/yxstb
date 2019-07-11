#include "NativeHandlerRecovery.h"
#include "NativeHandlerAssertions.h"
#include "Message.h"
#include "MessageTypes.h"
#include "EmergencyDialog.h"
#include "UpgradeManager.h"
#include "BrowserAgent.h"
#include "NetworkFunctions.h"

#include "AppSetting.h"
#include "SysSetting.h"
#include "BootImagesShow.h"
#include "sys_basic_macro.h"

#include "config.h"
#include "ipanel_event.h"

extern "C" int iptv_appInit();

namespace Hippo {

NativeHandlerRecovery::NativeHandlerRecovery()
    : mIsHandleKey(false)
    , mPressOKCount(0)
{

}

NativeHandlerRecovery::~NativeHandlerRecovery()
{

}

void
NativeHandlerRecovery::onActive()
{
    //DONT_SET(NMB_SHOW_ERROR_CODE);
    //DONT_SET(NMB_SHOW_BOTTOM_ICON);
    //DONT_SET(NMB_AUTO_DOMENU);
    //DONT_SET(NMB_REJOIN_WIRELESS);
}

void
NativeHandlerRecovery::onUnactive()
{
    //DONT_ZERO();
}

bool
NativeHandlerRecovery::handleMessage(Message *msg)
{
    NATIVEHANDLER_LOG_VERBOSE("what = 0x%x, arg1 = 0x%x, arg2 = 0x%x\n", msg->what, msg->arg1, msg->arg2);
    if(msg->what == MessageType_KeyDown) {
        if (msg->arg1 == EIS_IRKEY_SELECT) {
            if (mIsHandleKey)
                onEmergency();
        } else {
            if (mDialog)
                mDialog->handleMessage(msg);
        }
    } else if (msg->what == MessageType_Timer) {
        if (msg->arg1 == 0) //Set TimeOut intval
            checkPressKey();
        else //Receive "OK" TimeOut
            iptv_appInit();
    } else if (msg->what == MessageType_Upgrade) {
        if (msg->arg1 == 0)
            startNetWork();
    } else if (msg->what == MessageType_Network) {
        if (msg->arg1 == NETWORK_CONNECT_OK)
            upgradeManager()->touchOffUpgradeCheck(UpgradeManager::UMUT_IP_SOFTWARE, false);
    } else if (msg->what == MessageType_Unknow || msg->what == MessageType_System) {
        if (msg->arg1 == LITTLE_SYSTEM_SERVER_UPGRADE_FILAID
            ||msg->arg1 == LITTLE_SYSTEM_UDISK_UPGRADE_FILAID) {
            appSettingSetInt("recoveryMode", -1);
            BootImagesShowBootLogo(1);//显示第二张logo
            iptv_appInit();
        } else {
            return NativeHandlerPublic::handleMessage(msg);
        }
    } else {
        return NativeHandlerPublic::handleMessage(msg);
    }

    return true;
}

int
NativeHandlerRecovery::checkPressKey()
{
    mPressOKCount = 0;
    mIsHandleKey = true;
    Message *msg = defNativeHandler().obtainMessage(MessageType_Timer, 1, 0);
    defNativeHandler().sendMessageDelayed(msg, 1500);

    return 0;
}

void
NativeHandlerRecovery::onEmergency()
{
    NATIVEHANDLER_LOG_VERBOSE("mPressOKCount = %d\n", mPressOKCount);
    if (mPressOKCount == 0) {
        defNativeHandler().removeMessages(MessageType_Timer);
        Message *msg = defNativeHandler().obtainMessage(MessageType_Timer, 1, 0);
        defNativeHandler().sendMessageDelayed(msg, 2000);
        mPressOKCount = 1;
        return;
    }
    mPressOKCount++;
    if (mPressOKCount == 3) {
        mIsHandleKey = false;
        mPressOKCount = 0;
        defNativeHandler().removeMessages(MessageType_Timer);
        BootImagesShowBootLogo(0);//隐藏第二张logo
        mDialog = new EmergencyDialog(0);
        mDialog->draw();
    }
}
void
NativeHandlerRecovery::startNetWork()
{
    char ifname[USER_LEN] = { 0 };
    NetworkInterface* iface = networkManager().getInterface(network_default_ifname(ifname, USER_LEN));
    if (iface)
        iface->connect();
}

}

