
#include "ITCPlayer.h"

#include "Message.h"
#include "UltraPlayer.h"
#include "UltraPlayerUtility.h"
#include "Program.h"
#include "ProgramParser.h"
#include "SystemManager.h"

#include <unistd.h> //for sleep

namespace Hippo {

int ITCPlayer::mMaxPlayers = 100;

class OpenParameter : public Object {
public:
	std::string url;
	int type;
};

Result playerResult;

ITCPlayer::ITCPlayer()
	: ITC(&playerResult)
{
}

ITCPlayer::~ITCPlayer()
{
}

int 
ITCPlayer::open(char *streamUrl, int pType)
{
    OpenParameter *param = new OpenParameter();
    param->url = streamUrl;
    param->type = pType;

    call(Open, /*index*/0, /*arg*/0, param);
    param->unref();

    return playerResult.u.iValue;
}

int 
ITCPlayer::play(unsigned int startTime)
{
    call(Play, /*index*/0, /*arg*/0);
    return playerResult.u.iValue;
}

int 
ITCPlayer::seekTo(unsigned int playTime)
{
    call(Seek, /*index*/0, /*arg*/0);
    return playerResult.u.iValue;
}

int 
ITCPlayer::fastForward(int)
{
    call(FastForward, /*index*/0, /*arg*/0);
    return playerResult.u.iValue;
}

int 
ITCPlayer::fastRewind(int)
{
    call(FastRewind, /*index*/0, /*arg*/0);
    return playerResult.u.iValue;
}

int 
ITCPlayer::pause()
{
printf("ITCPlayer::pause() in !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    call(Pause, /*index*/0, /*arg*/0);
printf("ITCPlayer::pause() out !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    return playerResult.u.iValue;
}

int 
ITCPlayer::resume()
{
printf("ITCPlayer::resume() in !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    call(Resume, /*index*/0, /*arg*/0);
printf("ITCPlayer::resume() out !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    return playerResult.u.iValue;
}

int 
ITCPlayer::stop()
{
    call(Stop, /*index*/0, /*arg*/0);
    return playerResult.u.iValue;
}

int 
ITCPlayer::close()
{
    call(Close, /*index*/0, /*arg*/0);
    return playerResult.u.iValue;
}

void 
ITCPlayer::onDestroy()
{
    //mActualPlayer = NULL;
}

void 
ITCPlayer::handleMessage(Message *msg)
{
    UltraPlayer *player;
#if 0
    int index = msg->arg1;

    if (index == mMaxPlayers) {
        player = systemManager().obtainMainPlayer();
        if (msg->what == Open) {
            systemManager().releaseMainPlayer(player);
            systemManager().destoryMainPlayer();

            player = NULL;
        }
        else if (player == NULL) {
        }
    }
    else {
        player = NULL;
    }
#endif

    playerResult.type = Result::Int;
    playerResult.u.iValue = -1;

    if (msg->what == Open) {
        OpenParameter *param = (OpenParameter *)msg->obj;

        Program *program = programParser().parseSingleMedia(param->url.c_str());
        if (program == NULL)
            goto Exit;

        systemManager().destoryMainPlayer();

        player = UltraPlayerUtility::createPlayerByProgram(this, NULL, program);
        if (!player)
            goto Exit;
        program->unref();
        systemManager().attachMainPlayer(player);
        player->unref();
    }

    player = systemManager().obtainMainPlayer();
    if (player == NULL /*|| (player->magicNumber() != mPlayerMagic)*/) {
        systemManager().releaseMainPlayer(player);
        goto Exit;
    }

    switch (msg->what) {
        case Open:
            playerResult.u.iValue = player->open();
            break;
        case Play:
            playerResult.u.iValue = player->play(msg->arg2);
            break;
        case Seek:
            playerResult.u.iValue = player->seekTo(msg->arg2);
            break;
        case FastForward:
            playerResult.u.iValue = player->fastForward(msg->arg2);
            break;
        case FastRewind:
            playerResult.u.iValue = player->fastRewind(msg->arg2);
            break;
        case Pause:
printf("ITCPlayer::handleMessage() Pause in !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
            playerResult.u.iValue = player->pause();
            sleep(2);
printf("ITCPlayer::handleMessage() Pause out !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
            break;
        case Resume:
printf("ITCPlayer::handleMessage() Resume in !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
            playerResult.u.iValue = player->resume();
            sleep(4);
printf("ITCPlayer::handleMessage() Resume out !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
            break;
        case Stop:
            playerResult.u.iValue = player->stop();
            break;
        case Close:
            playerResult.u.iValue = player->close(UltraPlayer::BlackScreenMode);
            break;
        default:
            break;
    }

    systemManager().releaseMainPlayer(player);

    if (msg->what == Close) {
        systemManager().destoryMainPlayer();
    }

Exit:
    wakeUp();
}

static ITCPlayer *gITCPlayer = NULL;

ITCPlayer *
mainITCPlayer()
{
    return gITCPlayer;
}

} // namespace Hippo


extern "C" void 
mainITCPlayerCreate()
{
    Hippo::gITCPlayer = new Hippo::ITCPlayer();
}
