
#include "RestartDialog.h"

#include "Message.h"
#include "MessageTypes.h"
#include "MessageValueNetwork.h"

#include "KeyDispatcher.h"
#include "NativeHandlerAssertions.h"

#include "SystemManager.h"
#include "LayerMixerDevice.h"
#include "browser_event.h"
#include "config.h"

#include "mid_fpanel.h"
#define RESTART_POS_X (((720-400)/2)&0xfffe)
#define RESTART_POS_Y ((((576-80)/2)&0xfffe)-80)


#define PNG_TR069_UPGRADE_RESTARTBK_PATH   SYS_IMG_PATH_ROOT"/upgrade/tr069_upgrade/tr069_restart_bg.png"
#define PNG_STANDBY_OK_PATH           SYS_IMG_PATH_ROOT"/standby/standby_ok.png"
#define PNG_STANDBY_OK_1_PATH         SYS_IMG_PATH_ROOT"/standby/standby_ok1.png"
#define PNG_STANDBY_CANCEL_PATH       SYS_IMG_PATH_ROOT"/standby/standby_cancel.png"
#define PNG_STANDBY_CANCEL_1_PATH     SYS_IMG_PATH_ROOT"/standby/standby_cancel1.png"


namespace Hippo {

static WidgetSource RestartOkSource = {StandardScreen::S576, RESTART_POS_X+75, RESTART_POS_Y+85, 120, 46, 0, (void *)PNG_STANDBY_OK_PATH, 0};
static WidgetSource RestartOk1Source = {StandardScreen::S576, RESTART_POS_X+75, RESTART_POS_Y+85, 120, 46, 0, (void *)PNG_STANDBY_OK_1_PATH, 0};
static WidgetSource RestartCancelSource = {StandardScreen::S576, RESTART_POS_X+205, RESTART_POS_Y+85, 120, 46, 0, (void *)PNG_STANDBY_CANCEL_PATH, 0};
static WidgetSource RestartCancek1Source = {StandardScreen::S576, RESTART_POS_X+205, RESTART_POS_Y+85, 120, 46, 0, (void *)PNG_STANDBY_CANCEL_1_PATH, 0};
static WidgetSource RestartBackgroundSource = {StandardScreen::S576, RESTART_POS_X, RESTART_POS_Y, 400, 150, 0, (void *)PNG_TR069_UPGRADE_RESTARTBK_PATH, 0};

RestartDialog::RestartDialog()
	: m_dialogFocus(false)
	, RestartOK(&RestartOkSource)
	, RestartOK1(&RestartOk1Source)
	, RestartCancel(&RestartCancelSource)
	, RestartCancel1(&RestartCancek1Source)
	, RestartBackground(&RestartBackgroundSource)
{
    LayerMixerDevice::Layer* layer = Hippo::systemManager().mixer().topLayer();

    layer->attachChildToFront(&RestartBackground);
    layer->attachChildToFront(&RestartOK);
    layer->attachChildToFront(&RestartOK1);
    layer->attachChildToFront(&RestartCancel);
    layer->attachChildToFront(&RestartCancel1);
}

RestartDialog::~RestartDialog()
{
    RestartBackground.detachFromParent();
    RestartOK.detachFromParent();
    RestartOK1.detachFromParent();
    RestartCancel.detachFromParent();
    RestartCancel1.detachFromParent();
}

bool
RestartDialog::handleMessage(Message *msg)
{
    if(MessageType_Repaint != msg->what)
        NATIVEHANDLER_LOG("what(0x%x),message(0x%x/0x%x)\n", msg->what, msg->arg1, msg->arg2);

    if(MessageType_Network == msg->what) {
        switch (msg->arg1) {
        case MV_Network_PhysicalUp:
        case MV_Network_PhysicalDown:
            return false;
        }
    }
    switch (msg->arg1) {
        case EIS_IRKEY_SELECT:
            if (true == m_dialogFocus) {
                RestartOK1.setVisibleP(false);
                RestartCancel.setVisibleP(false);
            } else {
                RestartOK.setVisibleP(false);
                RestartCancel1.setVisibleP(false);
            }
            RestartBackground.setVisibleP(false);

            if (true == m_dialogFocus)
                mid_fpanel_reboot();
            /* 恢复键值表功能 */
            keyDispatcher().setEnabled(true);
            return false;

        case EIS_IRKEY_LEFT:
            m_dialogFocus = true;
            RestartCancel1.setVisibleP(false);
            RestartOK.setVisibleP(false);
            RestartCancel.setVisibleP(true);
            RestartOK1.setVisibleP(true);
            break;

        case EIS_IRKEY_RIGHT:
            m_dialogFocus = false;
            RestartCancel.setVisibleP(false);
            RestartOK1.setVisibleP(false);
            RestartCancel1.setVisibleP(true);
            RestartOK.setVisibleP(true);
            break;
        default:
            break;
    }
    return true;
}

void
RestartDialog::draw()
{
    m_dialogFocus = true;
    RestartBackground.setVisibleP(true);
    RestartOK1.setVisibleP(true);
    RestartCancel.setVisibleP(true);
}

} // namespace Hippo

