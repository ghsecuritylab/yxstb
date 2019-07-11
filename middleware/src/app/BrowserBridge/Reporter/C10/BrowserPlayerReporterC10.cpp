
#include "BrowserPlayerReporterC10.h"
#include "BrowserPlayerReporter.h"

#include "BrowserAssertions.h"
#include "BrowserPlayerMsgCode.h"

#include "UltraPlayer.h"
#include "UltraPlayerClient.h"

#include "Tr069.h"
extern "C" {
	void stream_port_set_rate(int rate,int pIndex);
	void stream_port_set_state(int state,int pIndex);
}

namespace Hippo {

BrowserPlayerReporterC10::BrowserPlayerReporterC10()
{
}

BrowserPlayerReporterC10::~BrowserPlayerReporterC10()
{
}

void
BrowserPlayerReporterC10::reportState(STRM_STATE state, int rate)
{
	int g_state2mode[10] = {
	  PLAYMODE_STOP,              //STRM_STATE_CLOSE
	  PLAYMODE_STOP,              //STRM_STATE_OPEN
	  PLAYMODE_BUFFERING,         //STRM_STATE_BUFFER
	  PLAYMODE_NORMAL_PLAY,       //STRM_STATE_PLAY
	  PLAYMODE_PAUSE,             //STRM_STATE_PAUSE
	  PLAYMODE_TRICK_MODE,        //STRM_STATE_FAST
	  PLAYMODE_STOP,              //STRM_STATE_STOP
	  PLAYMODE_NORMAL_PLAY,       //STRM_STATE_IPTV
	  PLAYMODE_PAUSE,             //STRM_STATE_IPAUSE
	  PLAYMODE_STOP               //STRM_STATE_LOCK
	};

	int oldmode, playmode;

    BROWSER_LOG("Current state(%d) rate(%d), New state(%d)rate(%d)\n",  m_curState, m_curRate, state, rate);

	/* 将本地的rtsp状态转换成a2的事件状态 */
	if ((g_state2mode[m_curState] == g_state2mode[state]
        && (g_state2mode[m_curState] != PLAYMODE_TRICK_MODE || m_curRate == rate))) {
        m_curState = state;
        m_curRate = rate;
        stream_port_set_rate(rate, mPlayer->getIndex());
        stream_port_set_state(state, mPlayer->getIndex());
        BROWSER_LOG_ERROR("DON'T SEND.RETURN\n");
        if(state != STRM_STATE_IPTV)
            return;
    }

    oldmode = g_state2mode[m_curState];
    playmode = g_state2mode[state];
    stream_port_set_rate(rate, mPlayer->getIndex());
    stream_port_set_state(state, mPlayer->getIndex());
    if(mPlayer) {
        switch(playmode) {
        case PLAYMODE_STOP:
            m_PlayingFlag = 0;
            mPlayer->onStop();
            break;
        case PLAYMODE_NORMAL_PLAY:
            if (mPlayer->mPlayState->getState() != PlayStateWidget::StateLive
                && mPlayer->mPlayState->getState() != PlayStateWidget::StateTimeShift
                && mPlayer->mPlayState->getState() != PlayStateWidget::StatePlay) {
                    mPlayer->sendEmptyMessage(MessageType_ClearPlayStateIcon);
                }
            mPlayer->onPlay();
            break;
        case PLAYMODE_TRICK_MODE:
            if(rate == 1)
                mPlayer->onPlay();
            else
                mPlayer->onTrick(rate);
            break;
        default:
            break;
        }
    }
    if(0 == mPlayer->getIndex())
        buildEvent(Event_PlayModeChange, getPlayerInstatncdId(), playmode, rate, oldmode, m_curRate, 0, "null", "null", "null");
    
	BROWSER_LOG("BrowserPlayerReporterHuawei::reportState state(%d)rate(%d)oldmode(%d)oldsate(%d)\n", playmode, rate, oldmode, m_curRate);
    m_curState = state;
    m_curRate = rate;
    
    BrowserPlayerReporterHuawei::reportState(state, rate);
        
	return;
}

void
BrowserPlayerReporterC10::reportMessage(STRM_MSG message, int code)
{
    PlayerCode* playerCodeInfo = NULL;     
    std::string mediaCode(""), entryID("");
    char CurrentPlayUrl[1024] = {0};
    char CurrentRtspCommand[32] = {0};

    mediaCode = mPlayer->getMediaCode();
    entryID = mPlayer->getEntryID();

    BROWSER_LOG("ReportMessag %d\n", message);
    mid_stream_get_rtspInfo(0, CurrentPlayUrl, CurrentRtspCommand);
    
    playerCodeInfo = browserPlayerMsgCode().getMsgCode(message, code);
    if (!playerCodeInfo) {
        BrowserPlayerReporterHuawei::reportMessage(message, code);
        return;
    }
    if (playerCodeInfo->mEventType == Event_MediaError) {        
        mPlayer->sendEmptyMessage(MessageType_ClearAllIcon);
        TR069_DIAGNOSTICS_SET_STATE(DIAGNOSTATICS_STATE_9822);
        TR069_API_SETVALUE("ErrorCode", "", playerCodeInfo->mEPGCodeC10);
    }            
    switch (playerCodeInfo->mPrintType) {
    case Print_ErrCmd:
        BROWSER_LOG_ERROR("Error code:%d, %s, Error Command: %s\n", playerCodeInfo->mEPGCodeC10, playerCodeInfo->mPrintMessage, CurrentRtspCommand);
        break;
    case Print_ErrUrl:
        BROWSER_LOG_ERROR("Error code:%d, %s %s\n", playerCodeInfo->mEPGCodeC10, playerCodeInfo->mPrintMessage, CurrentPlayUrl);        
        break;
    case Print_All:
        BROWSER_LOG_ERROR("Error code:%d, %s %s, Error Command: %s\n", playerCodeInfo->mEPGCodeC10, playerCodeInfo->mPrintMessage, CurrentPlayUrl, CurrentRtspCommand);                
        break;
    case Print_None:
        BROWSER_LOG_ERROR("Error code:RTSP %d, Description: %s, Hms Error URL: %s\n", playerCodeInfo->mEPGCodeC10, playerCodeInfo->mMessageC10, CurrentPlayUrl);
        break;        
    default:
        BROWSER_LOG("MessageCode:%d\n", playerCodeInfo->mStreamMsg);
        break;    
    }
    
    buildEvent(playerCodeInfo->mEventType, getPlayerInstatncdId(), 0, 0, 0, 0, playerCodeInfo->mEPGCodeC10, playerCodeInfo->mMessageC10, mediaCode, entryID);

    return;
}

} // namespace Hippo
