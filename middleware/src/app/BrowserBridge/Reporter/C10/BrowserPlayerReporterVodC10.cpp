
#include "BrowserPlayerReporterVodC10.h"
#include "BrowserAssertions.h"
#include "BrowserPlayerMsgCode.h"

#include "UltraPlayer.h"
#include "Session.h"

#include "mid_stream.h"
#include "app/app_sys.h"

namespace Hippo
{

BrowserPlayerReporterVodC10::BrowserPlayerReporterVodC10()
{
}

BrowserPlayerReporterVodC10::~BrowserPlayerReporterVodC10()
{
}

void
BrowserPlayerReporterVodC10::reportMessage(STRM_MSG message, int code)
{
    switch (message) {
    case STRM_MSG_PTS_VIEW:
#if defined(Chongqing) || defined(HUBEI_HD) // Hubei current EPG to send this message now leads to vod broadcast has paused.
#else
    if(session().getPlatform() != PLATFORM_ZTE)
        BrowserPlayerReporterC10::reportMessage(message, code);
#endif
        break;
    case STRM_MSG_SEEK_BEGIN:
        buildEvent(Event_PlayModeChange, getPlayerInstatncdId(), PLAYMODE_NORMAL_PLAY, 1, PLAYMODE_PAUSE, 0, 0, "", "", "");
        break;
    case STRM_MSG_SEEK_END:
#if defined(LIAONING_HD) || defined(LIAONING_SD)
        BrowserPlayerReporterC10::reportMessage(message, code);
#endif
        break;
    case STRM_MSG_RECV_TIMEOUT:
    case STRM_MSG_RECV_TIMEOUT15:
        if (code == 1)
            message = STRM_MSG_RECV_IGMP_TIMEOUT;
        else {
            if (m_PlayingFlag == 1)
                message = STRM_MSG_PLAY_RECV_RTSP_TIMEOUT;
            else
                message = STRM_MSG_START_RECV_RTSP_TIMEOUT;
        }
       BrowserPlayerReporterC10::reportMessage(message, code);
       break;
    case STRM_MSG_OPEN_ERROR:
        if (code == RTSP_CODE_Socket_Error) {
            if (m_PlayingFlag == 1)
                code = RTSP_CODE_Socket_Playing_Error;
            else
                code = RTSP_CODE_Socket_Start_Error;
        }
        BrowserPlayerReporterC10::reportMessage(message, code);
        break;
    default:
        BrowserPlayerReporterC10::reportMessage(message, code);
        break;
    }

    return;
}

} // namespace Hippo
