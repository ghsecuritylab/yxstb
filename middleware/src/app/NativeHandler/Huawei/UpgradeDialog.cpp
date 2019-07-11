
#include "UpgradeDialog.h"

#include "Message.h"
#include "MessageTypes.h"
#include "MessageValueNetwork.h"

#include "KeyDispatcher.h"
#include "NativeHandlerAssertions.h"
#include "NativeHandlerPublic.h"

#include "SystemManager.h"
#include "UpgradeManager.h"
#include "LayerMixerDevice.h"
#include "browser_event.h"
#include "config.h"

#include "Tr069.h"

#define UPGRADE_POS_X (((720-400)/2)&0xfffe)
#define UPGRADE_POS_Y ((((576-80)/2)&0xfffe)-80)

#define GIF_TR069_UPGRADE_BK_PATH      SYS_IMG_PATH_ROOT"/upgrade/tr069_upgrade/tr069_upgrade_bg.png"
#define GIF_STANDBY_OK_PATH            SYS_IMG_PATH_ROOT"/standby/standby_ok.png"
#define GIF_STANDBY_OK_1_PATH          SYS_IMG_PATH_ROOT"/standby/standby_ok1.png"
#define GIF_STANDBY_CANCEL_PATH        SYS_IMG_PATH_ROOT"/standby/standby_cancel.png"
#define GIF_STANDBY_CANCEL_1_PATH      SYS_IMG_PATH_ROOT"/standby/standby_cancel1.png"


namespace Hippo {
static WidgetSource UpgradeOkSource = {StandardScreen::S576, UPGRADE_POS_X+75, UPGRADE_POS_Y+85, 120, 46, 0, (void *)GIF_STANDBY_OK_PATH, 0};
static WidgetSource UpgradeOk1Source = {StandardScreen::S576, UPGRADE_POS_X+75, UPGRADE_POS_Y+85, 120, 46, 0, (void *)GIF_STANDBY_OK_1_PATH, 0};
static WidgetSource UpgradeCancelSource = {StandardScreen::S576, UPGRADE_POS_X+205, UPGRADE_POS_Y+85, 120, 46, 0, (void *)GIF_STANDBY_CANCEL_PATH, 0};
static WidgetSource UpgradeCancek1Source = {StandardScreen::S576, UPGRADE_POS_X+205, UPGRADE_POS_Y+85, 120, 46, 0, (void *)GIF_STANDBY_CANCEL_1_PATH, 0};
static WidgetSource UpgradeBackgroundSource = {StandardScreen::S576, UPGRADE_POS_X, UPGRADE_POS_Y, 400, 150, 0, (void *)GIF_TR069_UPGRADE_BK_PATH, 0};

UpgradeDialog::UpgradeDialog()
	: m_dialogFocus(false)
	, UpgradeOK(&UpgradeOkSource)
	, UpgradeOK1(&UpgradeOk1Source)
	, UpgradeCancel(&UpgradeCancelSource)
	, UpgradeCancel1(&UpgradeCancek1Source)
	, UpgradeBackground(&UpgradeBackgroundSource)
{
    LayerMixerDevice::Layer* layer = Hippo::systemManager().mixer().topLayer();


    layer->attachChildToFront(&UpgradeBackground);
    layer->attachChildToFront(&UpgradeOK);
    layer->attachChildToFront(&UpgradeOK1);
    layer->attachChildToFront(&UpgradeCancel);
    layer->attachChildToFront(&UpgradeCancel1);
}

UpgradeDialog::~UpgradeDialog()
{
    UpgradeBackground.detachFromParent();
    UpgradeOK.detachFromParent();
    UpgradeOK1.detachFromParent();
    UpgradeCancel.detachFromParent();
    UpgradeCancel1.detachFromParent();
}

bool
UpgradeDialog::handleMessage(Message *msg)
{
    if(MessageType_Repaint != msg->what)
        NATIVEHANDLER_LOG("what(0x%x),message(0x%x/0x%x)\n", msg->what, msg->arg1, msg->arg2);

    if(MessageType_Network == msg->what) {
        switch(msg->arg1) {
        case MV_Network_PhysicalUp:
        case MV_Network_PhysicalDown:
            return false;
        }
    }
    switch (msg->arg1) {
        case EIS_IRKEY_SELECT:
            if (true == m_dialogFocus) {
                UpgradeOK1.setVisibleP(false);
                UpgradeCancel.setVisibleP(false);
            } else {
                UpgradeOK.setVisibleP(false);
                UpgradeCancel1.setVisibleP(false);
            }
            UpgradeBackground.setVisibleP(false);

            if (true == m_dialogFocus)
                upgradeManager()->responseEvent(UpgradeManager::UMUT_IP_SOFTWARE, true);
            if (false == m_dialogFocus) {
                TR069_API_SETVALUE((char*)"Event.Upgraded", NULL, -1);
                upgradeManager()->responseEvent(UpgradeManager::UMUT_IP_SOFTWARE, false);
            }
            /* 恢复键值表功能 */
            if (!NativeHandlerPublic::mProgressBarDialog)
                keyDispatcher().setEnabled(true);
            return false;

        case EIS_IRKEY_LEFT:
            m_dialogFocus = true;
            UpgradeCancel1.setVisibleP(false);
            UpgradeOK.setVisibleP(false);
            UpgradeCancel.setVisibleP(true);
            UpgradeOK1.setVisibleP(true);
            break;

        case EIS_IRKEY_RIGHT:
            m_dialogFocus = false;
            UpgradeCancel.setVisibleP(false);
            UpgradeOK1.setVisibleP(false);
            UpgradeCancel1.setVisibleP(true);
            UpgradeOK.setVisibleP(true);
            break;
        default:
            break;
    }
    return true;
}

void
UpgradeDialog::draw()
{
    m_dialogFocus = true;
    UpgradeBackground.setVisibleP(true);
    UpgradeOK1.setVisibleP(true);
    UpgradeCancel.setVisibleP(true);
}

} // namespace Hippo

