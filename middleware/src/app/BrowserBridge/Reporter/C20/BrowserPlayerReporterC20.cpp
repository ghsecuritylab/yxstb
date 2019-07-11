#include "BrowserPlayerReporterC20.h"

#include "BrowserPlayerReporter.h"
#include "BrowserPlayerMsgCode.h"
#include "BrowserAssertions.h"
#include "UltraPlayer.h"
#include "UltraPlayerClient.h"
#include "ProgramChannel.h"
#include "Tr069.h"

#include "tr069_define.h"
#include "tr069_api.h"
#include "app_tr069_alarm.h"
#ifdef INCLUDE_cPVR
#include "Tstv.h"
#endif

extern "C" {
	void stream_port_set_rate(int rate,int pIndex);
	void stream_port_set_state(int state,int pIndex);
}

namespace Hippo {
BrowserPlayerReporterC20::BrowserPlayerReporterC20()
{
}

BrowserPlayerReporterC20::~BrowserPlayerReporterC20()
{
}

void
BrowserPlayerReporterC20::reportState(STRM_STATE state, int rate)
{
    int g_state2mode[10] = {
        PLAYMODE_STOP,              //STRM_STATE_CLOSE
        PLAYMODE_STOP,              //STRM_STATE_OPEN
        PLAYMODE_BUFFERING,         //STRM_STATE_ERROR
        PLAYMODE_NORMAL_PLAY,       //STRM_STATE_PLAY
        PLAYMODE_PAUSE,             //STRM_STATE_PAUSE
        PLAYMODE_TRICK_MODE,        //STRM_STATE_FAST
        PLAYMODE_STOP,              //STRM_STATE_STOP
#if VIETTEL_HD
        PLAYMODE_UNICAST_PLAY,
#else
        PLAYMODE_MULTICAST_PLAY,    //STRM_STATE_IPTV
#endif //VIETTEL_HD
        PLAYMODE_PAUSE,             //STRM_STATE_IPAUSE
        PLAYMODE_STOP               //STRM_STATE_LOCK
    };
    int oldmode = 0, playmode = 0;
    std::string mediaCode(""), channelUrl("");

    channelUrl = ((ProgramChannel *)(mPlayer->program()))->GetChanURL();
    if (!strncasecmp(channelUrl.c_str(), "rtsp://", 7))
        g_state2mode[7] = PLAYMODE_UNICAST_PLAY;
    mediaCode = mPlayer->getMediaCode();

    BROWSER_LOG("BrowserPlayerReporterMultipleC20::reportState state=%d, rate=%d\n",  state, rate);
#if VIETTEL_HD
#else
    if ((g_state2mode[m_curState] == g_state2mode[state]
        && (g_state2mode[m_curState] != PLAYMODE_TRICK_MODE || m_curRate == rate))){
        return;
    }
#endif //VIETTEL_HD
    oldmode = g_state2mode[m_curState];
    playmode = g_state2mode[state];
    if(0 == mPlayer->getIndex()){
        std::string errmsg = "null";
        int errcode = 0;
		if ((playmode == PLAYMODE_STOP && oldmode == PLAYMODE_NORMAL_PLAY)
		    || (playmode == PLAYMODE_NORMAL_PLAY && oldmode == PLAYMODE_STOP))
		    ;
		else
            buildEvent(Event_PlayModeChange, getPlayerInstatncdId(), playmode, rate, oldmode, m_curRate, errcode, errmsg, mediaCode, "");
    }
    m_curState = state;
    m_curRate = rate;
    stream_port_set_rate(rate, mPlayer->getIndex());
    stream_port_set_state(state, mPlayer->getIndex());

    BrowserPlayerReporterHuawei::reportState(state, rate);
    return;
}

void
BrowserPlayerReporterC20::reportMessage(STRM_MSG message, int code)
{
    PlayerCode* playerCodeInfo = NULL;
    std::string mediaCode = mPlayer->getMediaCode();

    playerCodeInfo = browserPlayerMsgCode().getMsgCode(message, code);
    if (playerCodeInfo) {
        buildEvent(playerCodeInfo->mEventType, getPlayerInstatncdId(), 0, 0, 0, 0, playerCodeInfo->mEPGCodeC20, playerCodeInfo->mMessageC20, mediaCode, "");
        if (playerCodeInfo->mEventType == Event_MediaError) {
            TR069_DIAGNOSTICS_SET_STATE(DIAGNOSTATICS_STATE_9822);
            buildEvent(Event_STBError, getPlayerInstatncdId(), 0, 0, 0, 0, playerCodeInfo->mEPGCodeC20, playerCodeInfo->mMessageC20, mediaCode, "");
        }
    } else {
        switch (message) {
#ifdef INCLUDE_cPVR
    	case RECORD_MSG_SUCCESS_BEGIN:
            TstvDealRecordMsg(RECORD_START_SUCCESS);
    		break;
    	case RECORD_MSG_DISK_ERROR:
            TstvDealRecordMsg(RECORD_NO_DISK);
    		break;
    	case RECORD_MSG_DATA_DAMAGE:
            TstvDealRecordMsg(RECORD_DISK_FIETYPE_ERR);
    		break;
    	case RECORD_MSG_DISK_FULL:
            TstvDealRecordMsg(RECORD_NO_FREE_SPACE);
    		break;
    	case RECORD_MSG_PVR_CONFLICT:
            TstvDealRecordMsg(RECORD_COLLIDE);
    		break;
    	case RECORD_MSG_DISK_DETACHED:
            TstvDealRecordMsg(RECORD_DISK_REMOVED);
    		break;
    	case RECORD_MSG_ERROR:
            TstvDealRecordMsg(RECORD_DISK_WRITE_ERR);
    		break;
#endif
    	default:
    	    BrowserPlayerReporterHuawei::reportMessage(message, code);
    	    break;
        }
    }

    return;
}



}