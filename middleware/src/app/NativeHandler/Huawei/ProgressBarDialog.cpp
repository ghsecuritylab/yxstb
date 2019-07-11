

#include "ProgressBarDialog.h"
#include "NativeHandlerAssertions.h"

#include "Message.h"
#include "KeyDispatcher.h"
#include "NativeHandlerPublic.h"
#include "SystemManager.h"
#include "StandardScreen.h"
#include "UltraPlayer.h"
#include "ProgressBarWidget.h"

#include "mid/mid_time.h"
#include "SysSetting.h"
#include "config.h"
#include "browser_event.h"


namespace Hippo {

// static WidgetSource ProgressBgSource = {StandardScreen::S576, 40, 360, 640, 79, 0, app_gif_progress_bg, 4000};

ProgressBarDialog::ProgressBarDialog()
    : m_pos(0)
    , m_handle(NULL)
{
    NATIVEHANDLER_LOG("ProgressBarDialog Constructor.\n");
}

ProgressBarDialog::~ProgressBarDialog()
{
}

void
ProgressBarDialog::setHandler(NativeHandlerPublic *handler)
{
    m_handle = handler;
}

bool
ProgressBarDialog::handleMessage(Message *msg)
{
    NATIVEHANDLER_LOG("Msg->arg1 = %d/%x.\n", msg->arg1, msg->arg1);
    switch (msg->arg1) {
    case EIS_IRKEY_UP:
    case EIS_IRKEY_DOWN:
    case EIS_IRKEY_LEFT:
    case EIS_IRKEY_RIGHT:
    case EIS_IRKEY_NUM0:
    case EIS_IRKEY_NUM1:
    case EIS_IRKEY_NUM2:
    case EIS_IRKEY_NUM3:
    case EIS_IRKEY_NUM4:
    case EIS_IRKEY_NUM5:
    case EIS_IRKEY_NUM6:
    case EIS_IRKEY_NUM7:
    case EIS_IRKEY_NUM8:
    case EIS_IRKEY_NUM9: {
        bool ret = false;
        UltraPlayer * player = systemManager().obtainMainPlayer();

        if(player) {
            ProgressBarWidget * ProgressBar = UltraPlayer::mProgressBar;
            if(ProgressBar && ProgressBar->isVisible()) {
                ret = ProgressBar->InputKey(msg->arg1);
            }
            systemManager().releaseMainPlayer(player);
        }
        if(!ret)
            Close();
        return ret;
    }
    case EIS_IRKEY_SELECT: {
        bool ret = false;
        UltraPlayer * player = systemManager().obtainMainPlayer();

        if(player) {
            ProgressBarWidget * ProgressBar = UltraPlayer::mProgressBar;
            if(ProgressBar && ProgressBar->isVisible()) {
                ret = ProgressBar->InputKey(msg->arg1);
                if(ret) {
                    unsigned int cur = ProgressBar->getCurrentTime();

                    if(MID_UTC_SUPPORT) {
                        if(ProgressBar->getEndTime() != time(NULL) && ProgressBar->getStartTime() != 0) {
                            int time_zone = 0;

                            sysSettingGetInt("timezone", &time_zone, 0);;
                            cur -= time_zone * 60 * 60;
                        }
                    }
                    player->seekTo(cur);
                }
                else {
                    return true;
                }
            }
        }
        Close();
        return true;
    }
    case EIS_IRKEY_MENU: {
        this->Close();
        break;
    }
    case EIS_IRKEY_BACK: {
        if(UltraPlayer::mProgressBar && UltraPlayer::mProgressBar->isVisible())
            return true;
        break;
    }
    case EIS_IRKEY_PAGE_UP: {
        return true;
    }
    case EIS_IRKEY_PAGE_DOWN: {
        return true;
    }
    default:
        break;
    }
    if(!UltraPlayer::mProgressBar || !UltraPlayer::mProgressBar->isVisible()) {
        NATIVEHANDLER_LOG("Close ProgressBar Dialog.\n");
        Close();
    }
    return false;
}

void
ProgressBarDialog::draw()
{
}

bool
ProgressBarDialog::onClose()
{
    NATIVEHANDLER_LOG("\n");
    return Dialog::onClose();
}

};

