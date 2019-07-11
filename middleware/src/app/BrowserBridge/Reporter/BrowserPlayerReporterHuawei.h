#ifndef _BrowserPlayerReporterHuawei_H_
#define _BrowserPlayerReporterHuawei_H_

#include "BrowserPlayerReporter.h"

#include <string>
#include "mid_stream.h"

#ifdef __cplusplus

namespace Hippo {

class BrowserPlayerReporterHuawei : public BrowserPlayerReporter {
public:
	BrowserPlayerReporterHuawei();
    ~BrowserPlayerReporterHuawei();

    enum{
        PLAYMODE_STOP = 0,
        PLAYMODE_PAUSE,
        PLAYMODE_NORMAL_PLAY,
        PLAYMODE_TRICK_MODE,
        PLAYMODE_MULTICAST_PLAY,
        PLAYMODE_UNICAST_PLAY,
        PLAYMODE_BUFFERING = 20
    };


    virtual void reportState(STRM_STATE state, int rate);
    virtual void reportMessage(STRM_MSG message, int code);
    int getPlayerInstatncdId();

protected:
	int	m_curState;
	int	m_curRate;
    int m_PlayingFlag;
    int g_end_begin;

	int buildEvent(int eventType, int instanceID, int newstate,
                    int newspeed, int oldstate, int oldspeed, int code, std::string errMsg, std::string mediaCode, std::string entryID);
};

} // namespace Hippo

#endif // __cplusplus

#endif // _BrowserPlayerReporterHuawei_H_
