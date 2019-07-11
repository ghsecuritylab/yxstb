#include "DMRPlayerHuawei.h"
#include "DMRPlayer.h"
#include "DMRManager.h"
#include "UltraPlayerAssertions.h"
#include "BrowserBridge/Huawei/BrowserEventQueue.h"
#include "DlnaAssertions.h"
#include "NativeHandler.h"
#include "BrowserAgent.h"
#include <string.h>
#include <stdlib.h>
#include "KeyDispatcher.h"

#include "mid_stream.h"
#include "json/json_public.h"
#include "include/dlna/dlna_type.h"
#include "ipanel_event.h"

namespace Hippo {
static int isplay = -1;
static int isepgfinish = -1;

DMRPlayerHuawei::DMRPlayerHuawei(int dmrType, DMRManager* dmrManager)
    : DMRPlayer(dmrType, dmrManager)
{
    printf("new DMRPlayerHuawei\n");
    isplay = -1;
    Local_flog = 1;
}

DMRPlayerHuawei::~DMRPlayerHuawei()
{

}

struct json_object *
DMRPlayerHuawei::GetResultDrm()
{
    return DMRPlayer::GetResultDrm();
}
void
setIsplay_huawei(int temp)
{
    isplay = temp;
}
void
setIsepgfinish_huawei(int temp)
{
    isepgfinish = temp;
}

int
DMRPlayerHuawei::Isplay()
{
    while(isplay == -1) {
        sleep(1);
    }
    if (isplay) {
	isplay = -1;
	return 1;
    }
    isplay = -1;
    return 0;

}
int
DMRPlayerHuawei::Isepgfinish()
{
    int i =5;
    while(isepgfinish == -1 && i-->0) {
        sleep(1);
    }
    if (1 == isepgfinish) {
	DLNA_LOG("isepgfinish = %d\n",isepgfinish);
	isepgfinish = -1;
	return 1;
    }
    isepgfinish = -1;
    return 0;

}

int
DMRPlayerHuawei::play()
{
    struct json_object *obj = NULL;
    struct json_object *eventJson = NULL;
    std::string instanceId;
    std::string playUrl;
    std::string currentPlayUrl;
    std::string ret;
    std::string strMediaType;
    std::string name;
    std::string speed;
    std::string mediaCode;
    std::string epgmediaType;
    std::string userID;
    std::string playByBookmark;
    std::string playByTime;
    std::string actionSource;
    std::string resolution;
    int mediaType = 0;

    obj = json_object_get_object_bykey(m_Object,"instanceID");
    if (obj == NULL)
        return -1;
    instanceId = json_object_get_string(obj);

    obj = json_object_get_object_bykey(m_Object,"PlayUrl");
    if (obj == NULL)
        return -1;
    playUrl = json_object_get_string(obj);

    obj = json_object_get_object_bykey(m_Object,"name");
    if ( obj )
        name = json_object_get_string(obj);

    obj = json_object_get_object_bykey(m_Object,"resolution");
    if ( obj )
        resolution = json_object_get_string(obj);

    obj = json_object_get_object_bykey(m_Object,"speed");
    if (obj){
        speed = json_object_get_string(obj);
	    if (speed == "1"  || speed == "NULL") {
		    obj = json_object_get_object_bykey(m_Object,"mediaType");
		    if (obj == NULL) {
		        ret = "-1";
		        m_playInfo.insert(std::make_pair(("returncode"), ret));
		        SetFinishStatus(true);
		        return 0;
		    } else {
		        strMediaType = json_object_get_string(obj);
		        if (!strncasecmp(strMediaType.c_str(), "M_VIDEO", 7)) {
		            mediaType = 1;
		        } else if (!strncasecmp(strMediaType.c_str(), "M_AUDIO", 7)) {
		            mediaType = 2;
		        } else if (!strncasecmp(strMediaType.c_str(), "PICTURE", 7)) {
		            mediaType = 3;
		        } else {
		            ret = "-1";
		            m_playInfo.insert(std::make_pair(("returncode"), ret));
		            SetFinishStatus(true);
		            return 0;
		        }
		    }
		    if (mediaType != 3) {
		        currentPlayUrl = m_DmrManager->GetPlayUrl();
		        unsigned int state = 0;
		        GetUltraPlayerInfo(DLNA_PLAYSTATE, &state);
		        if (currentPlayUrl == playUrl
		            && (state == STRM_STATE_PAUSE || state == STRM_STATE_FAST)) {
		            resume();
		            return 0;
		        }
		    }
		    /*sendMessageToNativeHandler(MessageType_DLNA, 0, 0, 0);
		    if( !Isplay() ) {
		        ret = "0";
		        m_playInfo.insert(std::make_pair(("returncode"), ret));
		        SetFinishStatus(true);
		        return 0;
		    }*/

		    //tudo jiance playUrl
		    //Local_flog = 1;

		    if (strstr(playUrl.c_str(),"platform=") && strstr(playUrl.c_str(),"ContentID=")) {
		        Local_flog = 0;
		        char temp[1024] = {0};
		        strcpy(temp,playUrl.c_str());
		        char *p = temp;
		        p = strstr(temp,"mediaCode=");
		        if (p) {
				 p += strlen("mediaCode=");
				 for(; *p != '&' && p;p++)
			        {
			            mediaCode += *p;
			        }
		        }
		        p = strstr(temp,"mediaType=");
		        if (p) {
				 p += strlen("mediaType=");
				 for(; *p != '&' && p;p++)
			        {
			            epgmediaType += *p;
			        }
		        }
		        p = strstr(temp,"userID=");
		        if (p) {
				 p += strlen("userID=");
				 for(; *p != '&' && p;p++)
			        {
			            userID += *p;
			        }
		        }
		        p = strstr(temp,"playByBookmark=");
		        if (p) {
				 p += strlen("playByBookmark=");
				 for(; *p != '&' && p;p++)
			        {
			            playByBookmark += *p;
			        }
		        }
		        p = strstr(temp,"playByTime=");
		        if (p) {
				 p += strlen("playByTime=");
				 for(; *p != '&' && p;p++)
			        {
			            playByTime += *p;
			        }
		        }
		        p = strstr(temp,"actionSource=");
		        if (p) {
				 p += strlen("actionSource=");
				 for(; *p != '&' && p;p++)
			        {
			            actionSource += *p;
			        }
		        }

		    }
		    else {
		        Local_flog = 1;
		    }
		    //epgBrowserAgent().openUrl("FILE:////home/hybroad/share/webpage/LocalPlayer/menu.html");
		    eventJson = json_object_new_object();
		    if(1 == Local_flog)
		    {
			    json_object_object_add(eventJson, "type", json_object_new_string("EVENT_DLNA_DMR_SETURI"));
			    json_object_object_add(eventJson, "instanceID", json_object_new_string(instanceId.c_str()));
			    json_object_object_add(eventJson, "filePath", json_object_new_string(playUrl.c_str()));
			    json_object_object_add(eventJson, "name", json_object_new_string(name.c_str()));
			    json_object_object_add(eventJson, "mediaType", json_object_new_int(mediaType));
			    json_object_object_add(eventJson, "bookmarkPosition", json_object_new_int(0));
			    json_object_object_add(eventJson, "resolution", json_object_new_string(resolution.c_str()));

		    }
		    else {
			//playUrl
		    sendMessageToKeyDispatcher(MessageType_KeyDown, EIS_IRKEY_MENU, 0, 0);
		    //sleep(3);
		    sleep(1);
		    if( !Isepgfinish() ) {
		        /*ret = "0";
		        m_playInfo.insert(std::make_pair(("returncode"), ret));
		        SetFinishStatus(true);
		        return 0;     */
		    }
		    sleep(1);
		    if (mediaType != 3) {

			    //if(if (speed == "1" ) {
			        json_object_add_object(eventJson, "type", json_object_new_string("EVENT_REMOTE_CONTROL"));
			        json_object_add_object(eventJson, "action", json_object_new_string("functionCall"));
			        json_object_add_object(eventJson, "functionType", json_object_new_string("startPlay"));
			        json_object_add_object(eventJson, "playUrl",  json_object_new_string(playUrl.c_str()));
			        //json_object_add_object(eventJson, "speed",  json_object_new_string(speed.c_str()));
			        json_object_add_object(eventJson, "mediaCode", json_object_new_string(mediaCode.c_str()));
			        json_object_add_object(eventJson, "mediaType", json_object_new_string(epgmediaType.c_str()));
			        json_object_add_object(eventJson, "userID", json_object_new_string(userID.c_str()));
			        json_object_add_object(eventJson, "playByBookmark",  json_object_new_string(playByBookmark.c_str()));
			        json_object_add_object(eventJson, "playByTime",  json_object_new_string(playByTime.c_str()));
			        json_object_add_object(eventJson, "actionSource",  json_object_new_string(actionSource.c_str()));
			   /* }else {

			        json_object_add_object(eventJson, "type", json_object_new_string("EVENT_REMOTE_CONTROL"));
			        json_object_add_object(eventJson, "action", json_object_new_string("trickPlayControl"));
			        json_object_add_object(eventJson, "functionType", json_object_new_string("startPlay"));
			        json_object_add_object(eventJson, "fastSpeed", json_object_new_string(speed.c_str()));
			    }*/
		    }
		    else {
			    json_object_add_object(eventJson, "type", json_object_new_string("EVENT_REMOTE_CONTROL"));
			    json_object_add_object(eventJson, "action", json_object_new_string("functionCall"));
			    json_object_add_object(eventJson, "functionType", json_object_new_string("pictureShow"));
			    json_object_add_object(eventJson, "pictureURL",  json_object_new_string(playUrl.c_str()));
		    }
    		}
		    m_DmrManager->SetPlayUrl(playUrl);
		    browserEventSend(json_object_to_json_string(eventJson), NULL);
		    json_object_put(eventJson);
		    ret = "0";
		    m_playInfo.insert(std::make_pair(("returncode"), ret));
		    SetFinishStatus(true);

		    return 0;
	    }
           else {
	            if(1 == Local_flog) {
		            int count = atoi(speed.c_str());
		            if ( count > 1) {
		                DLNA_LOG("press count fast = %d\n", count);
		                for (; 1 < count; ) {
		                    sendMessageToEPGBrowser(MessageType_KeyDown, EIS_IRKEY_FASTFORWARD, 0, 0);
					        count /= 2;
		                }
		            } else {
		            	count = abs(count);
		                for (; 1 < count;) {
		                    DLNA_LOG("press count rewind= %d\n", count);
		                    sendMessageToEPGBrowser(MessageType_KeyDown, EIS_IRKEY_REWIND, 0, 0);
					        count /= 2;
		                }
		            }
	            }
	            /*else {
	                 	json_object_add_object(eventJson, "type", json_object_new_string("EVENT_REMOTE_CONTROL"));
			        json_object_add_object(eventJson, "action", json_object_new_string("trickPlayControl"));
			        json_object_add_object(eventJson, "functionType", json_object_new_string("startPlay"));
			        json_object_add_object(eventJson, "fastSpeed", json_object_new_string(speed.c_str()));
	            }*/
	            m_DmrManager->SetPlayState(DMRManager::TickModeState);
	            ret = "0";
	            m_playInfo.insert(std::make_pair(("returncode"), ret));
	            m_isFinished = true;
	            return 0;
		}
    }
	obj = json_object_get_object_bykey(m_Object,"mediaType");
	if (obj == NULL) {
	ret = "-1";
	m_playInfo.insert(std::make_pair(("returncode"), ret));
	SetFinishStatus(true);
	return 0;
	} else {
		strMediaType = json_object_get_string(obj);
		if (!strncasecmp(strMediaType.c_str(), "M_VIDEO", 7)) {
			mediaType = 1;
		} else if (!strncasecmp(strMediaType.c_str(), "M_AUDIO", 7)) {
			mediaType = 2;
		} else if (!strncasecmp(strMediaType.c_str(), "PICTURE", 7)) {
			mediaType = 3;
		} else {
			ret = "-1";
			m_playInfo.insert(std::make_pair(("returncode"), ret));
			SetFinishStatus(true);
			return 0;
		}
	}
	if (mediaType != 3) {
		currentPlayUrl = m_DmrManager->GetPlayUrl();
		unsigned int state = 0;
		GetUltraPlayerInfo(DLNA_PLAYSTATE, &state);
		if (currentPlayUrl == playUrl
		&& (state == STRM_STATE_PAUSE || state == STRM_STATE_FAST)) {
		resume();
		return 0;
		}
	}
	/*sendMessageToNativeHandler(MessageType_DLNA, 0, 0, 0);
       if( !Isplay() ) {
	        ret = "0";
	        m_playInfo.insert(std::make_pair(("returncode"), ret));
	        SetFinishStatus(true);
	        return 0;
	    }*/

		    //tudo jiance playUrl
		    //Local_flog = 1;

		    if (strstr(playUrl.c_str(),"platform=") && strstr(playUrl.c_str(),"ContentID=")) {
		        Local_flog = 0;
		        char temp[1024] = {0};
		        strcpy(temp,playUrl.c_str());
		        char *p = temp;

		        p = strstr(temp,"mediaCode=");
		        if (p) {
				 p += strlen("mediaCode=");
				 for(; *p != '&' && p;p++)
			        {
			            mediaCode += *p;
			        }
		        }
		        p = strstr(temp,"mediaType=");
		        if (p) {
				 p += strlen("mediaType=");
				 for(; *p != '&' && p;p++)
			        {
			            epgmediaType += *p;
			        }
		        }
		        p = strstr(temp,"userID=");
		        if (p) {
				 p += strlen("userID=");
				 for(; *p != '&' && p;p++)
			        {
			            userID += *p;
			        }
		        }
		        p = strstr(temp,"playByBookmark=");
		        if (p) {
				 p += strlen("playByBookmark=");
				 for(; *p != '&' && p;p++)
			        {
			            playByBookmark += *p;
			        }
		        }
		        p = strstr(temp,"playByTime=");
		        if (p) {
				 p += strlen("playByTime=");
				 for(; *p != '&' && p;p++)
			        {
			            playByTime += *p;
			        }
		        }
		        p = strstr(temp,"actionSource=");
		        if (p) {
				 p += strlen("actionSource=");
				 for(; *p != '&' && p;p++)
			        {
			            actionSource += *p;
			        }
		        }

		    }
		    else {
		        Local_flog = 1;
		    }
	    //epgBrowserAgent().openUrl("FILE:////home/hybroad/share/webpage/LocalPlayer/menu.html");TAKIN_porting_load_notification
	eventJson = json_object_new_object();
	if(1 == Local_flog)
	{
		json_object_object_add(eventJson, "type", json_object_new_string("EVENT_DLNA_DMR_SETURI"));
		json_object_object_add(eventJson, "instanceID", json_object_new_string(instanceId.c_str()));
		json_object_object_add(eventJson, "filePath", json_object_new_string(playUrl.c_str()));
		json_object_object_add(eventJson, "name", json_object_new_string(name.c_str()));
		json_object_object_add(eventJson, "mediaType", json_object_new_int(mediaType));
		json_object_object_add(eventJson, "bookmarkPosition", json_object_new_int(0));
              json_object_object_add(eventJson, "resolution", json_object_new_string(resolution.c_str()));

	}
	else {
		//epg
		sendMessageToKeyDispatcher(MessageType_KeyDown, EIS_IRKEY_MENU, 0, 0);
		//sleep(3);
		sleep(1);
		if( !Isepgfinish() ) {
		        /*ret = "0";
		        m_playInfo.insert(std::make_pair(("returncode"), ret));
		        SetFinishStatus(true);
		        return 0;        */
		}
		sleep(1);
		if (mediaType != 3) {

			    //if(if (speed == "1" ) {
			        json_object_add_object(eventJson, "type", json_object_new_string("EVENT_REMOTE_CONTROL"));
			        json_object_add_object(eventJson, "action", json_object_new_string("functionCall"));
			        json_object_add_object(eventJson, "functionType", json_object_new_string("startPlay"));
			        json_object_add_object(eventJson, "playUrl",  json_object_new_string(playUrl.c_str()));
			        //json_object_add_object(eventJson, "speed",  json_object_new_string(speed.c_str()));
			        json_object_add_object(eventJson, "mediaCode", json_object_new_string(mediaCode.c_str()));
			        json_object_add_object(eventJson, "mediaType", json_object_new_string(epgmediaType.c_str()));
			        json_object_add_object(eventJson, "userID", json_object_new_string(userID.c_str()));
			        json_object_add_object(eventJson, "playByBookmark",  json_object_new_string(playByBookmark.c_str()));
			        json_object_add_object(eventJson, "playByTime",  json_object_new_string(playByTime.c_str()));
			        json_object_add_object(eventJson, "actionSource",  json_object_new_string(actionSource.c_str()));
			   /* }else {

			        json_object_add_object(eventJson, "type", json_object_new_string("EVENT_REMOTE_CONTROL"));
			        json_object_add_object(eventJson, "action", json_object_new_string("trickPlayControl"));
			        json_object_add_object(eventJson, "functionType", json_object_new_string("startPlay"));
			        json_object_add_object(eventJson, "fastSpeed", json_object_new_string(speed.c_str()));
			    }*/
		    }
		    else {
			    json_object_add_object(eventJson, "type", json_object_new_string("EVENT_REMOTE_CONTROL"));
			    json_object_add_object(eventJson, "action", json_object_new_string("functionCall"));
			    json_object_add_object(eventJson, "functionType", json_object_new_string("pictureShow"));
			    json_object_add_object(eventJson, "pictureURL",  json_object_new_string(playUrl.c_str()));
		    }
	}
	m_DmrManager->SetPlayUrl(playUrl);
	browserEventSend(json_object_to_json_string(eventJson), NULL);
	json_object_put(eventJson);
	ret = "0";
	m_playInfo.insert(std::make_pair(("returncode"), ret));
	SetFinishStatus(true);

	return 0;
}

int
DMRPlayerHuawei::stop()
{
    struct json_object *obj = NULL;
    struct json_object *eventJson = NULL;
    std::string instanceId;
    std::string ret;

    obj = json_object_get_object_bykey(m_Object,"instanceID");
    if (obj == NULL)
        return -1;
    instanceId = json_object_get_string(obj);

    eventJson = json_object_new_object();
    json_object_object_add(eventJson, "type", json_object_new_string("EVENT_DLNA_DMR_DESTORYPLAYER"));
    json_object_object_add(eventJson, "instanceID", json_object_new_string(instanceId.c_str()));

    browserEventSend(json_object_to_json_string(eventJson), NULL);
    json_object_put(eventJson);
    ret = "0";
    m_playInfo.insert(std::make_pair(("returncode"), ret));
    SetFinishStatus(true);

    return 0;
}


int
DMRPlayerHuawei::pause()
{
    struct json_object *obj = NULL;
    struct json_object *eventJson = NULL;
    std::string instanceId;
    std::string ret;

    obj = json_object_get_object_bykey(m_Object,"instanceID");
    if (obj == NULL)
        return -1;
    instanceId = json_object_get_string(obj);

    eventJson = json_object_new_object();
    json_object_object_add(eventJson, "type", json_object_new_string("EVENT_DLNA_DMR_TRICKPLAY"));
    json_object_object_add(eventJson, "instanceID", json_object_new_string(instanceId.c_str()));
    json_object_object_add(eventJson, "action", json_object_new_string("pause"));
    //printf("json_object_to_json_string(eventJson) =%s \n",json_object_to_json_string(eventJson));
    browserEventSend(json_object_to_json_string(eventJson), NULL);
    json_object_put(eventJson);
    ret = "0";
    m_playInfo.insert(std::make_pair(("returncode"), ret));
    SetFinishStatus(true);

    return 0;
}

int
DMRPlayerHuawei::resume()
{
    struct json_object *obj = NULL;
    struct json_object *eventJson = NULL;
    std::string instanceId;
    std::string ret;

    obj = json_object_get_object_bykey(m_Object,"instanceID");
    if (obj == NULL)
        return -1;
    instanceId = json_object_get_string(obj);

    eventJson = json_object_new_object();
    json_object_object_add(eventJson, "type", json_object_new_string("EVENT_DLNA_DMR_TRICKPLAY"));
    json_object_object_add(eventJson, "instanceID", json_object_new_string(instanceId.c_str()));
    json_object_object_add(eventJson, "action", json_object_new_string("play"));

    browserEventSend(json_object_to_json_string(eventJson), NULL);
    json_object_put(eventJson);
    ret = "0";
    m_playInfo.insert(std::make_pair(("returncode"), ret));
    SetFinishStatus(true);

    return 0;
}

int
DMRPlayerHuawei::seek()
{
    struct json_object *obj = NULL;
    struct json_object *eventJson = NULL;
    std::string instanceId;
    std::string seekMode;
    std::string seekTime;
    std::string ret;
    int playTime = 0;

    obj = json_object_get_object_bykey(m_Object,"instanceID");
    if (obj == NULL)
        return -1;
    instanceId = json_object_get_string(obj);
    obj = json_object_get_object_bykey(m_Object,"seek_mode");
    if (obj == NULL)
        return -1;
    seekMode = json_object_get_string(obj);
    obj = json_object_get_object_bykey(m_Object,"seek_target");
    if (obj == NULL)
        return -1;
    seekTime = json_object_get_string(obj);
    if (!strncasecmp(seekMode.c_str(), "REL_TIME", 8)) {
        std::string::size_type s1, s2;
        if ((s1 = seekTime.find_first_of(':')) != std::string::npos) {
            std::string hh, mm, ss;
            hh = seekTime.substr(0, s1);
            DLNA_LOG("hh = %s\n", hh.c_str());
            seekTime = seekTime.substr(s1 + 1, seekTime.length() - 1);
            DLNA_LOG("seekTime = %s\n", seekTime.c_str());
            if ((s2 = seekTime.find_first_of(':')) != std::string::npos) {
                mm = seekTime.substr(0, s2);
                DLNA_LOG("mm = %s\n", mm.c_str());
                seekTime = seekTime.substr(s2 + 1, seekTime.length() - 1);
                DLNA_LOG("seekTime = %s\n", seekTime.c_str());
                ss = seekTime;
                DLNA_LOG("ss = %s\n", ss.c_str());
                playTime = atoi(hh.c_str()) * 60 * 60 +  atoi(mm.c_str()) * 60 + atoi(ss.c_str());
            }
        }
    }
    eventJson = json_object_new_object();
    json_object_object_add(eventJson, "type", json_object_new_string("EVENT_DLNA_DMR_TRICKPLAY"));
    json_object_object_add(eventJson, "instanceID", json_object_new_string(instanceId.c_str()));
    json_object_object_add(eventJson, "action", json_object_new_string("seek"));
    json_object_object_add(eventJson, "SeekPosition", json_object_new_int(playTime));

    browserEventSend(json_object_to_json_string(eventJson), NULL);
    json_object_put(eventJson);
    ret = "0";
    m_playInfo.insert(std::make_pair(("returncode"), ret));
    SetFinishStatus(true);

    return 0;
}

int
DMRPlayerHuawei::setVolume()
{
    struct json_object *obj = NULL;
    struct json_object *eventJson = NULL;
    std::string instanceId;
    std::string volume;
    std::string ret;

    obj = json_object_get_object_bykey(m_Object,"InstanceID");
    if (obj == NULL)
        return -1;
    instanceId = json_object_get_string(obj);
    obj = json_object_get_object_bykey(m_Object,"Volume");
    if (obj == NULL)
        return -1;
    volume = json_object_get_string(obj);

    eventJson = json_object_new_object();
    json_object_object_add(eventJson, "type", json_object_new_string("EVENT_DLNA_DMR_VOLUME_SET"));
    json_object_object_add(eventJson, "instanceID", json_object_new_string(instanceId.c_str()));
    json_object_object_add(eventJson, "action", json_object_new_string("setVolume"));
    json_object_object_add(eventJson, "volume", json_object_new_string(volume.c_str()));

    browserEventSend(json_object_to_json_string(eventJson), NULL);
    json_object_put(eventJson);
    ret = "0";
    m_playInfo.insert(std::make_pair(("returncode"), ret));
    SetFinishStatus(true);

    return 0;
}

int
DMRPlayerHuawei::setMute()
{
    struct json_object *obj = NULL;
    struct json_object *eventJson = NULL;
    std::string instanceId;
    std::string mute;
    std::string ret;

    obj = json_object_get_object_bykey(m_Object,"InstanceID");
    if (obj == NULL)
        return -1;
    instanceId = json_object_get_string(obj);
    obj = json_object_get_object_bykey(m_Object,"Mute");
    if (obj == NULL)
        return -1;
    mute = json_object_get_string(obj);

    eventJson = json_object_new_object();
    json_object_object_add(eventJson, "type", json_object_new_string("EVENT_DLNA_DMR_VOLUME_SET"));
    json_object_object_add(eventJson, "instanceID", json_object_new_string(instanceId.c_str()));
    json_object_object_add(eventJson, "action", json_object_new_string("setMute"));
    json_object_object_add(eventJson, "volume", json_object_new_string(mute.c_str()));

    browserEventSend(json_object_to_json_string(eventJson), NULL);
    json_object_put(eventJson);
    ret = "0";
    m_playInfo.insert(std::make_pair(("returncode"), ret));
    SetFinishStatus(true);

    return 0;
}

int
DMRPlayerHuawei::getMute()
{
    unsigned int result = 0;
    std::string strResult;
    std::string ret = "0";
    char temp[16] = {0};

    GetUltraPlayerInfo(DLNA_GETMUTE, &result);
    m_playInfo.insert(std::make_pair(("returncode"), ret));
    sprintf(temp, "%d", result);
    strResult = temp;
    m_playInfo.insert(std::make_pair(("Mute"), strResult));
    m_isFinished = true;

    return 0;
}

int
DMRPlayerHuawei::getVolume()
{
    unsigned int result = 0;
    std::string strResult;
    std::string ret = "0";
    char temp[16] = {0};

    GetUltraPlayerInfo(DLNA_GETVOLUME, &result);
    m_playInfo.insert(std::make_pair(("returncode"), ret));
    sprintf(temp, "%d", result);
    strResult = temp;
    m_playInfo.insert(std::make_pair(("Volume"), strResult));
    m_isFinished = true;

    return 0;
}


void
DMRPlayerHuawei::handleDmrMessage()
{
    std::string ret = "0";
    std::string temp;
    unsigned int value = 0;
    char strValue[16] = {0};

    DLNA_LOG("m_DmrType = %d\n", m_DmrType);
    switch (m_DmrType) {
    case DLNA_EVENT_DMR_PLAY:
        play();
        break;
    case DLNA_EVENT_DMR_PAUSE:
        pause();
        break;
    case DLNA_EVENT_DMR_RESUME:
        resume();
        break;
    case DLNA_EVENT_DMR_SEEK:
        seek();
        break;
    case DLNA_EVENT_DMR_STOP:
        stop();
        break;
    case DLNA_EVENT_DMR_SETMUTE:
        setMute();
        break;
    case DLNA_EVENT_DMR_SETVOLUME:
        setVolume();
        break;
    case DLNA_EVENT_DMR_GETMUTE:
        getMute();
        break;
    case DLNA_EVENT_DMR_GETVOLUME:
        getVolume();
        break;
    case DLNA_EVENT_DMR_GETPOSITIONINFO:
        m_playInfo.insert(std::make_pair(("returncode"), ret));
        GetUltraPlayerInfo(DLNA_GETPOSITION, &value);
        sprintf(strValue, "%d", value);
        temp = strValue;
        m_playInfo.insert(std::make_pair(("Position"), temp));
        GetUltraPlayerInfo(DLNA_GETDURATION, &value);
        sprintf(strValue, "%d", value);
        temp = strValue;
        m_playInfo.insert(std::make_pair(("Duration"), temp));
        m_isFinished = true;
        break;
    default:
        ret = "0";
        m_playInfo.insert(std::make_pair(("returncode"), ret));
        SetFinishStatus(true);
        return;
    }
}

}

