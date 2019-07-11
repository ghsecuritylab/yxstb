#ifndef _ITCPlayer_H_
#define _ITCPlayer_H_

#include "ITC.h"
#include "UltraPlayerClient.h"

#ifdef __cplusplus

namespace Hippo {

/**
 * ITCPlayer will run in the created thread. 
 * and everyone can call the interface except handleMessage().
 */
class ITCPlayer : public ITC, public UltraPlayerClient {
public:
    static int mMaxPlayers;

    enum {
        Open,
        Play,
        Seek,
        FastForward,
        FastRewind,
        Pause,
        Resume,
        Stop,
        Close
    };

    ITCPlayer();
    ~ITCPlayer();

    int open(char *streamUrl, int pType);
    int play(unsigned int startTime);
    int seekTo(unsigned int playTime);
    int fastForward(int speed);
    int fastRewind(int speed);
    int pause();
    int resume();
    int stop();
    int close();

    virtual Type type() { return Monitor; }
    virtual int instanceId() { return 0; }

    virtual void onDestroy();

protected:
    virtual void handleMessage(Message *msg);
};

ITCPlayer *mainITCPlayer();

} // namespace Hippo

#endif // __cplusplus

#ifdef __cplusplus
extern "C" {
#endif

void mainITCPlayerCreate();

#ifdef __cplusplus
}
#endif

#endif // _ITCPlayer_H_
