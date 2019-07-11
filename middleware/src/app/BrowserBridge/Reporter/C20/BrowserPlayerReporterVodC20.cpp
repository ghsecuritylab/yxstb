
#include "BrowserPlayerReporterVodC20.h"
#include "BrowserAssertions.h"

#include "UltraPlayer.h"

namespace Hippo {

BrowserPlayerReporterVodC20::BrowserPlayerReporterVodC20()
{
}

BrowserPlayerReporterVodC20::~BrowserPlayerReporterVodC20()
{
}

void
BrowserPlayerReporterVodC20::reportMessage(STRM_MSG message, int code)
{
    switch(message) {
    case STRM_MSG_RECV_TIMEOUT:
    case STRM_MSG_RECV_TIMEOUT15:
       message = STRM_MSG_PLAY_RECV_RTSP_TIMEOUT;
       break;
    case STRM_MSG_RECV_RESUME:
       message = STRM_MSG_RECV_RTSP_RESUME;
       break;       
    default:
        break;           
    }
    BrowserPlayerReporterC20::reportMessage(message, code);

    return ;
}

} // namespace Hippo
