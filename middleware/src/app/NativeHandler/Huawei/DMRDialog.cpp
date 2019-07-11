
#include "DMRDialog.h"

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


#define PNG_DMR_BG_PATH          SYS_IMG_PATH_ROOT"/dmr/dmr_bg.png"
#define PNG_DMR_OK_PATH          SYS_IMG_PATH_ROOT"/dmr/dmr_ok.png"
#define PNG_DMR_OK_1_PATH        SYS_IMG_PATH_ROOT"/dmr/dmr_ok1.png"
#define PNG_DMR_CANCEL_PATH      SYS_IMG_PATH_ROOT"/dmr/dmr_cancel.png"
#define PNG_DMR_CANCEL_1_PATH    SYS_IMG_PATH_ROOT"/dmr/dmr_cancel1.png"

namespace Hippo {
extern void setIsplay(int temp);
extern void setIsplay_huawei(int temp);

static WidgetSource DlnaOkSource = {StandardScreen::S576, SANDBY_POS_X+75, SANDBY_POS_Y+85, 120, 46, 0, (void*)PNG_DMR_OK_PATH, 0};
static WidgetSource DlnaOk1Source = {StandardScreen::S576, SANDBY_POS_X+75, SANDBY_POS_Y+85, 120, 46, 0, (void*)PNG_DMR_OK_1_PATH, 0};
static WidgetSource DlnaCancelSource = {StandardScreen::S576, SANDBY_POS_X+205, SANDBY_POS_Y+85, 120, 46, 0, (void*)PNG_DMR_CANCEL_PATH, 0};
static WidgetSource DlnaCancek1Source = {StandardScreen::S576, SANDBY_POS_X+205, SANDBY_POS_Y+85, 120, 46, 0, (void*)PNG_DMR_CANCEL_1_PATH, 0};
static WidgetSource DlnaBackgroundSource = {StandardScreen::S576, SANDBY_POS_X, SANDBY_POS_Y, 400, 150, 0, (void*)PNG_DMR_BG_PATH, 0};

DlnaDialog::DlnaDialog()
	: m_dialogFocus(false)
	, m_handle(NULL)
	, DlnaOk(&DlnaOkSource)
	, DlnaOk1(&DlnaOk1Source)
	, DlnaCancel(&DlnaCancelSource)
	, DlnaCancel1(&DlnaCancek1Source)
	, DlnaBackground(&DlnaBackgroundSource)
{
    LayerMixerDevice::Layer* layer = Hippo::systemManager().mixer().topLayer();
    layer->attachChildToFront(&DlnaBackground);
    layer->attachChildToFront(&DlnaOk);
    layer->attachChildToFront(&DlnaOk1);
    layer->attachChildToFront(&DlnaCancel);
    layer->attachChildToFront(&DlnaCancel1);
}

DlnaDialog::~DlnaDialog()
{
    DlnaBackground.detachFromParent();
    DlnaOk.detachFromParent();
    DlnaOk1.detachFromParent();
    DlnaCancel.detachFromParent();
    DlnaCancel1.detachFromParent();
}

void
DlnaDialog::setHandler(NativeHandlerPublic *handler)
{
    m_handle = handler;
}

bool
DlnaDialog::handleMessage(Message *msg)
{
    if(MessageType_Repaint != msg->what)
        NATIVEHANDLER_LOG("what(0x%x),message(0x%x/0x%x)\n", msg->what, msg->arg1, msg->arg2);

    switch (msg->arg1) {
        case EIS_IRKEY_SELECT:
            if (true == m_dialogFocus) {
                DlnaOk1.setVisibleP(false);
                DlnaCancel.setVisibleP(false);
            } else {
                DlnaOk.setVisibleP(false);
                DlnaCancel1.setVisibleP(false);
            }
            DlnaBackground.setVisibleP(false);
#ifdef INCLUDE_DMR
            if (true == m_dialogFocus ) {
	            if (defNativeHandler().getState() == NativeHandler::Local) {
		            setIsplay_huawei(1);

	            }
	            else{
		            setIsplay(1);
	            }
            }
            else {
	            if (defNativeHandler().getState() == NativeHandler::Local) {
		            setIsplay_huawei(0);

	            }
	            else{
		            setIsplay(0);
	            }
            }
#endif
//setIsplay(int temp)

            /* 恢复键值表功能 */
            if (!NativeHandlerPublic::mProgressBarDialog)
               keyDispatcher().setEnabled(true);
            return false;

        case EIS_IRKEY_LEFT:
            m_dialogFocus = true;
            DlnaCancel1.setVisibleP(false);
            DlnaOk.setVisibleP(false);
            DlnaCancel.setVisibleP(true);
            DlnaOk1.setVisibleP(true);
            break;

        case EIS_IRKEY_RIGHT:
            m_dialogFocus = false;
            DlnaCancel.setVisibleP(false);
            DlnaOk1.setVisibleP(false);
            DlnaCancel1.setVisibleP(true);
            DlnaOk.setVisibleP(true);
            break;

        default:
            break;
    }
    return true;
}

void
DlnaDialog::draw()
{
    m_dialogFocus = true;
    DlnaBackground.setVisibleP(true);
    DlnaOk1.setVisibleP(true);
    DlnaCancel.setVisibleP(true);
}

} // namespace Hippo
