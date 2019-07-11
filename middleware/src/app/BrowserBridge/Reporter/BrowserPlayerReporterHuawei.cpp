
#include "BrowserPlayerReporterHuawei.h"
#include "BrowserAssertions.h"
#include "BrowserPlayerMsgCode.h"
#include "UltraPlayer.h"
#include "UltraPlayerClient.h"
#include "BrowserEventQueue.h"
#include "Tr069.h"

#include "json/json.h"
#include "json/json_object.h"
#include "json/json_public.h"
#ifdef TVMS_OPEN
#include "tvms.h"
#endif

namespace Hippo {
using namespace std;
BrowserPlayerReporterHuawei::BrowserPlayerReporterHuawei()
	: m_curState(0)
	, m_curRate(0)
	, m_PlayingFlag(0)
{
}

BrowserPlayerReporterHuawei::~BrowserPlayerReporterHuawei()
{
}


int
BrowserPlayerReporterHuawei::buildEvent(int eventType, int instanceID, int newState, int newSpeed,
        int oldState, int oldSpeed, int code, std::string errMsg, std::string mediaCode, std::string entryID)
{
    std::string event;
    json_object* jsonInfo = NULL;
    jsonInfo = json_object_create_object();
    if (!jsonInfo)
        return -1;
    switch (eventType) {
    case Event_MediaError:
        json_object_object_add(jsonInfo, "type", json_object_new_string("EVENT_MEDIA_ERROR"));
        json_object_object_add(jsonInfo, "instance_id", json_object_new_int(instanceID));
        json_object_object_add(jsonInfo, "error_code", json_object_new_int(code));
        json_object_object_add(jsonInfo, "error_message", json_object_new_string(errMsg.c_str()));
        json_object_object_add(jsonInfo, "media_code", json_object_new_string(mediaCode.c_str()));
        break;
    case Event_MediaBegin:
        json_object_object_add(jsonInfo, "type", json_object_new_string("EVENT_MEDIA_BEGINING"));
        json_object_object_add(jsonInfo, "instance_id", json_object_new_int(instanceID));
        json_object_object_add(jsonInfo, "media_code", json_object_new_string(mediaCode.c_str()));
        json_object_object_add(jsonInfo, "entry_id", json_object_new_string(entryID.c_str()));
        break;
    case Event_MediaEnd:
        json_object_object_add(jsonInfo, "type", json_object_new_string("EVENT_MEDIA_END"));
        json_object_object_add(jsonInfo, "instance_id", json_object_new_int(instanceID));
        json_object_object_add(jsonInfo, "media_code", json_object_new_string(mediaCode.c_str()));
        json_object_object_add(jsonInfo, "entry_id", json_object_new_string(entryID.c_str()));
        break;
    case Event_MediaADSBegin:
        json_object_object_add(jsonInfo, "type", json_object_new_string("EVENT_MEDIA_ADSBEGINING"));
        json_object_object_add(jsonInfo, "instance_id", json_object_new_int(instanceID));
        break;
    case Event_MediaADSEnd:
        json_object_object_add(jsonInfo, "type", json_object_new_string("EVENT_MEDIA_ADSEND"));
        json_object_object_add(jsonInfo, "instance_id", json_object_new_int(instanceID));
        break;
    case Event_MediaBuffer:
        json_object_object_add(jsonInfo, "type", json_object_new_string("EVENT_MEDIA_BUFFER"));
        json_object_object_add(jsonInfo, "event_id", json_object_new_int(code));
        json_object_object_add(jsonInfo, "media_code", json_object_new_string(mediaCode.c_str()));
        json_object_object_add(jsonInfo, "event_descript", json_object_new_string(""));
        break;
    case Event_PlayModeChange:
        json_object_object_add(jsonInfo, "type", json_object_new_string("EVENT_PLAYMODE_CHANGE"));
        json_object_object_add(jsonInfo, "instance_id", json_object_new_int(instanceID));
        json_object_object_add(jsonInfo, "new_play_mode", json_object_new_int(newState));
        json_object_object_add(jsonInfo, "new_play_rate", json_object_new_int(newSpeed));
        json_object_object_add(jsonInfo, "old_play_mode", json_object_new_int(oldState));
        json_object_object_add(jsonInfo, "old_play_rate", json_object_new_int(oldSpeed));
        break;
    case Event_PLTVModeChange:
        json_object_object_add(jsonInfo, "type", json_object_new_string("EVENT_PLTVMODE_CHANGE"));
        json_object_object_add(jsonInfo, "instance_id", json_object_new_int(instanceID));
        json_object_object_add(jsonInfo, "service_type", json_object_new_int(newState));
        json_object_object_add(jsonInfo, "channel_code", json_object_new_int(code));
        json_object_object_add(jsonInfo, "channel_num", json_object_new_int(code));
        break;
    case Event_GOTOChannel:
        json_object_object_add(jsonInfo, "type", json_object_new_string("EVENT_GO_CHANNEL"));
        json_object_object_add(jsonInfo, "instance_id", json_object_new_int(instanceID));
        json_object_object_add(jsonInfo, "channel_code", json_object_new_string(mediaCode.c_str()));
        json_object_object_add(jsonInfo, "channel_num", json_object_new_int(code));
        break;
    case Event_STBError:
        json_object_object_add(jsonInfo, "type", json_object_new_string("EVENT_STB_ERROR"));
        json_object_object_add(jsonInfo, "error_code", json_object_new_int(code));
        json_object_object_add(jsonInfo, "media_code", json_object_new_string(mediaCode.c_str()));
        break;
    case Event_STBRestore:
        json_object_object_add(jsonInfo, "type", json_object_new_string("EVENT_STB_RESTORE"));
        json_object_object_add(jsonInfo, "error_code", json_object_new_int(code));
        json_object_object_add(jsonInfo, "media_code", json_object_new_string(mediaCode.c_str()));
        break;
    default:
        BROWSER_LOG_ERROR("Error Event Type(%d).\n", eventType);
        json_object_delete(jsonInfo);
        return -1;
    }
    event = json_object_to_json_string(jsonInfo);
    json_object_delete(jsonInfo);

	BROWSER_LOG("EVETN (%s)\n", event.c_str());
	browserEventSend(event.c_str(), NULL);

	return 0;
}

int
BrowserPlayerReporterHuawei::getPlayerInstatncdId()
{
	return mPlayer->GetPlayerClient()->instanceId();
}

void
BrowserPlayerReporterHuawei::reportState(STRM_STATE state, int rate)
{
#ifdef TVMS_OPEN
    APP_TYPE atype = mid_stream_get_apptype(0);
    if (atype == APP_TYPE_VOD) {
        if (state == STRM_STATE_PLAY)
            vod_tvmsmsg_request();
        if (state == STRM_STATE_CLOSE)
            vod_tvmsmsg_clear_request();
    }
#endif
	return;
}

void
BrowserPlayerReporterHuawei::reportMessage(STRM_MSG message, int code)
{
    int eventType = -1;

    switch (message) {
    case STRM_MSG_STREAM_VIEW: {
        m_PlayingFlag = 1;
        TR069_DIAGNOSTICS_SET_STATE(DIAGNOSTATICS_STATE_COMPLETE);
#ifdef INCLUDE_TR069
        if(tr069_get_playurl_mode() == 1) {
            int eventID = TR069_API_SETVALUE("Event.Regist", "8 DIAGNOSTICS COMPLETE", 0);
            TR069_API_SETVALUE("Event.Post", "", eventID);
            tr069_set_playurl_mode(0);
        }
#endif
        break;
    }
    case STRM_MSG_RECV_RESUME:
        buildEvent(Event_PlayModeChange, getPlayerInstatncdId(), PLAYMODE_NORMAL_PLAY, 1, PLAYMODE_STOP, 0, 0, "", "", "");
        break;
    default:
        break;
    }

    return;

}

} // namespace Hippo
