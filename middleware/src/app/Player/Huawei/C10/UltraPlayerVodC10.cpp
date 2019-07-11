
#include "UltraPlayerVodC10.h"
#include "UltraPlayerAssertions.h"
#include "ProgramVOD.h"

#include "mid/mid_tools.h"
#include "stream_port.h"
#include "mid/mid_time.h"
#if defined(TR069_MONITOR) && defined(INCLUDE_TR069)
#include "Tr069X_CTC_IPTV_Monitor.h"
#endif
#include "Dialog.h"
#include "NativeHandlerPublic.h"

namespace Hippo {

UltraPlayerVodC10::UltraPlayerVodC10(UltraPlayerClient *client, BrowserPlayerReporter *pReporter, ProgramVOD *pProgram)
	: UltraPlayerVodHuawei(client, pReporter, pProgram)
	, m_isTr069Post(true)
{
}

UltraPlayerVodC10::~UltraPlayerVodC10()
{
    m_isTr069Post = false;
}


int
UltraPlayerVodC10::close(int mode)
{

    UltraPlayerVodHuawei::close(mode);
#if defined(TR069_MONITOR) && defined(INCLUDE_TR069)
    if (m_isTr069Post) {
        monitor_post_play(TYPE_VOD, 0,  NULL, mProgram->getVodSource(0)->GetUrl().c_str());
        PLAYER_LOG("url : %s\n", mProgram->getVodSource(0)->GetUrl().c_str());
    }
#endif
    return 0;
}

bool
UltraPlayerVodC10::onPause()
{
    if( mProgressBar
        && UIFlagIsEnabled(ProgressBar_Mask)
        && (mMediaType == APP_TYPE_IPTV || mMediaType == APP_TYPE_IPTV2 || mMediaType == APP_TYPE_VOD)) {
        unsigned int total, current, now;

        total = getTotalTime();
        current = getCurrentTime();
        now     = mid_time();
        if(total > 0) {
            if(mMediaType == APP_TYPE_IPTV){
                mProgressBar->setStartTime(now - total);
                mProgressBar->setEndTime(now);
            }
            else if(mMediaType ==  APP_TYPE_VOD){
                mProgressBar->setStartTime(0);
                mProgressBar->setEndTime(total);
            }
            mProgressBar->setCurrentTime(current);
            mProgressBar->setState(ProgressBarWidget::StateSeek);
            mProgressBar->setVisibleP(true);
        }
        else {
            PLAYER_LOG_ERROR("Error! total time is zero.\n");
        }
    }
    return UltraPlayer::onPause();
}

bool
UltraPlayerVodC10::onPlay()
{
    if(mProgressBar) {
        mProgressBar->setVisibleP(false);
    }
    if (NativeHandlerPublic::mProgressBarDialog)
        NativeHandlerPublic::mProgressBarDialog->Close();

    return UltraPlayer::onPlay();
}

bool
UltraPlayerVodC10::onStop()
{
    if(mProgressBar) {
        mProgressBar->setVisibleP(false);
    }
    if (NativeHandlerPublic::mProgressBarDialog)
        NativeHandlerPublic::mProgressBarDialog->Close();

    return UltraPlayer::onStop();
}

bool
UltraPlayerVodC10::onTrick(int rate)
{
    if( mProgressBar
        && UIFlagIsEnabled(ProgressBar_Mask)
        && (mMediaType == APP_TYPE_IPTV || mMediaType == APP_TYPE_IPTV2 || mMediaType == APP_TYPE_VOD)) {
        unsigned int total, current, now;

        total = getTotalTime();
        current = getCurrentTime();
        now = mid_time();
        if(total > 0) {
            if(mMediaType == APP_TYPE_IPTV){
                mProgressBar->setStartTime(now - total);
                mProgressBar->setEndTime(now);
            }
            else if(mMediaType == APP_TYPE_VOD){
                mProgressBar->setStartTime(0);
                mProgressBar->setEndTime(total);
            }
            mProgressBar->setCurrentTime(current);
            mProgressBar->setState(ProgressBarWidget::StateProgress);
            mProgressBar->setVisibleP(true);
            onProgress();
        }
        else {
            PLAYER_LOG_ERROR("Error! total time is zero.\n");
        }
    }

    return UltraPlayer::onTrick(rate);
}

void
UltraPlayerVodC10::onProgress(void)
{
    int ret = stream_port_get_state();

    if(ret == STRM_STATE_FAST) {
        if( mProgressBar
            && UIFlagIsEnabled(ProgressBar_Mask)
            && (mMediaType == APP_TYPE_IPTV || mMediaType == APP_TYPE_IPTV2 || mMediaType == APP_TYPE_VOD)) {
            unsigned int current = getCurrentTime();

            mProgressBar->setCurrentTime(current);
            mProgressBar->inval(NULL);
            this->sendEmptyMessageDelayed(MessageType_Timer, 1000);
        }
    }
}

void
UltraPlayerVodC10::handleMessage(Message * msg)
{
    if(msg->what == MessageType_Timer && msg->arg1 != 0x100) {
        onProgress();
        return;
    }
#if defined(TR069_MONITOR) && defined(INCLUDE_TR069)
    if (msg->what == MessageType_Play && (msg->arg1 == STRM_MSG_STREAM_VIEW || msg->arg1 == STRM_MSG_STREAM_END)) {
        PLAYER_LOG("vod url: %s\n", mProgram->getVodSource(0)->GetUrl().c_str());
        monitor_post_play(TYPE_VOD, (mid_10ms() - mPlayStartTime) * 10,  NULL, mProgram->getVodSource(0)->GetUrl().c_str());
    }
#endif
    UltraPlayerVodHuawei::handleMessage(msg);
}

void
UltraPlayerVodC10::SeekToStart(void)
{
    seekTo(0);
}

} // namespace Hippo

