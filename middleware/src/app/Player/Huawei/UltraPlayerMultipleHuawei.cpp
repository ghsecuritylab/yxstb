
#include "UltraPlayerMultipleHuawei.h"
#include "UltraPlayerAssertions.h"
#include "UltraPlayerClient.h"

#include "BrowserPlayerReporterHuawei.h"

#include "Message.h"
#include "MessageTypes.h"
#include "MessageHandler.h"
#include "ProgramChannel.h"
#include "BrowserEventQueue.h"

#ifdef HUAWEI_C20
#include "ProgramChannelC20.h"
#endif
#include "ResourceManager.h"

#include "Session.h"
#include "AppSetting.h"

#include "mid/mid_time.h"
#include "mid/mid_tools.h"
#include "app_sys.h"
#include "stream_port.h"
#ifdef INCLUDE_cPVR
#include "Tstv.h"
#endif

namespace Hippo {

static void stateCall(int pIndex, STRM_STATE state, int rate, unsigned int magic, int callarg)
{
    UltraPlayer *p = UltraPlayer::lockPlayer(pIndex);
    if (p) {
        UltraPlayerMultipleHuawei *player = (UltraPlayerMultipleHuawei *)p;
        Message *message = player->obtainMessage(MessageType_Play, state+0x1000, rate);
        player->sendMessage(message);
        //player->reporter()->reportState(state, rate);
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
        //player->reporter()->reportMessage(msg, arg);
        UltraPlayer::unlockPlayer(pIndex, p);
    }
}

UltraPlayerMultipleHuawei::UltraPlayerMultipleHuawei(UltraPlayerClient *client, BrowserPlayerReporter *pReporter, ProgramChannel *pProgram)
	: UltraPlayerMultiple(client, pReporter, pProgram)
	, mProgram(pProgram)
	, mSighMagic(0)
{
    mProgram->ref();
}

UltraPlayerMultipleHuawei::~UltraPlayerMultipleHuawei()
{
#if 0 // move to C10
    if(sys_change_videomode_get() == 0){ // 0 is black screen 1 is last frame 2 is smooth change
        close(UltraPlayer::BlackScreenMode);
    }
    else{
        close(UltraPlayer::LastFrameMode);
    }
#endif
    mProgram->unref();

}

float
UltraPlayerMultipleHuawei::getRequiredBandwidth()
{
#ifdef HUAWEI_C20
    return ((ProgramChannelC20 *)mProgram)->GetChanBandwith();
#else
    return 0.0;
#endif
}

int
UltraPlayerMultipleHuawei::onAcquireResource()
{
    return 0;
}

int
UltraPlayerMultipleHuawei::onLoseResource()
{
    return 0;
}

int
UltraPlayerMultipleHuawei::closeForResource()
{
    return 0;
}

int
UltraPlayerMultipleHuawei::open()
{
    return 0;
}

int
UltraPlayerMultipleHuawei::play(unsigned int startTime)
{
#ifdef ENABLE_IPV6
    char host[STREAM_URL_SIZE] = {0};
    struct ind_sin_ex sin_ex;
#endif

#ifdef RESOURCE_MANAGER
    mResourceUserType = SimplePlay;
    mResourceRequirement = Resource::RTM_Bandwidth;
    if (!resourceManager().requestResource(this)) {
        return -1;
    }
#endif

    mPlayStartTime = mid_10ms();
    UltraPlayer::registerPlayer(mIndex, this);
    mid_stream_set_call(mIndex, stateCall, msgCall, (int)this);
#ifdef ENABLE_IPV6
    if(strncmp(mProgram->GetChanURL().c_str(),"igmp://[", 8) == 0) {
        memcpy(host, (char *)mProgram->GetChanURL().c_str() + 7, mProgram->GetChanURL().length() - 7);
        if(ind_net_pton(host, &(sin_ex.host)) > 0) {
            mid_stream_set_igmp(0, stream_port_mldv2_play, stream_port_mldv2_stop, &sin_ex, sizeof(struct ind_sin_ex));
        }
    }
#endif

#ifdef INCLUDE_SQA
    int fccSwitch = 0;
	appSettingGetInt("fcc_switch", &fccSwitch, 0);
    if(1 == fccSwitch ){
        mid_stream_set_fcc(0, mProgram->GetSqaCode());
    }
#endif

    char buf[2048] = {0};
    std::string json_bgn = "{\"type\":\"EVENT_GO_CHANNEL\",";

    snprintf(buf, sizeof(buf), "%s\"instance_id\":\"%d\",\"channel_code\":\"%d\",\"channel_num\":\"%d\"}", json_bgn.c_str(), \
        mClient->instanceId(), atoi(mProgram->GetChanID().c_str()), mProgram->GetUserChanID());
    browserEventSend(buf, NULL);

    mMediaType = APP_TYPE_IPTV;
    //OTT check
    if (strstr((char *)mProgram->GetChanURL().c_str(), ".m3u8") || strstr((char *)mProgram->GetChanURL().c_str(), "m3u8:")/*四川*/) {
        PLAYER_LOG("begin to play http live stream .......url(%s)\n", (char *)mProgram->GetChanURL().c_str());
        if (strstr((char *)mProgram->GetChanURL().c_str(), "&servicetype=1"))//Servicetype取值：0为Webtv点播、1为Webtv直播、2为Webtv时移、3为Webtv录播
            mMediaType = APP_TYPE_APPLE_IPTV;
        else
            mMediaType = APP_TYPE_APPLE_VOD;
    }
    if(mProgram->GetTimeShift() == 0) {
       mSighMagic = mid_stream_open(mIndex, (char *)mProgram->GetChanURL().c_str(), mMediaType, mProgram->GetTimeShiftLength());
    }
    else {
        struct IPTVUrl tPlayUrl = {0};

        if(mMediaType == APP_TYPE_IPTV)
            mMediaType = APP_TYPE_IPTV2;

        snprintf(tPlayUrl.channel_url, STREAM_URL_SIZE, "%s", (char *)mProgram->GetChanURL().c_str());
        if((session().getPlatform() == PLATFORM_ZTE) && (!(strcasestr((char *)mProgram->GetTimeShiftURL().c_str(),"rtsp://"))))
            memset(tPlayUrl.tmshift_url, 0, STREAM_URL_SIZE);
        else
            snprintf(tPlayUrl.tmshift_url, STREAM_URL_SIZE, "%s", (char *)mProgram->GetTimeShiftURL().c_str());

        mSighMagic = mid_stream_open(mIndex, (char *)&tPlayUrl, mMediaType, mProgram->GetTimeShiftLength());
    }
    return 0;
}

int
UltraPlayerMultipleHuawei::seekTo(unsigned int playTime)
{
    PLAYER_LOG("Timeshift length(%d) Seek to %d\n", mProgram->GetTimeShiftLength(), playTime);
    if(mProgram->GetTimeShift()
#ifdef INCLUDE_cPVR
        || TstvStartFlagGet()
#endif
        ){
        if(playTime == 0){ //goto start set playTime 0 in timeshift mode
            playTime = mid_time() - mid_stream_get_totaltime(mIndex);
        } if (playTime == mProgram->GetTimeShiftLength()) {  //goto start set playTime timeshift length in timeshift mode
            playTime = mid_time();
        }
        PLAYER_LOG("Seek to %d, current time %d\n", playTime, mid_time());
        mid_stream_seek(mIndex, playTime);
    }
    return -1;
}

int
UltraPlayerMultipleHuawei::fastForward(int speed)
{
    if(mProgram->GetTimeShift()
#ifdef INCLUDE_cPVR
        || TstvStartFlagGet()
#endif
        ){
        mid_stream_fast(mIndex, speed);
    }
    return 0;
}

int
UltraPlayerMultipleHuawei::fastRewind(int speed)
{
    if(mProgram->GetTimeShift()
#ifdef INCLUDE_cPVR
        || TstvStartFlagGet()
#endif
        ){
        mid_stream_fast(mIndex, speed);
    }
    return 0;
}

int
UltraPlayerMultipleHuawei::pause()
{
    if(mProgram->GetTimeShift()
#ifdef INCLUDE_cPVR
        || TstvStartFlagGet()
#endif
        ){
        mid_stream_pause(mIndex);
    }
    return 0;
}

int
UltraPlayerMultipleHuawei::resume()
{
    if(mProgram->GetTimeShift()
#ifdef INCLUDE_cPVR
        || TstvStartFlagGet()
#endif
        ){
        mid_stream_resume(mIndex);
    }
    return 0;
}

int
UltraPlayerMultipleHuawei::stop()
{
    mid_stream_stop(mIndex);
    return 0;
}

int
UltraPlayerMultipleHuawei::close(int mode)
{
    PLAYER_LOG("mid_stream_close\n");
    mid_stream_close(mIndex, mode);
    UltraPlayer::ClearAllIcon();
    UltraPlayer::unregisterPlayer(mIndex, this);
    removeMessages(MessageType_Play);
#ifdef RESOURCE_MANAGER
    resourceManager().releaseResource(this);
#endif
    return 0;
}

int
UltraPlayerMultipleHuawei::seekEnd()
{
    mid_stream_stop(mIndex);
    return 0;
}

unsigned int
UltraPlayerMultipleHuawei::getTotalTime()
{
    PLAYER_LOG("UltraPlayerMultipleHuawei::getTotalTime %d\n", mid_stream_get_totaltime(mIndex));
    return mid_stream_get_totaltime(mIndex);
}

unsigned int
UltraPlayerMultipleHuawei::getCurrentTime()
{
    PLAYER_LOG("UltraPlayerMultipleHuawei::getCurrentTime %d\n", mid_stream_get_currenttime(mIndex));
    return mid_stream_get_currenttime(mIndex);
}

unsigned int
UltraPlayerMultipleHuawei::getCurrentTimeString(char *TimeString)
{
    unsigned int CurPlayTime = 0;

    CurPlayTime = mid_stream_get_currenttime(mIndex);
    CurPlayTime -= mid_get_times_sec();
    mid_tool_time2string(CurPlayTime, TimeString, 'T');
    PLAYER_LOG("UltraPlayerMultipleHuawei::getCurrentTime %s\n", CurPlayTime);
    return 0;
}

std::string&
UltraPlayerMultipleHuawei::getMediaCode()
{
    if(mProgram)
        return mProgram->GetMediaCode();
    return m_mediaCode;
}

std::string&
UltraPlayerMultipleHuawei::getEntryID()
{
    if(mProgram)
        return mProgram->GetEntryID();
    return m_entryId;
}

void
UltraPlayerMultipleHuawei::handleMessage(Message *msg)
{
    if(msg->what >= MessageType_ConfigSave && msg->what <= MessageType_ClearAllIcon){
        return UltraPlayer::handleMessage(msg);
    }
    if(msg->arg1 >= 0x1000){
        switch(msg->arg1 - 0x1000){
            case STRM_STATE_PAUSE:{
                UltraPlayer::ShowPlayStateIcon(PlayStateWidget::StatePause);
                break;
            }
            case STRM_STATE_PLAY:{
                if(-1 != mCurrentStatus || 0 != mCurrentStatus){
                    UltraPlayer::ShowPlayStateIcon(PlayStateWidget::StateTimeShift);
                }
                break;
            }
            case STRM_STATE_FAST:{
                switch(msg->arg2){
                    case 2:{
                        UltraPlayer::ShowPlayStateIcon(PlayStateWidget::StateFW2);
                        break;
                    }
                    case 4:{
                        UltraPlayer::ShowPlayStateIcon(PlayStateWidget::StateFW4);
                        break;
                    }
                    case 8:{
                        UltraPlayer::ShowPlayStateIcon(PlayStateWidget::StateFW8);
                        break;
                    }
                    case 16:{
                        UltraPlayer::ShowPlayStateIcon(PlayStateWidget::StateFW16);
                        break;
                    }
                    case 32:{
                        UltraPlayer::ShowPlayStateIcon(PlayStateWidget::StateFW32);
                        break;
                    }
                    case -2:{
                        UltraPlayer::ShowPlayStateIcon(PlayStateWidget::StateRW2);
                        break;
                    }
                    case -4:{
                        UltraPlayer::ShowPlayStateIcon(PlayStateWidget::StateRW4);
                        break;
                    }
                    case -8:{
                        UltraPlayer::ShowPlayStateIcon(PlayStateWidget::StateRW8);
                        break;
                    }
                    case -16:{
                        UltraPlayer::ShowPlayStateIcon(PlayStateWidget::StateRW16);
                        break;
                    }
                    case -32:{
                        UltraPlayer::ShowPlayStateIcon(PlayStateWidget::StateRW32);
                        break;
                    }
                    default:{
                        break;
                    }
                }
                break;
            }
            case STRM_STATE_CLOSE:{
                if(mPlayState){
                    mPlayState->setVisibleP(false);
                }
                break;
            }
            default:{
                break;
            }
        }
        mCurrentStatus = msg->arg1 - 0x1000;
        mCurrentSpeed = msg->arg2;
        reporter()->reportState((STRM_STATE)mCurrentStatus, mCurrentSpeed);
    }
    else{
        switch(msg->arg1){
            case STRM_MSG_STREAM_END:{
                UltraPlayer::ShowPlayStateIcon(PlayStateWidget::StateLive);
                return;
            }
            case STRM_MSG_STREAM_VIEW: {
                UltraPlayer::handleMessage(msg);
                break;
            }
            default:{
                break;
            }
        }
        reporter()->reportMessage((STRM_MSG)msg->arg1, msg->arg2);
    }
}

} // namespace Hippo
