
#include "BrowserPlayerReporterMultipleC10.h"
#include "BrowserPlayerMsgCode.h"

#include "ProgramChannel.h"
#include "UltraPlayerMultipleHuawei.h"
#include "BrowserAssertions.h"

#include "Tr069.h"
#include "app_tr069_alarm.h"

namespace Hippo
{

BrowserPlayerReporterMultipleC10::BrowserPlayerReporterMultipleC10()
{
}

BrowserPlayerReporterMultipleC10::~BrowserPlayerReporterMultipleC10()
{
}

int
BrowserPlayerReporterMultipleC10::getChannelKey()
{
    ProgramChannel *programChannel = ((UltraPlayerMultipleHuawei *)mPlayer)->getProgramChannel();
    if(programChannel)
        return programChannel->GetUserChanID();
    else
        return -1;
}

void
BrowserPlayerReporterMultipleC10::reportState(STRM_STATE state, int rate)
{
    BROWSER_LOG("newState: %d, rate: %d; curSate: %d\n", state, rate, this->m_curState);
    switch(state) { /*  解决切台或通过频道列表进入频道不显示live图标需求 */
    case STRM_STATE_IPTV:
    case STRM_STATE_CLOSE: {
        if(STRM_STATE_PLAY == this->m_curState
           || STRM_STATE_PAUSE == this->m_curState
           || STRM_STATE_FAST == this->m_curState) {
            int channelKey = getChannelKey();
            buildEvent(Event_PLTVModeChange, getPlayerInstatncdId(), 0, 0, 0, 0, channelKey,"", "", "") ;
        }
        break;
    }
    case STRM_STATE_PLAY:
    case STRM_STATE_FAST:
    case STRM_STATE_PAUSE: {
        if(STRM_STATE_IPTV == this->m_curState) {       // enter_pltv_mode();
            int channelKey = getChannelKey();
            buildEvent(Event_PLTVModeChange, getPlayerInstatncdId(), 1, 0, 0, 0, channelKey,"", "", "") ;
        }
        break;
    }
    default:
        break;
    }

    return BrowserPlayerReporterC10::reportState(state, rate);
}

void
BrowserPlayerReporterMultipleC10::reportMessage(STRM_MSG message, int code)
{
    BROWSER_LOG("Messag %d\n", message);

    TR069_REPORT_CHANNEL_ALARM(message);
    switch(message) {
#if defined(GUANGDONG)
    case STRM_MSG_SEEK_BEGIN:
    case STRM_MSG_STREAM_BEGIN:
        return;
#endif
    case STRM_MSG_OPEN_ERROR:
        if (code == RTSP_CODE_Socket_Error) {
            if (m_PlayingFlag == 1)
                code = RTSP_CODE_Socket_Playing_Error;
            else
                code = RTSP_CODE_Socket_Start_Error;
        }
        break;
    case STRM_MSG_RECV_TIMEOUT:
    case STRM_MSG_RECV_TIMEOUT15:
        UltraPlayer::setVideoClearFlag(1);
        if (code == 1)
            message = STRM_MSG_RECV_IGMP_TIMEOUT;
        else {
            if (m_PlayingFlag == 1)
                message = STRM_MSG_PLAY_RECV_RTSP_TIMEOUT;
            else
                message = STRM_MSG_START_RECV_RTSP_TIMEOUT;
        }
        break;
    default:
        break;
    }
    BrowserPlayerReporterC10::reportMessage(message, code);

}

} // namespace Hippo
