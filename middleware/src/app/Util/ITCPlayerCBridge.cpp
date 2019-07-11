
#include "ITCPlayerCBridge.h"

#include "ITCPlayer.h"


extern "C" int ITCPlayerOpen(int index, char *streamUrl, int type)
{
    return Hippo::mainITCPlayer()->open(streamUrl, type);
}

extern "C" int ITCPlayerPlay(int index, unsigned int startTime)
{
    return Hippo::mainITCPlayer()->play(startTime);
}

extern "C" int ITCPlayerSeek(int index, unsigned int playTime)
{
    return Hippo::mainITCPlayer()->seekTo(playTime);
}

extern "C" int ITCPlayerFastForward(int index, int speed)
{
    return Hippo::mainITCPlayer()->fastForward(speed);
}

extern "C" int ITCPlayerFastRewind(int index, int speed)
{
    return Hippo::mainITCPlayer()->fastRewind(speed);
}

extern "C" int ITCPlayerPause(int index)
{
    return Hippo::mainITCPlayer()->pause();
}

extern "C" int ITCPlayerResume(int index)
{
    return Hippo::mainITCPlayer()->resume();
}

extern "C" int ITCPlayerStop(int index)
{
    return Hippo::mainITCPlayer()->stop();
}

extern "C" int ITCPlayerClose(int index)
{
    return Hippo::mainITCPlayer()->close();
}
