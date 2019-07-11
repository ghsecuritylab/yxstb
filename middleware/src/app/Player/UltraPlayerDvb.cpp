
#include "UltraPlayerDvb.h"

#include "Message.h"
#include "MessageTypes.h"
#include "MessageHandler.h"
#include "ProgramChannel.h"
#include "BrowserPlayerReporterHuawei.h"
#include "ResourceManager.h"

#include "AppSetting.h"
#include "SysSetting.h"

#include "mid/mid_time.h"
#include "mid/mid_tools.h"
#include "mid_stream.h"
#include "app_sys.h"

namespace Hippo {


static void stateCall(int pIndex, STRM_STATE state, int rate, unsigned int magic, int callarg)
{
    UltraPlayer *p = UltraPlayer::lockPlayer(pIndex);
    if (p) {
        UltraPlayerDvb *player = (UltraPlayerDvb *)p;
        Message *message = player->obtainMessage(MessageType_Play, state+0x1000, rate);
        player->sendMessage(message);
        UltraPlayer::unlockPlayer(pIndex, p);
    }
}

static void msgCall(int pIndex, STRM_MSG msg, int arg, unsigned int magic, int callarg)
{
    UltraPlayer *p = UltraPlayer::lockPlayer(pIndex);
    if (p) {
        UltraPlayer *player = (UltraPlayer *)p;
        Message *message = player->obtainMessage(MessageType_Play, msg, arg);
        player->sendMessage(message);
        UltraPlayer::unlockPlayer(pIndex, p);
    }
}

UltraPlayerDvb::UltraPlayerDvb(UltraPlayerClient *client, BrowserPlayerReporter *reporter, ProgramChannel *program)
	: UltraPlayer(client, reporter, program)
	, mDvbProgram(program)
	, m_tunerIndex(1)
{
	mDvbProgram->ref();
}

UltraPlayerDvb::~UltraPlayerDvb()
{
	int changeVideoMode = 0;
    sysSettingGetInt("changevideomode", &changeVideoMode, 0);

	if(!changeVideoMode){
        close(UltraPlayer::BlackScreenMode);
    }else{
        close(UltraPlayer::LastFrameMode);
    }
	mDvbProgram->unref();
}

int
UltraPlayerDvb::play(unsigned int startTime)
{
    mResourceUserType = SimplePlay;
    mResourceRequirement = Resource::RTM_Tuner;

    int tunerCount = resourceManager().tunerResourceCount();
    if (tunerCount <= 0)
    	return -1;
#ifdef RESOURCE_MANAGER
    for (int i = 0; i < tunerCount; i++) {
    	m_tunerIndex = i + 1;
        if (!resourceManager().requestResource(this)) {
        	m_tunerIndex = -1;
        	continue;
        } else
            break;
    }
    if (m_tunerIndex == -1)
    	return -1;
#endif
	UltraPlayer::registerPlayer(mIndex, this);
    mid_stream_set_call(mIndex, stateCall, msgCall, (int)this);

    if(mIndex == 0){
    	if(mDvbProgram->GetChanType() == 1){
            sysSettingSetInt("LastVideoNumber", mDvbProgram->GetChanKey());
    	}else if(mDvbProgram->GetChanType() == 2){
            sysSettingSetInt("LastAudioNumber", mDvbProgram->GetChanKey());
    	}
        sysSettingSetInt("LastChannelType", mDvbProgram->GetChanType());
    	removeMessages(MessageType_ConfigSave);
		Message *message = this->obtainMessage(MessageType_ConfigSave, 0, 0);
		this->sendMessageDelayed(message, 3000);
    }

    char buf[2048] = {0};
	std::string json_bgn = "{\"ChannelKey\":";

    snprintf(buf, sizeof(buf), "%s%d,\"ProgramNum\":%d	\
   				,\"ServiceID\":%d,\"PMTPID\":%d	\
   				,\"Freq10KHZ\":%d}", json_bgn.c_str(), mDvbProgram->GetChanKey(), mDvbProgram->GetDVB_ProgNum(), \
   				mDvbProgram->GetDVB_ProgNum(), mDvbProgram->GetDVB_PMT_PID(), mDvbProgram->GetDVB_TpFreq()/10);
   	mid_stream_set_tuner(mIndex, m_tunerIndex);
   	setStreamVideoLocation(mIndex, m_VideoDisplaytop,m_VideoDisplayleft,m_VideoDisplaywidth,m_VideoDisplayheight, mDisplayMode);

    mMediaType = APP_TYPE_DVBS;
	mid_stream_open(mIndex, buf , mMediaType, 600);

    return 0;
}

int
UltraPlayerDvb::getRequiredProgNumber()
{
    return mDvbProgram->GetChanKey();
}

int
UltraPlayerDvb::getRequiredFrequency()
{
    return 0.0;
}

int
UltraPlayerDvb::getSpecialDevice()
{
   return m_tunerIndex;
}

int
UltraPlayerDvb::onAcquireResource()
{
    return 0;
}

int
UltraPlayerDvb::onLoseResource()
{
    return 0;
}

int
UltraPlayerDvb::closeForResource()
{
    return 0;
}

int
UltraPlayerDvb::stop()
{
    mid_stream_stop(mIndex);
    return 0;
}

int
UltraPlayerDvb::close(int mode)
{
    mid_stream_close(mIndex, mode);
    UltraPlayer::unregisterPlayer(mIndex, this);
    removeMessages(MessageType_Play);
#ifdef RESOURCE_MANAGER
    resourceManager().releaseResource(this);
#endif
    return 0;
}

int
UltraPlayerDvb::seekTo(unsigned int playTime)
{
    if (playTime == 0)
        playTime = mid_time() - getTotalTime();
    mid_stream_seek(mIndex, playTime);

    return 0;
}


int
UltraPlayerDvb::fastForward(int speed)
{
    mid_stream_fast(mIndex, speed);

    return 0;
}

int
UltraPlayerDvb::fastRewind(int speed)
{
    mid_stream_fast(mIndex, speed);

    return 0;
}

int
UltraPlayerDvb::pause()
{

    mid_stream_pause(mIndex);

    return 0;
}

int
UltraPlayerDvb::resume()
{
    mid_stream_resume(mIndex);

    return 0;
}

int
UltraPlayerDvb::seekEnd()
{
    mid_stream_stop(mIndex);

    return 0;
}

std::string&
UltraPlayerDvb::getMediaCode()
{
    if(mDvbProgram)
        return mDvbProgram->GetMediaCode();
    return m_mediaCode;
}

unsigned int
UltraPlayerDvb::getCurrentTimeString(char *TimeString)
{
    unsigned int CurPlayTime = 0;

    CurPlayTime = mid_stream_get_currenttime(mIndex);
    mid_tool_time2string(CurPlayTime, TimeString, 'T');

    return 0;
}

unsigned int
UltraPlayerDvb::getTotalTime()
{
	unsigned int totalTime = 0;

	int localTimeShift = 0;
	appSettingGetInt("localTimeShift_maxDuration", &localTimeShift, 0);
	localTimeShift = localTimeShift * 60;
	totalTime = mid_time() - mid_stream_get_totaltime(mIndex);
	if (totalTime > localTimeShift)
		totalTime = localTimeShift;

	return totalTime;
}

void
UltraPlayerDvb::handleMessage(Message *msg)
{
	if(msg->what == MessageType_ConfigSave){
        //TODO
        return;
    }
    if(msg->arg1 >= 0x1000){
        mCurrentStatus = msg->arg1 - 0x1000;
        mCurrentSpeed = msg->arg2;
        reporter()->reportState((STRM_STATE)mCurrentStatus, mCurrentSpeed);
    } else {
    	reporter()->reportMessage((STRM_MSG)msg->arg1, msg->arg2);
    }
}

} // namespace Hippo
