
#include "UltraPlayerMultipleC10.h"
#include "UltraPlayerAssertions.h"
#include "ProgramChannelC10.h"

#include "SystemManager.h"
#include "LayerMixerDevice.h"

#include "mid/mid_time.h"
#include "mid/mid_tools.h"
#include "stream_port.h"
#if defined(TR069_MONITOR) && defined(INCLUDE_TR069)
#include "Tr069X_CTC_IPTV_Monitor.h"
#endif

#include "SysSetting.h"
#include "app_sys.h"
#include "Dialog.h"
#include "NativeHandlerPublic.h"


extern "C" char* ChanLogo_http_get(char const* url, int* length);

namespace Hippo {

UltraPlayerMultipleC10::UltraPlayerMultipleC10(UltraPlayerClient *client, BrowserPlayerReporter *pReporter, ProgramChannel *pProgram)
    : UltraPlayerMultipleHuawei(client, pReporter, pProgram)
    , mChannelLogo(NULL)
    , m_isTr069Post(true)
{
}

UltraPlayerMultipleC10::~UltraPlayerMultipleC10()
{
	int changeVideoMode = 0;

    m_isTr069Post = false;
	sysSettingGetInt("changevideomode", &changeVideoMode, 0);
	if(!changeVideoMode){ // 0 is black screen 1 is last frame 2 is smooth change
    	close(UltraPlayer::BlackScreenMode);
    } else {
    	close(UltraPlayer::LastFrameMode);
    }
}

int
UltraPlayerMultipleC10::showChanLogo()
{
    ProgramChannelC10 *tProgram = (ProgramChannelC10 *)mProgram;

    if(mChannelLogo) {
        delete(mChannelLogo);
        mChannelLogo = NULL;
    }
    if(tProgram->GetLogoURL().empty() == true) {
        PLAYER_LOG_ERROR("Channel logo url is empty\n");
        return -1;
    }
    if(tProgram->GetLasting() < 0) {
        PLAYER_LOG_ERROR("Lasting time(%d) is error\n", tProgram->GetLasting());
        return -1;
    }
    PLAYER_LOG("Channel logo INFO:\n");
    PLAYER_LOG("Channel logo url %s \n", tProgram->GetLogoURL().c_str());
    PLAYER_LOG("Channel logo PosX:%d, PosY:%d, Begin:%ds, Last:%ds, Interval:%dS\n", tProgram->GetLogoXPos(), tProgram->GetLogoYPos(), tProgram->GetBeginTime(), tProgram->GetLasting(), tProgram->GetInterval());
    if(!mChannelLogo) {
        static WidgetSource source;

        source.standard = StandardScreen::S576;
        source.x = tProgram->GetLogoXPos();
        source.y = tProgram->GetLogoYPos();
        source.w = -1;
        source.h = 0;
        this->removeMessages(MessageType_ShowChanLogoIcon);
        this->removeMessages(MessageType_ClearChanLogoIcon);
        source.image = ChanLogo_http_get(tProgram->GetLogoURL().c_str(), &(source.imageLength));
        if(source.image == NULL || source.imageLength <= 0) {
            PLAYER_LOG_ERROR("URL is error!!!!!\n");
            return -1;
        }
        mChannelLogo = new ChannelLogoWidget(&source);
        if(!mChannelLogo) {
            PLAYER_LOG_ERROR("mChannelLogo is NULL\n");
            return -1;
        }
        SystemManager &sysManager = systemManager();
        LayerMixerDevice::Layer* layer = sysManager.mixer().topLayer();
        layer->attachChildToFront(mChannelLogo);
        Message *message = this->obtainMessage(MessageType_ShowChanLogoIcon, tProgram->GetLasting(), tProgram->GetInterval());

        if(tProgram->GetBeginTime() <= 0)
            this->sendMessage(message);
        else
            this->sendMessageDelayed(message,  tProgram->GetBeginTime() * 1000);
    } else {
        PLAYER_LOG_ERROR("mChannelLogo NOT NULL!\n");
        return -1;
    }
    return 0;
}

int
UltraPlayerMultipleC10::hideChanLogo()
{
    if(mChannelLogo) {
        this->removeMessages(MessageType_ShowChanLogoIcon);
        this->removeMessages(MessageType_ClearChanLogoIcon);
        mChannelLogo->detachFromParent();
        Message *message = this->obtainMessage(MessageType_ClearChanLogoIcon, 0, 0);
        this->sendMessage(message);
    }
    return 0;
}

int
UltraPlayerMultipleC10::play(unsigned int startTime)
{
    int ret = 0;

    ret = UltraPlayerMultipleHuawei::play(startTime);
    showChanLogo();
    return ret;
}

int
UltraPlayerMultipleC10::close(int mode)
{
    int ret = 0;

    ret = UltraPlayerMultipleHuawei::close(mode);
#if defined(TR069_MONITOR) && defined(INCLUDE_TR069)
    if (m_isTr069Post) {
        monitor_post_play(TYPE_CHANNEL, 0,  mProgram->GetChanName().c_str(), mProgram->GetChanURL().c_str());
    }
#endif
    hideChanLogo();
    return ret;
}

unsigned int
UltraPlayerMultipleC10::getCurrentTimeString(char *TimeString)
{
    unsigned int CurPlayTime = 0;

#if defined(HUBEI_HD) || defined(GUANGDONG)
    if (STRM_STATE_IPTV == mCurrentStatus) {
        CurPlayTime = mid_time();
    } else
#endif
    {
        CurPlayTime = mid_stream_get_currenttime(mIndex);
    }
    mid_tool_time2string(CurPlayTime, TimeString, 'T');
    PLAYER_LOG("Current time string(%s)\n", TimeString);
    return 0;
}

void
UltraPlayerMultipleC10::handleMessage(Message *msg)
{
    if(msg->what == MessageType_Timer) {
        onProgress();
        return;
    }
#if defined(TR069_MONITOR) && defined(INCLUDE_TR069)
    if (msg->what == MessageType_Play && msg->arg1 == STRM_MSG_STREAM_VIEW) {
        monitor_post_play(TYPE_CHANNEL, (mid_10ms() - mPlayStartTime) * 10,  mProgram->GetChanName().c_str(), mProgram->GetChanURL().c_str());
    }
#endif
    if(msg) {
        PLAYER_LOG("Message what:0x%x,arg1:0x%x,arg2:0x%x\n", msg->what, msg->arg1, msg->arg2);
        switch(msg->what) {
        case MessageType_ShowChanLogoIcon: {
            if(mChannelLogo) {
                PLAYER_LOG("MessageType_ShowChanLogoIcon\n");
                mChannelLogo->setVisibleP(true);
                if((msg->arg1 > 0) && (msg->arg2 >= 0)) {
                    this->removeMessages(MessageType_ShowChanLogoIcon);
                    this->removeMessages(MessageType_ClearChanLogoIcon);
                    Message *message = this->obtainMessage(MessageType_ClearChanLogoIcon, msg->arg1, msg->arg2);
                    this->sendMessageDelayed(message, msg->arg1 * 1000);
                }
            }
            break;
        }
        case MessageType_ClearChanLogoIcon: {
            if(mChannelLogo) {
                mChannelLogo->setVisibleP(false);
                if(msg->arg2 > 0) {
                    this->removeMessages(MessageType_ShowChanLogoIcon);
                    this->removeMessages(MessageType_ClearChanLogoIcon);
                    Message *message = this->obtainMessage(MessageType_ShowChanLogoIcon, msg->arg1, msg->arg2);
                    this->sendMessageDelayed(message, msg->arg2 * 1000);
                }
            }
            break;
        }
        default: {
            UltraPlayerMultipleHuawei::handleMessage(msg);
            break;
        }
        }
    }
    return ;
}

bool
UltraPlayerMultipleC10::onPause()
{
    if(mProgressBar && UIFlagIsEnabled(ProgressBar_Mask)) {
        unsigned int total, current, now;

        total   = getTotalTime();
        current = getCurrentTime();
        now     = mid_time();

        if(total > 0) {
            mProgressBar->setStartTime(now - total);
            mProgressBar->setEndTime(now);
            mProgressBar->setCurrentTime(current);
            mProgressBar->setState(ProgressBarWidget::StateSeek);
            mProgressBar->setVisibleP(true);
        }
        else {
            PLAYER_LOG_ERROR("Error! total time is zero.\n");
        }
    }
    return UltraPlayerMultipleHuawei::onPause();
}

void
UltraPlayerMultipleC10::onProgress(void)
{
    int ret = stream_port_get_state();

    if(ret == STRM_STATE_FAST) {
        if( mProgressBar
            && UIFlagIsEnabled(ProgressBar_Mask)
            && (mMediaType == APP_TYPE_IPTV || mMediaType == APP_TYPE_IPTV2 || mMediaType == APP_TYPE_VOD)) {
            unsigned int c = getCurrentTime();

            mProgressBar->setCurrentTime(c);
            mProgressBar->inval(NULL);
            this->sendEmptyMessageDelayed(MessageType_Timer, 1000);
        }
    }
}

bool
UltraPlayerMultipleC10::onPlay()
{
    if(mProgressBar) {
        mProgressBar->setVisibleP(false);
    }
    if (NativeHandlerPublic::mProgressBarDialog)
        NativeHandlerPublic::mProgressBarDialog->Close();

    return UltraPlayerMultipleHuawei::onPlay();
}

bool
UltraPlayerMultipleC10::onStop()
{
    if(mProgressBar) {
        mProgressBar->setVisibleP(false);
    }
    if (NativeHandlerPublic::mProgressBarDialog)
        NativeHandlerPublic::mProgressBarDialog->Close();

    return UltraPlayerMultipleHuawei::onStop();
}

bool
UltraPlayerMultipleC10::onTrick(int rate)
{
    if(mProgressBar
       && UIFlagIsEnabled(ProgressBar_Mask)
       && (mMediaType == APP_TYPE_IPTV || mMediaType == APP_TYPE_IPTV2 || mMediaType == APP_TYPE_VOD)) {
        unsigned int total, current, now;

        total   = getTotalTime();
        current = getCurrentTime();
        now     = mid_time();

        if(total > 0) {
            mProgressBar->setStartTime(now - total);
            mProgressBar->setEndTime(now);
            mProgressBar->setCurrentTime(current);
            mProgressBar->setState(ProgressBarWidget::StateProgress);
            mProgressBar->setVisibleP(true);
            onProgress();
        }
        else {
            PLAYER_LOG_ERROR("Error! total time is zero.\n");
        }
    }
    return UltraPlayerMultipleHuawei::onTrick(rate);
}

void
UltraPlayerMultipleC10::SeekToStart(void)
{
    unsigned int total, current, now;

    total   = getTotalTime();
    current = getCurrentTime();
    now     = mid_time();
    seekTo(now - total);
}

} // namespace Hippo

