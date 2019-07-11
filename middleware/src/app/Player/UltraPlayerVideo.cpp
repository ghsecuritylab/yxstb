
#include "UltraPlayerVideo.h"
#include "VideoBrowserPlayer.h"
#include "BrowserEventQueue.h"

#include "libzebra.h"
#include "mid_stream.h"
#include "sys_msg.h"
#include "yxsdk_enum.h"

namespace Hippo {

static void _ReceivedVideoMessage(int what/*state*/, int arg1/*playID*/, int arg2/*reserved*/)
{

}

UltraPlayerVideo::UltraPlayerVideo(UltraPlayerClient *client, BrowserPlayerReporter *pReporter, Program *pProgram)
    : UltraPlayer(client, pReporter, pProgram)
{

}

UltraPlayerVideo::~UltraPlayerVideo()
{
    this->close(0);
}

int
UltraPlayerVideo::play(unsigned int startTime)
{
    printf("UltraPlayerVideo::play \n");

    RegisterReceivedVideoMessageCallback(mClient->instanceId(), _ReceivedVideoMessage); //play state callback register
    mid_stream_open(mIndex, (char*)m_playerUrl.c_str(), APP_TYPE_ZEBRA, 0);

    return 0;
}

int
UltraPlayerVideo::pause()
{
    mid_stream_pause(mIndex);

    return 0;
}

int
UltraPlayerVideo::resume()
{
    mid_stream_resume(mIndex);

    return 0;
}

int
UltraPlayerVideo::seekTo(unsigned int playTime)
{
    mid_stream_seek(mIndex, playTime);

    return 0;
}

int
UltraPlayerVideo::fastForward(int speed)
{
    mid_stream_fast(mIndex, speed);

    return 0;
}

int
UltraPlayerVideo::fastRewind(int speed)
{
    mid_stream_fast(mIndex, speed);

    return 0;
}

int
UltraPlayerVideo::close(int mode)
{
    mid_stream_close(mIndex, mode);
    RegisterReceivedVideoMessageCallback(0, 0); //callback unregister
    return 0;
}

unsigned int
UltraPlayerVideo::getTotalTime()
{
    return mid_stream_get_totaltime(mIndex);
}

unsigned int
UltraPlayerVideo::getCurrentTime()
{
    return mid_stream_get_currenttime(mIndex);
}

}







