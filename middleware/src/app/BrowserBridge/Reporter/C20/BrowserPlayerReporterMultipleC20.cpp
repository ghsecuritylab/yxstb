#include "UltraPlayer.h"
#include "BrowserPlayerReporterMultipleC20.h"
#include "UltraPlayerClient.h"
#include "BrowserPlayer.h"
#include "BrowserAssertions.h"
#include "ProgramChannel.h"
#include "BrowserPlayerMsgCode.h"

#include "app/app_tr069_alarm.h"

namespace Hippo {

BrowserPlayerReporterMultipleC20::BrowserPlayerReporterMultipleC20()
{
}

BrowserPlayerReporterMultipleC20::~BrowserPlayerReporterMultipleC20()
{
}


void
BrowserPlayerReporterMultipleC20::reportMessage(STRM_MSG message, int code)
{
    std::string mediaCode("");

    TR069_REPORT_CHANNEL_ALARM(message);
    mediaCode = mPlayer->getMediaCode();
    if (mediaCode.empty()) {
        mediaCode = ((ProgramChannel *)(mPlayer->program()))->GetChanID();
    } else {
        mediaCode = mPlayer->getMediaCode();
    }
    if (mPlayer->m_PlayerInstanceType == BrowserPlayer::PlayerType_ePIP) {
        int channelnum = mPlayer->GetCurrentChannelNum();
            switch(message) {
            case STRM_MSG_STREAM_VIEW:
            case STRM_MSG_PTS_VIEW:
                buildEvent(Event_GOTOChannel, getPlayerInstatncdId(),0, 0, 0, 0, channelnum, "", mediaCode, "");
                return;
            case STRM_MSG_RECV_TIMEOUT:
#if VIETTEL_HD
                return;
#else
                break;
#endif
            case STRM_MSG_RECV_RESUME:
                break;
            case STRM_MSG_RECV_TIMEOUT15:
#if VIETTEL_HD
                return;
#else
                break;
#endif
            default:
                break;
         }
    } else if (mPlayer->m_PlayerInstanceType == BrowserPlayer::PlayerType_eMosaic) {
        switch(message){
        case MOSAIC_MSG_ERROR:
            BROWSER_LOG("MOSAIC_MSG_ERROR\n");
            break;
        case MOSAIC_MSG_ERROR_RECT:
            BROWSER_LOG("MOSAIC_MSG_ERROR_RECT\n");
            break;
        case MOSAIC_MSG_SUCCESS:
            BROWSER_LOG("MOSAIC_MSG_SUCCESS\n");
            break;
        case MOSAIC_MSG_TIMEOUT:
            BROWSER_LOG("MOSAIC_MSG_TIMEOUT\n");
            message = STRM_MSG_RECV_TIMEOUT;
            break;
        case MOSAIC_MSG_RESUME:
            BROWSER_LOG("MOSAIC_MSG_RESUME\n");
            message = STRM_MSG_RECV_RESUME;
            break;
        default:
            BROWSER_LOG("DEFAULT\n");
            break;
        }
    } else {
        switch (message){
        case STRM_MSG_STREAM_END:
            return;
        case STRM_MSG_PPV_END:
            break;
        default:
            break;
        }
    }
    if (message == STRM_MSG_RECV_TIMEOUT || message == STRM_MSG_RECV_TIMEOUT15) {
        if (code == 1)
            message = STRM_MSG_RECV_IGMP_TIMEOUT;
        else
            message = STRM_MSG_PLAY_RECV_RTSP_TIMEOUT;
    }
    if (message == STRM_MSG_RECV_RESUME) {
        if (code == 1)
            message = STRM_MSG_RECV_IGMP_RESUME;
        else
            message = STRM_MSG_RECV_RTSP_RESUME;
    }

    BrowserPlayerReporterC20::reportMessage(message, code);
    return ;
}

} // namespace Hippo

