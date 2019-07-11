
#include "PowerOffDialog.h"

#include "Message.h"
#include "MessageTypes.h"

#include "KeyDispatcher.h"
#include "NativeHandlerPublic.h"
#include "NativeHandlerAssertions.h"

#include "SystemManager.h"
#include "LayerMixerDevice.h"
#include "browser_event.h"
#include "config.h"

#define SANDBY_POS_X (((720-400)/2)&0xfffe)
#define SANDBY_POS_Y ((((576-80)/2)&0xfffe)-80)

extern "C" {

int monitor_close();
}

#define PNG_STANDBY_BK_PATH          SYS_IMG_PATH_ROOT"/standby/standby_bk.png"
#define PNG_STANDBY_OK_PATH          SYS_IMG_PATH_ROOT"/standby/standby_ok.png"
#define PNG_STANDBY_OK_1_PATH        SYS_IMG_PATH_ROOT"/standby/standby_ok1.png"
#define PNG_STANDBY_CANCEL_PATH      SYS_IMG_PATH_ROOT"/standby/standby_cancel.png"
#define PNG_STANDBY_CANCEL_1_PATH    SYS_IMG_PATH_ROOT"/standby/standby_cancel1.png"

namespace Hippo {
static WidgetSource StandbyOkSource = {StandardScreen::S576, SANDBY_POS_X+75, SANDBY_POS_Y+85, 120, 46, 0, (void *)PNG_STANDBY_OK_PATH, 0};
static WidgetSource StandbyOk1Source = {StandardScreen::S576, SANDBY_POS_X+75, SANDBY_POS_Y+85, 120, 46, 0, (void *)PNG_STANDBY_OK_1_PATH, 0};
static WidgetSource StandbyCancelSource = {StandardScreen::S576, SANDBY_POS_X+205, SANDBY_POS_Y+85, 120, 46, 0, (void *)PNG_STANDBY_CANCEL_PATH, 0};
static WidgetSource StandbyCancek1Source = {StandardScreen::S576, SANDBY_POS_X+205, SANDBY_POS_Y+85, 120, 46, 0, (void *)PNG_STANDBY_CANCEL_1_PATH, 0};
static WidgetSource StandbyBackgroundSource = {StandardScreen::S576, SANDBY_POS_X, SANDBY_POS_Y, 400, 150, 0, (void *)PNG_STANDBY_BK_PATH, 0};

PowerOffDialog::PowerOffDialog()
	: m_dialogFocus(false)
	, m_handle(NULL)
	, StandbyOk(&StandbyOkSource)
	, StandbyOk1(&StandbyOk1Source)
	, StandbyCancel(&StandbyCancelSource)
	, StandbyCancel1(&StandbyCancek1Source)
	, StandbyBackground(&StandbyBackgroundSource)
{
    LayerMixerDevice::Layer* layer = Hippo::systemManager().mixer().topLayer();
    layer->attachChildToFront(&StandbyBackground);
    layer->attachChildToFront(&StandbyOk);
    layer->attachChildToFront(&StandbyOk1);
    layer->attachChildToFront(&StandbyCancel);
    layer->attachChildToFront(&StandbyCancel1);
}

PowerOffDialog::~PowerOffDialog()
{
    StandbyBackground.detachFromParent();
    StandbyOk.detachFromParent();
    StandbyOk1.detachFromParent();
    StandbyCancel.detachFromParent();
    StandbyCancel1.detachFromParent();
}

void
PowerOffDialog::setHandler(NativeHandlerPublic *handler)
{
    m_handle = handler;
}

bool
PowerOffDialog::handleMessage(Message *msg)
{
    if(MessageType_Repaint != msg->what)
        NATIVEHANDLER_LOG("what(0x%x),message(0x%x/0x%x)\n", msg->what, msg->arg1, msg->arg2);

    switch (msg->arg1) {
        case EIS_IRKEY_SELECT:
            if (true == m_dialogFocus) {
                StandbyOk1.setVisibleP(false);
                StandbyCancel.setVisibleP(false);
            } else {
                StandbyOk.setVisibleP(false);
                StandbyCancel1.setVisibleP(false);
            }
            StandbyBackground.setVisibleP(false);

            if (true == m_dialogFocus && m_handle)
                m_handle->doPowerOff();

            /* 恢复键值表功能 */
            if (!NativeHandlerPublic::mProgressBarDialog)
               keyDispatcher().setEnabled(true);
            return false;

        case EIS_IRKEY_LEFT:
            m_dialogFocus = true;
            StandbyCancel1.setVisibleP(false);
            StandbyOk.setVisibleP(false);
            StandbyCancel.setVisibleP(true);
            StandbyOk1.setVisibleP(true);
            break;

        case EIS_IRKEY_RIGHT:
            m_dialogFocus = false;
            StandbyCancel.setVisibleP(false);
            StandbyOk1.setVisibleP(false);
            StandbyCancel1.setVisibleP(true);
            StandbyOk.setVisibleP(true);
            break;

        default:
            break;
    }
    return true;
}

void
PowerOffDialog::draw()
{
    m_dialogFocus = true;
    StandbyBackground.setVisibleP(true);
    StandbyOk1.setVisibleP(true);
    StandbyCancel.setVisibleP(true);
}

} // namespace Hippo
