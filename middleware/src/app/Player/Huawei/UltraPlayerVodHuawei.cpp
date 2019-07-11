
#include "UltraPlayerVodHuawei.h"
#include "UltraPlayerAssertions.h"

#include "BrowserPlayerReporterHuawei.h"

#include "Message.h"
#include "MessageTypes.h"
#include "MessageHandler.h"
#include "ProgramVOD.h"
#include "ResourceManager.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include "mid_stream.h"
#include "libzebra.h"
#include "sys_msg.h"
#include "mid/mid_time.h"
#include "mid/mid_http.h"
#include "UtilityTools.h"

extern "C" {

void HDPlayerPictureScaleGet(int* w, int* h);
}

namespace Hippo
{

static int sIndex[2] = {0};
#if defined(Chongqing) || defined(C30_EC1308H) // Fixme: should use this macro: INCLUDE_LocalPlayer
#else
static int cache_progress = 0;
static int ymm_stream_progress_callback(YX_PLAYER_PROGRESS type, unsigned int progress, long long duration, long long buf_size)
{
	cache_progress = progress;
	//PLAYER_LOG("########PROGRESS: type=%d, progress=%d, duration=%lld, buf_size=%lld\n", type, progress, duration, buf_size);
	return 0;
}
#endif
static void savepicture(const char* pathName)
{
    char tempName[512 + 5] = {0};

#ifdef INCLUDE_LocalPlayer

    char *p = NULL;
    char *p1 = NULL;
    char path[512 + 5] = {0};
    int surfaceHandle = 0;
    int w = 0;
    int h = 0;

    if (!pathName)
        return;
    if (strlen(pathName) > 512 || !strncmp(pathName, "http://", 7))
        return;
    strcpy(tempName, pathName);
    p = strrchr(tempName, '.');
    if (!p)
        return;
    if (!strcasecmp(p, ".mp3") || !strcasecmp(p, ".wav") || !strcasecmp(p, ".wma"))
        return;
    PLAYER_LOG("save picture\n");
    strcpy(p, ".jpeg");
    if (!access(tempName, R_OK))
        return;

    p1 = strrchr(tempName, '/');
    if (!p1)
        return;

    strncpy(path, tempName, p1 - tempName + 1);
    if (strlen(path) > 512)
        return;

    strcpy(path + strlen(path), "t.jpeg");
    ygp_layer_getScreenSize(&w,  &h);
    int ret = ygp_layer_createSurface(w, h, YX_COLOR_ARGB8888, 0, &surfaceHandle);
    if (ret != YX_OK)
        PLAYER_LOG_ERROR("Create Surface Error\n");
    ret = ymm_decoder_fillSurfaceFromVideo(0, surfaceHandle);
    if (ret != YX_OK)
        PLAYER_LOG_ERROR("fill from video Error!\n");
    ret = ygp_layer_saveToPicture(surfaceHandle, path, 99, YX_PICTURE_JPEG);
    if (ret != YX_OK)
        PLAYER_LOG_ERROR("save picture Error!\n");

   ygp_layer_destroySurface(surfaceHandle);

    int ww = 320;
    int hh = 240;

    HDPlayerPictureScaleGet(&ww, &hh);
    PLAYER_LOG("ww = %d, hh = %d", ww, hh);
    ret = ygp_pic_shrink(path, tempName,  &ww, &hh, 0, 0);
    removeFile(path);

#endif

}

static void localPlayerMsgCall(int pIndex, int msgno, int arg2 /*reserved*/)
{
    int msg;

    switch(msgno) {
#if defined(Chongqing) || defined(C30_EC1308H) // Fixme: should use this macro: INCLUDE_LocalPlayer
#else
    case 0x400:
        if (99 > cache_progress) {
            mid_stream_close(0, 0);
            msg = STRM_MSG_OPEN_ERROR;
            break;
        } else {
            return;
        }
#endif
    case STREAM_BOUND_BEGIN:
        msg = STRM_MSG_STREAM_BEGIN;
        break;
    case STREAM_BOUND_END:
        usleep(500000);
        msg = STRM_MSG_STREAM_END;
        break;
    case STREAM_STATUS_PLAY: {
        UltraPlayer *p1 = UltraPlayer::lockPlayer(pIndex);

        if(p1) {
            UltraPlayerVodHuawei *player1 = (UltraPlayerVodHuawei *)p1;
            Message *message1 = player1->obtainMessage(MessageType_Play, STRM_MSG_STREAM_VIEW, 0);
            player1->sendMessage(message1);
            int playerStartTime = player1->getStartTime();
            PLAYER_LOG("startTime = %d\n", playerStartTime);
            if (playerStartTime > 0) {
                mid_stream_seek(pIndex, playerStartTime);
            }
            UltraPlayer::unlockPlayer(pIndex, p1);
        }
        msg = STRM_MSG_STREAM_BEGIN;
        break;
    }
    case STREAM_STATUS_FAILURE:
        msg = STRM_MSG_OPEN_ERROR;
        break;
    case STREAM_STATUS_STOP:
        msg = STRM_STATE_CLOSE + 0x1000;
        break;
    case STREAM_STATUS_NEXTPLAY:
    default:
        return;
    }
    UltraPlayer *p = UltraPlayer::lockPlayer(pIndex);
    if(p) {
        UltraPlayerVodHuawei *player = (UltraPlayerVodHuawei *)p;
        Message *message = player->obtainMessage(MessageType_Play, msg, 0);
        player->sendMessage(message);
        if (msgno == STREAM_BOUND_BEGIN) {
            Message *msgSeekBegin = player->obtainMessage(MessageType_Play, 0x1000 + STRM_STATE_PLAY, 0);
            player->sendMessage(msgSeekBegin);
        }
        if (msg == STRM_MSG_STREAM_BEGIN) {
            Message *msgTimer = player->obtainMessage(MessageType_Timer, 0x100, 0);
            player->sendMessageDelayed(msgTimer, 3000);
        }
        UltraPlayer::unlockPlayer(pIndex, p);
    }

}

static void stateCall(int pIndex, STRM_STATE state, int rate, unsigned int magic, int callarg)
{
    PLAYER_LOG("UltraPlayerVodHuawei stateCall from decoder(%d) state(%d)\rate(%d)n", pIndex, state, rate);
    UltraPlayer *p = UltraPlayer::lockPlayer(pIndex);
    if(p) {
        UltraPlayerVodHuawei *player = (UltraPlayerVodHuawei *)p;
        Message *message = player->obtainMessage(MessageType_Play, state + 0x1000, rate);
        player->sendMessage(message);
        PLAYER_LOG("UltraPlayerVodHuawei stateCall state(%d) from decoder(%d) to InstanceId(%d)\n", stateCall, pIndex, player->mInstanceId);
        UltraPlayer::unlockPlayer(pIndex, p);
    }
}

static void msgCall(int pIndex, STRM_MSG msg, int arg, unsigned int magic, int callarg)
{
    PLAYER_LOG("UltraPlayerVodHuawei msgCall from decoder(%d) msg(%d)\n", pIndex, msg);
    UltraPlayer *p = UltraPlayer::lockPlayer(pIndex);
    if(p) {
        UltraPlayer *player = (UltraPlayer *)p;
        Message *message = player->obtainMessage(MessageType_Play, msg, arg);
        player->sendMessage(message);
        PLAYER_LOG("UltraPlayerVodHuawei msgCall message(%d) from decoder(%d) to InstanceId(%d)\n", msg, pIndex, player->mInstanceId);
        UltraPlayer::unlockPlayer(pIndex, p);
    }
}

#define IN
#define OUT

typedef int (*LPFNPC1)( IN const char *url_str,
                  IN OUT void **Handle,
                  IN OUT char **contentType,
                  OUT long long *contentLength,
                  OUT int *httpStatus,
                  IN int timeout );

typedef int (*LPFNPC2)( IN void *Handle );

void httpProcLocation(int result, char * buf, int len, int arg)
{
    if (!buf)
        return;
    PLAYER_LOG("Enter httpProcLocation\n");
    if (result == HTTP_OK_LOCATION) {
        char *location_url = (char *)malloc(len + 1);
        memcpy(location_url, buf, len);
        location_url[len] = 0;
	    if(strstr(location_url, ".m3u8")){
            PLAYER_LOG("enter location play m3u8 url=%s\n", location_url);
            mid_stream_open(arg, location_url, APP_TYPE_APPLE_VOD, 0);
	    } else if(strstr(location_url, ".mp4")){
            PLAYER_LOG("enter location play MP4 url=%s\n", location_url);
            RegisterReceivedVideoMessageCallback(arg, localPlayerMsgCall);
            mid_stream_open(arg, location_url, APP_TYPE_ZEBRA, 0);
	    } else {
            PLAYER_LOG("enter 302 location and transfer arg =%d#\n", arg);
	        mid_http_call(location_url, (mid_http_f)httpProcLocation, arg, NULL, 0, NULL);
	  	}
        free(location_url);
    } else if(result == HTTP_ERROR_DATA){
         RegisterReceivedVideoMessageCallback(arg, localPlayerMsgCall);
        mid_stream_open(arg, buf, APP_TYPE_ZEBRA, 0);
    }
    return;
}

UltraPlayerVodHuawei::UltraPlayerVodHuawei(UltraPlayerClient *client, BrowserPlayerReporter *pReporter, ProgramVOD *pProgram)
    : UltraPlayerVod(client, pReporter, pProgram)
    , mProgram(pProgram)
    , mSighMagic(0)
    , mStartTime(0)
{
    mProgram->ref();
}

UltraPlayerVodHuawei::~UltraPlayerVodHuawei()
{

    PLAYER_LOG("UltraPlayerVodHuawei::~UltraPlayerVodHuawei %d\n", mIndex);
    sIndex[mIndex] = 0;
    close(UltraPlayer::BlackScreenMode);
    mProgram->unref();
}

float
UltraPlayerVodHuawei::getRequiredBandwidth()
{
    VodSource *source = mProgram->getVodSource(0);
    if(source == NULL)
        return 0.0;
    return source->GetBandwidth();
}

int
UltraPlayerVodHuawei::onAcquireResource()
{
    return 0;
}

int
UltraPlayerVodHuawei::onLoseResource()
{
    return 0;
}

int
UltraPlayerVodHuawei::closeForResource()
{
    return 0;
}

int
UltraPlayerVodHuawei::open()
{
    return 0;
}

int
UltraPlayerVodHuawei::play(unsigned int startTime)
{
    mResourceUserType = SimplePlay;
    mResourceRequirement = Resource::RTM_Bandwidth;
    int VodSourceCount = 0;
    PLAYER_LOG("UltraPlayerVodHuawei::play !\n");

#ifdef RESOURCE_MANAGER
    if(!resourceManager().requestResource(this)) {
        PLAYER_LOG_ERROR("UltraPlayerVodHuawei::play error !\n");
        return -1;
    }
#endif

    if(mProgram->getVodSource(0)->GetUrl().find("http") != std::string::npos
       && (mProgram->getVodSource(0)->GetUrl().find(".mp3") != std::string::npos
       	   || mProgram->getVodSource(0)->GetUrl().find(".wav") != std::string::npos)) {	//WZW modified to fix pc-lint warning 568
        if(sIndex[0] == 0) {
            mIndex = 0;
        } else if(sIndex[1] == 0) {
            mIndex = 1;
        } else {
            PLAYER_LOG("UltraPlayerVodHuawei::play have free decoder !\n");
            PLAYER_LOG_ERROR("UltraPlayerVodHuawei::play error !\n");
            return -1;
        }
    }
    if (mIndex >= 2)
        mIndex = 0;
    sIndex[mIndex] = mInstanceId;
    PLAYER_LOG("UltraPlayerVodHuawei::play mInstanceId(%d)-sIndex[0](%d)-sIndex[1](%d)-mIndex(%d)\n", mInstanceId, sIndex[0], sIndex[1], mIndex);
    mStartTime = startTime;
    mPlayStartTime = mid_10ms();
    UltraPlayer::registerPlayer(mIndex, this);
    mid_stream_set_call(mIndex, stateCall, msgCall, (int)this);

    int mediaType = mProgram->getVodSource(0)->GetMediaType();

    VodSourceCount = mProgram->getVodSourceCount();
    PLAYER_LOG("UltraPlayerVodHuawei::play Vod Source Count(%d)\n", VodSourceCount);

    if(VodSourceCount <= 0) {
        PLAYER_LOG_ERROR("UltraPlayerVodHuawei::play error !\n");
        return -1;
    } else if(VodSourceCount > 1) {
        int i = 0, j = 0;
        struct VODAdv list;

        memset(&list, 0, sizeof(VODAdv));
        list.adv_num = VodSourceCount - 1;
        for(i = 0; i < VodSourceCount; i ++) {
            if(mProgram->getVodSource(i)->GetIsPostive()) {
                strcpy(list.url, mProgram->getVodSource(i)->GetUrl().c_str());
            } else {
                list.adv_array[j].insert = mProgram->getVodSource(i)->GetInsertTime();
                strcpy(list.adv_array[j].url, mProgram->getVodSource(i)->GetUrl().c_str());
                list.adv_array[j].apptype = 0;
                j ++;
            }
        }
        mMediaType = APP_TYPE_VOD;
        mSighMagic = mid_stream_open_vodAdv(mIndex, &list, startTime);
    } else {
        setStreamVideoLocation(mIndex, m_VideoDisplaytop, m_VideoDisplayleft, m_VideoDisplaywidth, m_VideoDisplayheight, mDisplayMode);
#if defined(DEBUG_BUILD)
        PLAYER_LOG("UltraPlayerVodHuawei::play url(%s) media type(%d) by decoder(%d)\n", (char *)mProgram->getVodSource(0)->GetUrl().c_str(), mediaType, mIndex);
#endif
        if(mediaType == ProgramVOD::TYPE_STBMONITOR) { //stbmonitor play Unicast channel
            if (!strncmp(mProgram->getVodSource(0)->GetUrl().c_str(), "igmp://", 7)
                 || (!strncmp(mProgram->getVodSource(0)->GetUrl().c_str(), "rtsp://", 7)
                     && strstr(mProgram->getVodSource(0)->GetUrl().c_str(), ".smil"))) {
                mMediaType = APP_TYPE_IPTV;
                mSighMagic = mid_stream_open(mIndex, (char *)mProgram->getVodSource(0)->GetUrl().c_str(), mMediaType, 3600);
                return 0;
            }
        }
        if(mediaType == ProgramVOD::TYPE_MUSIC && !strstr((char *)mProgram->getVodSource(0)->GetUrl().c_str(), "rtsp://")) {
            if(mProgram->getVodSource(0)->GetUrl().find(".wav") != std::string::npos) {
                mMediaType = APP_TYPE_HTTP_PCM;
            } else {
                mMediaType = APP_TYPE_HTTP_MPA;
            }
            mSighMagic = mid_stream_open(mIndex, (char *)mProgram->getVodSource(0)->GetUrl().c_str(), mMediaType, 0);
            return 0;
        }
        if (mediaType == ProgramVOD::TYPE_DLNA) {
            setStreamVideoLocation(mIndex, 0, 0, 0, 0, 1);
            RegisterReceivedVideoMessageCallback(mIndex, localPlayerMsgCall);
            mMediaType = APP_TYPE_ZEBRA;
            mSighMagic = mid_stream_open(mIndex, (char *)mProgram->getVodSource(0)->GetUrl().c_str(), mMediaType, 0);
            return 0;
        }
        if(mediaType == ProgramVOD::TYPE_HDPlayer || mediaType == ProgramVOD::TYPE_PVR || mediaType == ProgramVOD::TYPE_DOWNLOAD || mediaType == ProgramVOD::TYPE_VOD) {
            char filePath[512] = {0};
            char *pos = NULL;

            strcpy(filePath, mProgram->getVodSource(0)->GetUrl().c_str()); /* e.g. file:///mnt/usb1/eg.mpg */
            if(mediaType == ProgramVOD::TYPE_HDPlayer || mediaType == ProgramVOD::TYPE_VOD) {
                if(!strncmp(filePath, "file://", 7)) {
                    pos = filePath + 7;
                    setStreamVideoLocation(mIndex, 0, 0, 0, 0, 1);
                    RegisterReceivedVideoMessageCallback(mIndex, localPlayerMsgCall);
                    mMediaType = APP_TYPE_ZEBRA;
                    mSighMagic = mid_stream_open(mIndex, pos, mMediaType, 0);
                    m_pathName = pos;
#if defined(Chongqing) || defined(C30_EC1308H) // Fixme: should use this macro: INCLUDE_LocalPlayer
#else
                    ymm_stream_setProgressCallback(ymm_stream_progress_callback);
#endif
                    //SetSubtitileFlag(0);
                    return 0;
                }
            } else if(mediaType == ProgramVOD::TYPE_PVR) {
                if(!strncmp(filePath, "file:///", 8)) {
                    pos = filePath + 8;
                    mMediaType = APP_TYPE_PVR;
                    mSighMagic = mid_stream_open(mIndex, pos, mMediaType, 0);
                    //SetSubtitileFlag(0);
                    return 0;
                }
            } else {
                if(!strncmp(filePath, "downloadfile:///", 16)) {
                    pos = filePath + 16;
                    mMediaType = APP_TYPE_PVR;
                    mSighMagic = mid_stream_open(mIndex, pos, mMediaType, 0);
                    //SetSubtitileFlag(0);
                    return 0;
                }
            }
        }

        const int mPlayerStartTime = atoi(mProgram->getVodSource(0)->GetStartTime().c_str());
        const int mPlayerEndTime = atoi(mProgram->getVodSource(0)->GetEndTime().c_str());
		mMediaType = APP_TYPE_VOD;
        if(strstr((char *)mProgram->getVodSource(0)->GetUrl().c_str(), "http://")) {
            //OTT check
            if (strstr((char *)mProgram->getVodSource(0)->GetUrl().c_str(), ".m3u8")) {
#if defined(DEBUG_BUILD)
                PLAYER_LOG("begin to play http live stream m3u8 .......url(%s)\n", (char *)mProgram->getVodSource(0)->GetUrl().c_str());
#endif
                mMediaType = APP_TYPE_APPLE_VOD;
            } else if (strstr((char *)mProgram->getVodSource(0)->GetUrl().c_str(), ".ts")) {
                mMediaType = APP_TYPE_HTTP;
            } else if(strstr((char *)mProgram->getVodSource(0)->GetUrl().c_str(), ".mp4")) {
#if defined(DEBUG_BUILD)
                PLAYER_LOG("begin to play mp4 stream .......url(%s)\n", (char *)mProgram->getVodSource(0)->GetUrl().c_str());
#endif
                setStreamVideoLocation(mIndex, 0, 0, 0, 0, 1);
                RegisterReceivedVideoMessageCallback(mIndex, localPlayerMsgCall);
                mMediaType = APP_TYPE_ZEBRA;
            } else {
                PLAYER_LOG("enter 302 location and transfer arg mIndex=%d#\n", mIndex);
                int ret = mid_http_call((char *)mProgram->getVodSource(0)->GetUrl().c_str(), (mid_http_f)httpProcLocation, mIndex, NULL, 0, NULL);
                return 0;
            }
        }
        if(startTime > 0) {
            mSighMagic = mid_stream_open(mIndex, (char *)mProgram->getVodSource(0)->GetUrl().c_str(), mMediaType, startTime);
        } else {
            if(!mPlayerEndTime && !mPlayerStartTime) {
                mSighMagic = mid_stream_open(mIndex, (char *)mProgram->getVodSource(0)->GetUrl().c_str(), mMediaType, startTime);
            } else if(!mPlayerEndTime) {
                mSighMagic = mid_stream_open(mIndex, (char *)mProgram->getVodSource(0)->GetUrl().c_str(), mMediaType, (mPlayerStartTime));
            } else if(!mPlayerStartTime) {
                mSighMagic = mid_stream_open_range(mIndex, (char *)mProgram->getVodSource(0)->GetUrl().c_str(), mMediaType, 0, 0, (mPlayerEndTime));
            } else {
                mSighMagic = mid_stream_open_range(mIndex, (char *)mProgram->getVodSource(0)->GetUrl().c_str(), mMediaType, 0, (mPlayerStartTime), (mPlayerEndTime));
            }
        }
    }
    return 0;
}

int
UltraPlayerVodHuawei::seekTo(unsigned int playTime)
{
    mid_stream_seek(mIndex, playTime);
    return -1;
}

int
UltraPlayerVodHuawei::fastForward(int speed)
{
    mid_stream_fast(mIndex, speed);
    return 0;
}

int
UltraPlayerVodHuawei::fastRewind(int speed)
{
    mid_stream_fast(mIndex, speed);
    return 0;
}

int
UltraPlayerVodHuawei::pause()
{
    mid_stream_pause(mIndex);
    return 0;
}

int
UltraPlayerVodHuawei::resume()
{
    mid_stream_resume(mIndex);
    return 0;
}

int
UltraPlayerVodHuawei::stop()
{
    sIndex[mIndex] = 0;
    mid_stream_stop(mIndex);
    return 0;
}

int
UltraPlayerVodHuawei::close(int mode)
{
    sIndex[mIndex] = 0;
    mid_stream_close(mIndex, mode);
    UltraPlayer::ClearAllIcon();
    UltraPlayer::unregisterPlayer(mIndex, this);
    removeMessages(MessageType_Play);
    removeMessages(MessageType_Timer);
    RegisterReceivedVideoMessageCallback(0, 0);
#ifdef RESOURCE_MANAGER
    resourceManager().releaseResource(this);
#endif
    return 0;
}

int
UltraPlayerVodHuawei::seekEnd()
{
    mid_stream_seek(mIndex, getTotalTime());
    return 0;
}

unsigned int
UltraPlayerVodHuawei::getTotalTime()
{
    PLAYER_LOG("UltraPlayerVodHuawei::getTotalTime %d\n", mid_stream_get_totaltime(mIndex));
    return mid_stream_get_totaltime(mIndex);
}

unsigned int
UltraPlayerVodHuawei::getCurrentTime()
{
    PLAYER_LOG("UltraPlayerVodHuawei::currenttime %d\n", mid_stream_get_currenttime(mIndex));
    return mid_stream_get_currenttime(mIndex);
}

unsigned int
UltraPlayerVodHuawei::getCurrentTimeString(char *TimeString)
{
    if(TimeString == 0) {
        return 1;
    }
    sprintf(TimeString, "%d", mid_stream_get_currenttime(mIndex));
    PLAYER_LOG("UltraPlayerVodHuawei::getCurrentTime %s\n", TimeString);
    return 0;
}

std::string&
UltraPlayerVodHuawei::getMediaCode()
{
    if(mProgram)
        return mProgram->getVodSource(0)->GetMediaCode();
    return m_mediaCode;
}

std::string&
UltraPlayerVodHuawei::getEntryID()
{
    if(mProgram)
        return mProgram->getVodSource(0)->GetEntryID();
    return m_entryId;
}

void
UltraPlayerVodHuawei::handleMessage(Message *msg)
{
    if (msg->what == MessageType_Timer && msg->arg1 == 0x100) {
    	if (!m_pathName.empty()) {
               savepicture(m_pathName.c_str());
               m_pathName.clear();
    	}
    	return;
    }
    if(msg->what >= MessageType_ConfigSave && msg->what <= MessageType_ClearAllIcon) {
        return UltraPlayer::handleMessage(msg);
    }
    PLAYER_LOG("Ultraplayer huawei vod receive msessage what(0x%x), info(0x%x & 0x%x)\n", msg->what, msg->arg1, msg->arg2);
    if(msg->arg1 >= 0x1000) {
        switch(msg->arg1 - 0x1000) {
        case STRM_STATE_PAUSE: {
            UltraPlayer::ShowPlayStateIcon(PlayStateWidget::StatePause);
            break;
        }
        case STRM_STATE_PLAY: {
            UltraPlayer::ShowPlayStateIcon(PlayStateWidget::StatePlay);
            break;
        }
        case STRM_STATE_FAST: {
            switch(msg->arg2) {
            case 2: {
                UltraPlayer::ShowPlayStateIcon(PlayStateWidget::StateFW2);
                break;
            }
            case 4: {
                UltraPlayer::ShowPlayStateIcon(PlayStateWidget::StateFW4);
                break;
            }
            case 8: {
                UltraPlayer::ShowPlayStateIcon(PlayStateWidget::StateFW8);
                break;
            }
            case 16: {
                UltraPlayer::ShowPlayStateIcon(PlayStateWidget::StateFW16);
                break;
            }
            case 32: {
                UltraPlayer::ShowPlayStateIcon(PlayStateWidget::StateFW32);
                break;
            }
            case -2: {
                UltraPlayer::ShowPlayStateIcon(PlayStateWidget::StateRW2);
                break;
            }
            case -4: {
                UltraPlayer::ShowPlayStateIcon(PlayStateWidget::StateRW4);
                break;
            }
            case -8: {
                UltraPlayer::ShowPlayStateIcon(PlayStateWidget::StateRW8);
                break;
            }
            case -16: {
                UltraPlayer::ShowPlayStateIcon(PlayStateWidget::StateRW16);
                break;
            }
            case -32: {
                UltraPlayer::ShowPlayStateIcon(PlayStateWidget::StateRW32);
                break;
            }
            default: {
                break;
            }
            }
            break;
        }
        case STRM_STATE_CLOSE: {
            if (-1 != mCurrentStatus)
                UltraPlayer::ShowPlayStateIcon(PlayStateWidget::StateStop);
            break;
        }
        default: {
            break;
        }
        }
        mCurrentStatus = msg->arg1 - 0x1000;
        mCurrentSpeed = msg->arg2;
        reporter()->reportState((STRM_STATE)mCurrentStatus, mCurrentSpeed);
    } else {
        switch(msg->arg1) {
        case STRM_MSG_STREAM_VIEW: {
            UltraPlayer::ShowPlayStateIcon(PlayStateWidget::StatePlay);
            UltraPlayer::handleMessage(msg);
            break;
        }
        case STRM_MSG_STREAM_END: {
            UltraPlayer::ClearAllIcon();
            sIndex[mIndex] = 0;
#ifndef GUANGDONG //All guangdong EPG this flag now is 0
            if(!mPlayCycleFlag) {
                PLAYER_LOG("UltraPlayerVodHuawei::handleMessage mPlayCycleFlag %d\n", mPlayCycleFlag);
                close(LastFrameMode);
                play(0);
                UltraPlayer::ShowPlayStateIcon(PlayStateWidget::StatePlay);
                return ;
            }
#endif //GUANGDONG
            break;
        }
        case STRM_MSG_ADVERTISE_BEGIN: {
            UltraPlayer::ShowPlayStateIcon(PlayStateWidget::StatePlay);
            break;
        }
        default: {
            break;
        }
        }
        reporter()->reportMessage((STRM_MSG)msg->arg1, msg->arg2);
    }
}

} // namespace Hippo
