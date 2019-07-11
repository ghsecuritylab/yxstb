
#include "UltraPlayerMultipleC20.h"
#include "UltraPlayerAssertions.h"

#include "Message.h"
#include "MessageTypes.h"
#include "MessageHandler.h"
#include "ProgramChannel.h"
#include "UltraPlayerClient.h"
#include "BrowserPlayer.h"
#include "ProgramChannelC20.h"
#include "ResourceManager.h"
#include "BrowserEventQueue.h"

#include "PPVProgram.h"
#include "PPVListInfo.h"

#include <pthread.h>

#include "AppSetting.h"
#include "SysSetting.h"

#include "mid_stream.h"
#include "app_sys.h"

namespace Hippo {
static pthread_mutex_t gMosicMutex = PTHREAD_MUTEX_INITIALIZER;
UltraPlayerMultipleC20::UltraPlayerMultipleC20(UltraPlayerClient *client, BrowserPlayerReporter *reporter, ProgramChannel *program)
	: UltraPlayerMultipleHuawei(client, reporter, program)
{

}

UltraPlayerMultipleC20::~UltraPlayerMultipleC20()
{
    int changeVideoMode = 0;
	sysSettingGetInt("changevideomode", &changeVideoMode, 0);
	if(!changeVideoMode){            //0 is black screen 1 is last frame 2 is smooth change
    	close(UltraPlayer::BlackScreenMode);
    } else {
    	close(UltraPlayer::LastFrameMode);
    }
}


static void stateCall(int pIndex, STRM_STATE state, int rate, unsigned int magic, int callarg)
{
    UltraPlayer *p = UltraPlayer::lockPlayer(pIndex);
    if (p) {
        UltraPlayerMultipleHuawei *player = (UltraPlayerMultipleHuawei *)p;
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

static void mosaicMsgCall(int pIndex, STRM_MSG msg, int arg, unsigned int magic, int callarg)
{
    int tmp_index = UltraPlayerMultipleC20::GetMosaicIndexByKey(pIndex);

    PLAYER_LOG("mosaicMsgCall call index(%d), MosaicIndex(%d)\n", pIndex, tmp_index);
    if (tmp_index < 2)
        return;
    UltraPlayer *p = UltraPlayer::lockPlayer(tmp_index);
    if (p && p == (UltraPlayer *)callarg) {
        UltraPlayer *player = (UltraPlayer *)p;
        Message *message = player->obtainMessage(MessageType_Play, msg, arg);
        player->sendMessage(message);
       // UltraPlayer::unlockPlayer(tmp_index, p);
    }
    UltraPlayer::unlockPlayer(tmp_index, p);

}

#define MAXMOSAICKEY 11
static int mosaicKey[MAXMOSAICKEY] = {-1};
int
UltraPlayerMultipleC20::GetMosaicIndexByKey(int key)
{
    int indexKey = -1;

    pthread_mutex_lock(&gMosicMutex);

    for(int tIndex = 2; tIndex <= MAXMOSAICKEY; tIndex++){
        if(key == mosaicKey[tIndex]){
            indexKey =  tIndex;
        }
    }
    pthread_mutex_unlock(&gMosicMutex);

    return indexKey;
}

void
UltraPlayerMultipleC20::SetMosaicKey(int pIndex, int key)
{
    pthread_mutex_lock(&gMosicMutex);

    if(pIndex > MAXMOSAICKEY){
        PLAYER_LOG_ERROR("UltraPlayerMultipleC20::SetMosaicKey mosaic index() greater than max mosaic key(%d)\n", pIndex, MAXMOSAICKEY);
        return;
    }
    mosaicKey[pIndex] = key;
    pthread_mutex_unlock(&gMosicMutex);

    return;
}

float
UltraPlayerMultipleC20::getRequiredBandwidth()
{
    if (m_PlayerInstanceType == BrowserPlayer::PlayerType_eMosaic) {
        return 0;
    } else {
        return ((ProgramChannelC20 *)mProgram)->GetChanBandwith();
    }
}

int
UltraPlayerMultipleC20::play(unsigned int startTime)
{
    mResourceUserType = SimplePlay;
    mResourceRequirement = Resource::RTM_Bandwidth;

#ifdef RESOURCE_MANAGER
    if (m_PlayerInstanceType != BrowserPlayer::PlayerType_ePIP) {
        if (!resourceManager().requestResource(this)) {
            return -1;
        }
    }
#endif

	UltraPlayer::registerPlayer(mIndex, this);
	if(m_PlayerInstanceType == BrowserPlayer::PlayerType_eMain){
		mid_stream_set_call(mIndex, stateCall, msgCall, (int)this);
#ifdef INCLUDE_SQA
        int fccSwitch = 0;
	    appSettingGetInt("fcc_switch", &fccSwitch, 0);
        if(1 == fccSwitch){
	        PLAYER_LOG("UltraPlayerMultipleC20::play sqacode(%d), retcode(%d)\n",mProgram->GetSqaCode(),mProgram->GetRetCode());
		    mid_stream_set_fcc(0, mProgram->GetSqaCode());
		}
#endif
		appSettingSetInt("lastChannelID", mProgram->GetChanKey());

		removeMessages(MessageType_ConfigSave);
		Message *message = this->obtainMessage(MessageType_ConfigSave, 0, 0);
		this->sendMessageDelayed(message, 3000);

        char buf[2048] = {0};
        std::string json_bgn = "{\"type\":\"EVENT_GO_CHANNEL\",";

        snprintf(buf, sizeof(buf), "%s\"instance_id\":\"%d\",\"channel_code\":\"%d\",\"channel_num\":\"%d\"}", json_bgn.c_str(), \
            mClient->instanceId(), atoi(mProgram->GetChanID().c_str()), mProgram->GetChanKey());
        browserEventSend(buf, NULL);

        setStreamVideoLocation(mIndex, m_VideoDisplaytop,m_VideoDisplayleft,m_VideoDisplaywidth,m_VideoDisplayheight, mDisplayMode);
#if defined(DEBUG_BUILD)
        PLAYER_LOG("UltraPlayerMultipleC20::play channel URL(%s)\n", mProgram->GetChanURL().c_str());
#endif
        mMediaType = APP_TYPE_IPTV;
        //OTT check
        if (strstr((char *)mProgram->GetChanURL().c_str(), ".m3u8")) {
#if defined(DEBUG_BUILD)
            PLAYER_LOG("begin to play http live stream .......url(%s)\n", (char *)mProgram->GetChanURL().c_str());
#endif
            if (strstr((char *)mProgram->GetChanURL().c_str(), "&servicetype=1"))//Servicetype取值：0为Webtv点播、1为Webtv直播、2为Webtv时移、3为Webtv录播
                mMediaType = APP_TYPE_APPLE_IPTV;
            else
                mMediaType = APP_TYPE_APPLE_VOD;
        }
		if(mProgram->GetTimeShift() == 0){
		    if (mMediaType == APP_TYPE_APPLE_IPTV || mMediaType == APP_TYPE_APPLE_VOD) {
		        mSighMagic = mid_stream_open(mIndex, (char *)mProgram->GetChanURL().c_str(), mMediaType, mProgram->GetTimeShiftLength());
		        return 0;
		    }

		    PPVAuthorizedProgram *ppvProgram = ppvListInfo()->checkPPVChannel(atoi(mProgram->GetChanID().c_str()));
			if(ppvProgram){
				int timeshift = -1;
				PLAYER_LOG("UltraPlayerMultipleC20::play ppv channel\n");
#ifdef INCLUDE_cPVR
				int localTimeshiftEnable = 0;
				int localTimeshiftMaxDuration = 0;
				appSettingGetInt("localTimeShift_enable", &localTimeshiftEnable, 0);
				appSettingGetInt("localTimeShift_maxDuration", &localTimeshiftMaxDuration, 0);
				if (localTimeshiftEnable) {
					mMediaType = APP_TYPE_TSTV;
					timeshift = localTimeshiftMaxDuration * 60;
				} else
#endif
					mMediaType = APP_TYPE_IPTV;
				mSighMagic = mid_stream_open_range(mIndex, (char *)mProgram->GetChanURL().c_str(), mMediaType, timeshift, (int)(ppvProgram->m_startTimeNum), (int)(ppvProgram->m_stopTimeNum));
			} else {
                std::string playUrl = mProgram->GetChanURL();
                if (playUrl.find("igmp") == std::string::npos && playUrl.find("IGMP") == std::string::npos) { //WZW modified to fix pc-lint warning 568
                    playUrl.clear();
                    playUrl = mProgram->GetTimeShiftURL();
                }
#ifdef INCLUDE_cPVR
				int localTimeshiftEnable = 0;
				int localTimeshiftMaxDuration = 0;
				appSettingGetInt("localTimeShift_enable", &localTimeshiftEnable, 0);
				appSettingGetInt("localTimeShift_maxDuration", &localTimeshiftEnable, 0);
				if (localTimeshiftEnable) {
					mMediaType = APP_TYPE_TSTV;
					int timeshift = localTimeshiftEnable * 60;
					mSighMagic = mid_stream_open(mIndex, (char *)playUrl.c_str(), mMediaType, timeshift);
				} else
#endif
				{
					mMediaType = APP_TYPE_IPTV;
					mSighMagic = mid_stream_open(mIndex, (char *)playUrl.c_str(), mMediaType, mProgram->GetTimeShiftLength());
				}
			}
		}
		else{
			struct IPTVUrl tPlayUrl = {0};

			strcpy(tPlayUrl.channel_url, (char *)mProgram->GetChanURL().c_str());
			strcpy(tPlayUrl.tmshift_url, (char *)mProgram->GetTimeShiftURL().c_str());
			if (mMediaType == APP_TYPE_IPTV)
			    mMediaType = APP_TYPE_IPTV2;

			mSighMagic = mid_stream_open(mIndex, (char *)&tPlayUrl, mMediaType, mProgram->GetTimeShiftLength());

		}
		//SetSubtitileFlag(m_subtitleFlag);
		return 0;
	}else if(m_PlayerInstanceType == BrowserPlayer::PlayerType_ePIP){
	    appSettingSetInt("pipchannelid", mProgram->GetChanKey());
		removeMessages(MessageType_ConfigSave);
		Message *message = this->obtainMessage(MessageType_ConfigSave, 0, 0);
		this->sendMessageDelayed(message, 3000);
		mid_stream_set_call(mIndex, stateCall, msgCall, (int)this);
		setStreamVideoLocation(mIndex, m_VideoDisplaytop,m_VideoDisplayleft,m_VideoDisplaywidth,m_VideoDisplayheight, mDisplayMode);
#if defined(DEBUG_BUILD)
		PLAYER_LOG("UltraPlayerMultipleC20::play pip url(%s), m_subtitleFlag(%d)\n", mProgram->GetMiniChanURL().c_str(), m_subtitleFlag);
#endif
		mMediaType = APP_TYPE_IPTV;
		mSighMagic = mid_stream_open(mIndex, (char *)mProgram->GetMiniChanURL().c_str(), mMediaType, -1);
		//SetSubtitileFlag(m_subtitleFlag);
	} else {
		int mosaicKey = atoi(mProgram->GetChanID().c_str());
		SetMosaicKey(mIndex, mosaicKey);
		mid_stream_mosaic_setcall(mosaicKey, mosaicMsgCall, (int)this);
		PLAYER_LOG("UltraPlayerMultipleC20::play mosaic url(%s)\n", mProgram->GetMiniChanURL().c_str());
		mSighMagic = mid_stream_mosaic_open(mosaicKey,(char *)mProgram->GetMiniChanURL().c_str(),m_VideoDisplaytop,		\
											m_VideoDisplayleft,m_VideoDisplaywidth,m_VideoDisplayheight,NULL,0);
		if(m_muteFlag == 0){
			mid_stream_mosaic_set(mosaicKey);
		}
	}
	return 0;
}

int
UltraPlayerMultipleC20::close(int mode)
{
	int tmp_index = 0;

    PLAYER_LOG("UltraPlayerMultipleC20::close decoder(%d) by stop mode()\n", mIndex, mode);
	if(m_PlayerInstanceType == BrowserPlayer::PlayerType_eMain){
		mid_stream_close(mIndex, mode);
		tmp_index = mIndex;
	} else if(m_PlayerInstanceType == BrowserPlayer::PlayerType_ePIP){
		mid_stream_close(mIndex, mode);
		tmp_index = mIndex;
	} else {
		mid_stream_mosaic_close(atoi(mProgram->GetChanID().c_str()));
		tmp_index = GetMosaicIndexByKey(atoi(mProgram->GetChanID().c_str()));
		SetMosaicKey(tmp_index, -1);
		PLAYER_LOG("UltraPlayerMultipleC20::close mosaic by index(%d)\n", tmp_index);
	}
	UltraPlayer::unregisterPlayer(tmp_index, this);
	removeMessages(MessageType_Play);
#ifdef RESOURCE_MANAGER
	resourceManager().releaseResource(this);
#endif
	return 0;
}

void
UltraPlayerMultipleC20::SetMosaicMute(int mode)
{
	if( mode == 0){
		mid_stream_mosaic_set(atoi(mProgram->GetChanID().c_str()));
	} else {
		if(mid_stream_mosaic_get() == atoi(mProgram->GetChanID().c_str())){
			mid_stream_mosaic_set(-1);
		}
	}
}

int
UltraPlayerMultipleC20::GetMosaicMute(void)
{
	if(mid_stream_mosaic_get() == atoi(mProgram->GetChanID().c_str())){
		return 0;
	} else {
		return 1;
	}
}

unsigned int
UltraPlayerMultipleC20::getCurrentTimeString(char *TimeString)
{
    unsigned int CurPlayTime = 0;
    CurPlayTime = mid_stream_get_currenttime(mIndex);
    if( MID_UTC_SUPPORT == 0 ){
            int timezone = 0;
			sysSettingGetInt("timezone", &timezone, 0);

            PLAYER_LOG("timezone:%d,flag:%d\n", timezone,CurPlayTime);
            CurPlayTime = CurPlayTime - mid_tool_timezone2sec(timezone);
    }
    mid_tool_time2string(CurPlayTime, TimeString, 'T');
    return 0;
}

int
UltraPlayerMultipleC20::seekTo(unsigned int playTime)
{
    if (mProgram->GetTimeShift()) {
        if (playTime == 0)                //goto start set playTime 0 in timeshift mode
            playTime = mid_time() - getTotalTime();
        mid_stream_seek(mIndex, playTime);
    }

    return 0;
}

unsigned int
UltraPlayerMultipleC20::getTotalTime()
{
	unsigned int totalTime = 0;

	int type = mid_stream_get_apptype(mIndex);
	unsigned int currentTime = mid_time();
	mid_stream_sync_totaltime(0, 5000);
	if (type == APP_TYPE_IPTV){
		totalTime = mProgram->GetTimeShiftLength();
		PPVAuthorizedProgram *ppvProgram = ppvListInfo()->checkPPVChannel(atoi(mProgram->GetChanID().c_str()));
        if (ppvProgram) {
		unsigned int ppvTotalTime = currentTime - ppvProgram->m_startTimeNum;
            totalTime = ppvTotalTime > totalTime ? totalTime : ppvTotalTime;
        }
	} else if (type == APP_TYPE_TSTV){
		int localTimeShift = 0;
		appSettingGetInt("localTimeShift_maxDuration", &localTimeShift, 0);
		localTimeShift = localTimeShift * 60;
		totalTime = currentTime - mid_stream_get_totaltime(mIndex);
		if (totalTime > localTimeShift)
			totalTime = localTimeShift;
	} else {
		totalTime = mid_stream_get_totaltime(mIndex);
	}

	return totalTime;
}

std::string&
UltraPlayerMultipleC20::getMediaCode()
{
    if(mProgram)
	    return mProgram->GetMediaCode();
	return m_mediaCode;
}

} // namespace Hippo
