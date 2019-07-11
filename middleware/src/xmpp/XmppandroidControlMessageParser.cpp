
#include "XmppControlMessageParser.h"
#include "XmppandroidControlMessageParser.h"
#include "XmppAssertions.h"
#include "XmppService.h"

#include "message.h"
#include "BrowserEventQueue.h"
#include "AppSetting.h"
#include "MessageTypes.h"
#include "KeyDispatcher.h"
#include "NativeHandlerCustomer.h"
#include "SystemManager.h"
#include "UltraPlayer.h"
#include "json/json_object.h"
#include "json/json_public.h"
#include <unistd.h>
#include "customer.h"
#include "AppSetting.h"

#include "codec.h"
#include "mid_fpanel.h"
#include "ipanel_event.h"
#include "mid_stream.h"
#include "app_epg_para.h"

#include <string.h>
#include <string>

using std::string;
using namespace Hippo;

namespace gloox {
static XmppAndroidControlMessageParser * g_Xmpp = NULL;

XmppAndroidControlMessageParser::XmppAndroidControlMessageParser()
{
}
XmppAndroidControlMessageParser::~XmppAndroidControlMessageParser()
{
}

void
XmppAndroidControlMessageParser::startPlayEncapsulateAndroid(struct json_object *obj)
{
    const char *mediaCode      = json_object_get_string(json_object_get_object_bykey(obj, "mediaCode"));
    const char *mediaType      = json_object_get_string(json_object_get_object_bykey(obj, "mediaType"));
    const char *playByBookmark = json_object_get_string(json_object_get_object_bykey(obj, "playByBookmark"));
    const char *playByTime     = json_object_get_string(json_object_get_object_bykey(obj, "playByTime"));
    const char *playUrl        = json_object_get_string(json_object_get_object_bykey(obj, "playUrl"));
    const char *actionSource   = json_object_get_string(json_object_get_object_bykey(obj, "actionSource"));

    bool isPlayByEvent = true;//((XmppService::XMPP_PLAY_BY_EVENT == m_xmppService->m_xmppPlayMode)? true: false);
    struct json_object *contentJson = NULL;
    string redirectionUrl;

    if (isPlayByEvent) {
        contentJson = json_object_create_object();
        if (!contentJson) {
            xmppLogError("Create object error.\n");
            return;
        }
        json_object_add_object(contentJson, "type", json_object_new_string("EVENT_REMOTE_CONTROL"));
        json_object_add_object(contentJson, "action", json_object_new_string("functionCall"));
        json_object_add_object(contentJson, "functionType", json_object_new_string("startPlay"));
    } else {
        std::string url = Hippo::Customer().AuthInfo().AvailableEpgUrlWithoutPath();
        if (!url.empty()) {
            if (url.substr(url.length() - 1, 1).compare(std::string("/")) == 0)
                url.erase(url.length() -1, 1);
            redirectionUrl.assign(url);
            redirectionUrl.append("/EPG/jsp/playcontentbyott.jsp?action=functionCall&functionType=startPlay");
        } else {
            xmppLogError("get EPG url error.\n");
            return;
        }
    }
    if (mediaCode) {
        if (isPlayByEvent)
            json_object_add_object(contentJson, "mediaCode", json_object_new_string(mediaCode));
        else {
            redirectionUrl.append("&mediaCode=");
            redirectionUrl.append(mediaCode);
        }
    }
    if (mediaType) {
        if (isPlayByEvent)
            json_object_add_object(contentJson, "mediaType", json_object_new_string(mediaType));
        else {
            redirectionUrl.append("&mediaType=");
            redirectionUrl.append(mediaType);
        }
    }
    char userID[34] = {0};
    appSettingGetString("ntvuser", userID, 32, 0); // This is optional.
    if (isPlayByEvent)
        json_object_add_object(contentJson, "userID", json_object_new_string(userID));
    else {
        redirectionUrl.append("&ntvuser=");
        redirectionUrl.append(userID);
    }
    if (playByBookmark) {
        if (isPlayByEvent) {
            json_object_add_object(contentJson, "playByBookmark", json_object_new_string(playByBookmark));
            json_object_add_object(contentJson, "playByTime", json_object_new_string(playByTime));
        } else {
            redirectionUrl.append("&playByBookmark=");
            redirectionUrl.append(playByBookmark);
            redirectionUrl.append("&playByTime=");
            redirectionUrl.append(playByTime);
        }
    }
    if (playUrl) {
        if (isPlayByEvent)
            json_object_add_object(contentJson, "playUrl", json_object_new_string(playUrl));
        else {
            redirectionUrl.append("&playUrl=");
            redirectionUrl.append(playUrl);
        }
    }
    if (actionSource) {
        if (isPlayByEvent)
            json_object_add_object(contentJson, "actionSource", json_object_new_string(actionSource));
        else {
            redirectionUrl.append("&actionSource=");
            redirectionUrl.append(actionSource);
        }
    }

    if (isPlayByEvent) {
        const char *contentString = json_object_to_json_string(contentJson);
        if (contentString && strlen(contentString) < 4096) {
            xmppLogDebug("%s\n", contentString);
            browserEventSend(contentString, 0);
        } else
            xmppLogError("Content string is Invalid([%d]%s).\n", strlen(contentString), contentString);
        json_object_put(contentJson);
    } else {
        xmppLogDebug("%s\n", redirectionUrl.c_str());
        //m_xmppService->getMessageBridge()->m_redirectionURL = redirectionUrl;
        //m_xmppService->getMessageBridge()->translateMessage(XmppMessageBridge::XMPP_MESSAGE_REDIRECT_URL, 0);
        //todo
        /* 这里不能直接使用epgBrowserAgentOpenUrl(redirectionUrl.c_str());
         * 因为此函数必须放在主线程被调用，否则会死机。
         */
    }
}

void
XmppAndroidControlMessageParser::trickPlayControlEncapsulateAndroid(struct json_object *obj)
{
    int seekPosition = 0;
    int fastSpeed = 0;
    const int trickPlayMode = json_object_get_int(json_object_get_object_bykey(obj, "trickPlayMode"));
    struct json_object *seekPositionObject = json_object_get_object_bykey(obj, "seekPosition");
    if (seekPositionObject)
        seekPosition = json_object_get_int(seekPositionObject);
    struct json_object *fastSpeedObject = json_object_get_object_bykey(obj, "fastSpeed");
    if (fastSpeedObject)
        fastSpeed = json_object_get_int(fastSpeedObject);

    bool isPlayByEvent = true; // 目前规格规定视频及图片的控制操作都采用0x300事件. bool isPlayByEvent = ((XmppService::XMPP_PLAY_BY_EVENT == m_xmppService->m_xmppPlayMode)? true: false);
    struct json_object *contentJson = NULL;
    string redirectionUrl;

    if (isPlayByEvent) {
        contentJson = json_object_create_object();
        if (!contentJson) {
            xmppLogError("Create object error.\n");
            return;
        }
        json_object_add_object(contentJson, "type", json_object_new_string("EVENT_REMOTE_CONTROL"));
        json_object_add_object(contentJson, "action", json_object_new_string("functionCall"));
        json_object_add_object(contentJson, "functionType", json_object_new_string("trickPlayControl"));
        json_object_add_object(contentJson, "trickPlayMode", json_object_new_int(trickPlayMode));
    } else {
        std::string url = Hippo::Customer().AuthInfo().AvailableEpgUrlWithoutPath();
        if (!url.empty()) {
            if (url.substr(url.length() - 1, 1).compare(std::string("/")) == 0)
                url.erase(url.length() -1, 1);
            redirectionUrl.assign(url);
            redirectionUrl.append("/EPG/jsp/playcontentbyott.jsp?action=functionCall&functionType=trickPlayControl");
        } else {
            xmppLogError("get EPG url error.\n");
            return;
        }
        char modeBuffer[32] = {0};
        snprintf(modeBuffer, sizeof(modeBuffer), "&trickPlayMode=%d", trickPlayMode);
        redirectionUrl.append(modeBuffer);
    }

    if (seekPosition) {
        if (isPlayByEvent)
            json_object_add_object(contentJson, "seekPosition", json_object_new_int(seekPosition));
        else {
            char seekPositionBuffer[32] = {0};
            snprintf(seekPositionBuffer, sizeof(seekPositionBuffer), "&seekPosition=%d", seekPosition);
            redirectionUrl.append(seekPositionBuffer);
        }
    }
    if (fastSpeed) {
        if (isPlayByEvent)
            json_object_add_object(contentJson, "fastSpeed", json_object_new_int(fastSpeed));
        else {
            char fastSpeedBuffer[32] = {0};
            snprintf(fastSpeedBuffer, sizeof(fastSpeedBuffer), "&fastSpeed=%d", fastSpeed);
            redirectionUrl.append(fastSpeedBuffer);
        }
    }

    if (isPlayByEvent) {
        const char *contentString = json_object_to_json_string(contentJson);
        if (contentString && strlen(contentString) < 4096) {
            xmppLogDebug("%s\n", contentString);
            browserEventSend(contentString, 0);
        } else
            xmppLogError("Content string is Invalid([%d]%s).\n", strlen(contentString), contentString);
            json_object_put(contentJson);
    } else {
        xmppLogDebug("%s\n", redirectionUrl.c_str());
        //m_xmppService->getMessageBridge()->m_redirectionURL = redirectionUrl;
        //m_xmppService->getMessageBridge()->translateMessage(XmppMessageBridge::XMPP_MESSAGE_REDIRECT_URL, 0);
        //todo
    }

}

void
XmppAndroidControlMessageParser::channelListEncapsulateAndroid()
{
   //implement it later ...
   return;
}


void
XmppAndroidControlMessageParser::pictureShowEncapsulateAndroid(struct json_object *obj)
{
    const char *showType = json_object_get_string(json_object_get_object_bykey(obj, "showType"));
    const char *pictureURL = json_object_get_string(json_object_get_object_bykey(obj, "pictureURL"));
    if (!showType) {
        xmppLogError("Invalid data.\n");
        return;
    }

    //bool isPlayByEvent = ((XmppService::XMPP_PLAY_BY_EVENT == m_xmppService->m_xmppPlayMode)? true: false);
    bool isPlayByEvent = true;
    if (!strncmp(showType, "showNew", sizeof("showNew")))
        isPlayByEvent = false;
    else
        isPlayByEvent = true;

    struct json_object *contentJson = NULL;
    string redirectionUrl;

    if (isPlayByEvent) {
        contentJson = json_object_create_object();
        if (!contentJson) {
            xmppLogError("Create object error.\n");
            return;
        }
        json_object_add_object(contentJson, "type", json_object_new_string("EVENT_REMOTE_CONTROL"));
        json_object_add_object(contentJson, "action", json_object_new_string("functionCall"));
        json_object_add_object(contentJson, "functionType", json_object_new_string("pictureShow"));
        json_object_add_object(contentJson, "showType", json_object_new_string(showType));
    } else {
        std::string url = Hippo::Customer().AuthInfo().AvailableEpgUrlWithoutPath();
        if (!url.empty()) {
            if (url.substr(url.length() - 1, 1).compare(std::string("/")) == 0)
                url.erase(url.length() -1, 1);
            redirectionUrl.assign(url);
            redirectionUrl.append("/EPG/jsp/playpicbyott.jsp?action=functionCall&functionType=pictureShow");
        } else {
            xmppLogError("get EPG url error.\n");
            return;
        }
        redirectionUrl.append("&showType=");
        redirectionUrl.append(showType);
    }

    if (pictureURL) {
        if (isPlayByEvent)
            json_object_add_object(contentJson, "pictureURL", json_object_new_string(pictureURL));
        else {
            redirectionUrl.append("&pictureURL=");
            redirectionUrl.append(pictureURL);
        }
    }

    if (isPlayByEvent) {
        const char *contentString = json_object_to_json_string(contentJson);
        if (contentString && strlen(contentString) < 4096) {
            xmppLogDebug("%s\n", contentString);
            browserEventSend(contentString, 0);
        } else
            xmppLogError("Content string is Invalid([%d]%s).\n", strlen(contentString), contentString);
        json_object_put(contentJson);
    } else {
        xmppLogDebug("%s\n", redirectionUrl.c_str());
        ///m_xmppService->getMessageBridge()->m_redirectionURL = redirectionUrl;
        //m_xmppService->getMessageBridge()->translateMessage(XmppMessageBridge::XMPP_MESSAGE_REDIRECT_URL, 0);
        //todo
        //epgBrowserAgentOpenUrl(redirectionUrl.c_str());
    }
}

void
XmppAndroidControlMessageParser::remoteControlHandlerAndroid(struct json_object *obj)
{
    const char *keyCode = json_object_get_string(json_object_get_object_bykey(obj, "keyCode"));
    if (keyCode) {
        int keyValue = atoi(keyCode + 2);
        //implement it later ... 当前还没有环境调试
        xmppLogDebug("KeyValue: 0x%x\n", keyValue);
        sendMessageToKeyDispatcher(MessageType_KeyDown, keyValue, 0, 0);
        return;
    }

    const char *STBControl = json_object_get_string(json_object_get_object_bykey(obj, "STBControl"));
    if (STBControl) {
        if (!strncmp(STBControl, "reboot", strlen("reboot"))) {
            mid_fpanel_reboot();
        } else if (!strncmp(STBControl, "standby", strlen("standby"))) {
            NativeHandlerInputMessage(MessageType_System, EIS_IRKEY_PAGE_STANDBY, 0, NULL);
        } else if (!strncmp(STBControl, "deep standby", strlen("deep standby"))) {
            settingManagerSave();
            mid_fpanel_poweroffdeep();
        }
        return;
    }
}

void
XmppAndroidControlMessageParser::setVolumeHandlerAndroid(struct json_object *obj)
{
    return;
}

void
XmppAndroidControlMessageParser::playerStateEncapsulateAndroid(char *result,int len)
{
    char jsonBuffer[1024] = {0};

    SystemManager &sysManager = systemManager();
    UltraPlayer *player = NULL;
    player = sysManager.obtainMainPlayer();
    if (!player || (STRM_STATE_CLOSE == player->mCurrentStatus)) {
        snprintf(jsonBuffer, sizeof(jsonBuffer), "{\"respondAction\":\"getAppState\",\"stateType\":\"playerState\",\"playBackState\":0}");
        //m_xmppService->getSession()->send(std::string(jsonBuffer), Message::Sysctrl);
        if ( 1024 > len) {
            strcpy(result,"strlen error");
        }
        else {
            strcpy(result,jsonBuffer);
        }

        if (player)
            sysManager.releaseMainPlayer(player);
        return;
    }

    XmppPlayerStateContent_t playerStateContent;
    memset(&playerStateContent, 0, sizeof(XmppPlayerStateContent_t));
    playerStateContent.trickPlayMode = -1;
    playerStateContent.playerInstance = player->mInstanceId;

    xmppLogDebug("Current play status: %d\n", player->mCurrentStatus);
    if (-1 == player->mCurrentStatus) {
        //sleep(1);
        usleep(1000000);
    }
    switch (player->mCurrentStatus) {
    case STRM_STATE_PLAY:
    case STRM_STATE_IPTV:
    case STRM_STATE_ADVERTISE:
        playerStateContent.trickPlayMode = XMPP_TRICK_MODE_PLAY;
        break;
    case STRM_STATE_PAUSE:
        playerStateContent.trickPlayMode = XMPP_TRICK_MODE_PAUSE;
        break;
    case STRM_STATE_FAST:
        playerStateContent.trickPlayMode = XMPP_TRICK_MODE_FAST;
        break;
    default:
        playerStateContent.trickPlayMode = XMPP_TRICK_MODE_PLAY;
    }
    playerStateContent.fastSpeed = player->mCurrentSpeed;
    playerStateContent.playPostion = player->getCurrentTime();

    xmppLogDebug("Current media type: %d\n", player->getMediaType());
    switch (player->getMediaType()) {
    case APP_TYPE_IPTV:
    case APP_TYPE_IPTV2:
        strncpy(playerStateContent.mediaType, "1", sizeof(playerStateContent.mediaType));
        break;
    case APP_TYPE_VOD:
    case APP_TYPE_VODADV:
    case APP_TYPE_APPLE_VOD:   //添加OTT播放类型
        strncpy(playerStateContent.mediaType, "2", sizeof(playerStateContent.mediaType));
        break;
    // TVOD wait ...
    case APP_TYPE_AUDIO:
    case APP_TYPE_HTTP_PCM:
    case APP_TYPE_HTTP_MP3:
        strncpy(playerStateContent.mediaType, "4", sizeof(playerStateContent.mediaType));
        break;
    case APP_TYPE_ZEBRA:
        strncpy(playerStateContent.mediaType, "5", sizeof(playerStateContent.mediaType));
        break;
    default:
        break;
    }

    playerStateContent.mediaCode = atoi(player->getMediaCode().c_str());
    playerStateContent.chanKey = player->GetCurrentChannelNum();
    playerStateContent.duration = player->getTotalTime();


    struct json_object *contentJson = json_object_create_object();
    if (!contentJson) {
        xmppLogError("Create object error.\n");
        sysManager.releaseMainPlayer(player);
        return;
    }
    json_object_add_object(contentJson, "respondAction", json_object_new_string("getAppState"));
    json_object_add_object(contentJson, "stateType", json_object_new_string("playerState"));
    json_object_add_object(contentJson, "playBackState", json_object_new_int(1));
    json_object_add_object(contentJson, "playerInstance", json_object_new_int(playerStateContent.playerInstance));
    json_object_add_object(contentJson, "trickPlayMode", json_object_new_int(playerStateContent.trickPlayMode));
    if (XMPP_TRICK_MODE_FAST == playerStateContent.trickPlayMode)
        json_object_add_object(contentJson, "fastSpeed", json_object_new_int(playerStateContent.fastSpeed));
    if (!strncmp(playerStateContent.mediaType, "2", 1))// && (XMPP_TRICK_MODE_PLAY == playerStateContent.trickPlayMode))
        json_object_add_object(contentJson, "playPosition", json_object_new_int(playerStateContent.playPostion));
    if (strlen(playerStateContent.mediaType))
        json_object_add_object(contentJson, "mediaType", json_object_new_string(playerStateContent.mediaType));
    json_object_add_object(contentJson, "mediaCode", json_object_new_int(playerStateContent.mediaCode));
    if (!strncmp(playerStateContent.mediaType, "1", 1))
        json_object_add_object(contentJson, "chanKey", json_object_new_int(playerStateContent.chanKey));
    if (!strncmp(playerStateContent.mediaType, "2", 1) || !strncmp(playerStateContent.mediaType, "5", 1))
        json_object_add_object(contentJson, "duration", json_object_new_int(playerStateContent.duration));

    const char *contentString = json_object_to_json_string(contentJson);
    if (contentString && strlen(contentString) < 4096) {
        xmppLogDebug("%s\n", contentString);
        //m_xmppService->getSession()->send(std::string(contentString), Message::Sysctrl);
        if ( strlen(contentString) >= (unsigned int)len) {
            strcpy(result,"strlen error");
        }
        else {
            strcpy(result,contentString);
        }
    } else
        xmppLogError("Content string is Invalid([%d]%s).\n", strlen(contentString), contentString);
    json_object_put(contentJson);


    sysManager.releaseMainPlayer(player);
}

void
XmppAndroidControlMessageParser::playerVolumeEncapsulateAndroid(char *result,int len)
{
    int volume = 0;
    char jsonBuffer[1024] = {0};
    appSettingGetInt("volume", &volume, 0);

    if (volume < 0) {
        volume = 0;
    } else if(volume > AUDIO_VOLUME_MAX) {
        volume = AUDIO_VOLUME_MAX;
    }
    xmppLogDebug("Get volume: %d\n", volume);

    snprintf(jsonBuffer, sizeof(jsonBuffer), "{\"respondAction\":\"getAppState\",\"stateType\":\"playerVolume\",\"currentVolume\":\"%d\"}", volume);
    xmppLogDebug("%s\n", jsonBuffer);
    if ( 1024 > len) {
        strcpy(result,"strlen error");
    }
    else {
        strcpy(result,jsonBuffer);
    }

    //m_xmppService->getSession()->send(std::string(jsonBuffer), Message::Sysctrl);
}

void
XmppAndroidControlMessageParser::parseActionOfFunctionCallAndroid(struct json_object *obj)
{
    const char *functionType = json_object_get_string(json_object_get_object_bykey(obj, "functionType"));
    if (!functionType) {
        xmppLogError("Get functionType error.\n");
        return;
    }
    if (!strncmp(functionType, "startPlay", strlen("startPlay"))) {
        startPlayEncapsulateAndroid(obj);
    } else if (!strncmp(functionType, "trickPlayControl", strlen("trickPlayControl"))) {
        trickPlayControlEncapsulateAndroid(obj);
    } else if (!strncmp(functionType, "toShowChannelList", strlen("toShowChannelList"))) {
        channelListEncapsulateAndroid();
    } else if (!strncmp(functionType, "pictureShow", strlen("pictureShow"))) {
        pictureShowEncapsulateAndroid(obj);
    } else if (!strncmp(functionType, "remoteControl", strlen("remoteControl"))) {
        remoteControlHandlerAndroid(obj);
    } else {
        xmppLogWarning("Undefined functionType.\n");
    }
}

void
XmppAndroidControlMessageParser::parseActionOfEventInfoAndroid(struct json_object *obj)
{
    const char *eventType = json_object_get_string(json_object_get_object_bykey(obj, "eventType"));
    if (!eventType) {
        xmppLogError("Get eventType error.\n");
        return;
    }

    XmppVolumeSetContent_t volumeSetContent;
    memset(&volumeSetContent, -1, sizeof(XmppVolumeSetContent_t));

    strncpy(volumeSetContent.eventType, eventType, sizeof(volumeSetContent.eventType));

    struct json_object *newVolume = json_object_get_object_bykey(obj, "newVolume");
    if (newVolume)
        volumeSetContent.newVolume = json_object_get_int(newVolume);
    struct json_object *oldVolume = json_object_get_object_bykey(obj, "oldVolume");
    if (oldVolume)
        volumeSetContent.oldVolume = json_object_get_int(oldVolume);

    struct json_object *contentJson = json_object_create_object();
    if (!contentJson) {
        xmppLogError("Create object error.\n");
        return;
    }
    json_object_add_object(contentJson, "type", json_object_new_string("EVENT_REMOTE_CONTROL"));
    json_object_add_object(contentJson, "action", json_object_new_string("eventInfo"));
    json_object_add_object(contentJson, "eventType", json_object_new_string(volumeSetContent.eventType));
    if (-1 != volumeSetContent.newVolume)
        json_object_add_object(contentJson, "newVolume", json_object_new_int(volumeSetContent.newVolume));
    if (-1 != volumeSetContent.oldVolume)
        json_object_add_object(contentJson, "oldVolume", json_object_new_int(volumeSetContent.oldVolume));

    const char *contentString = json_object_to_json_string(contentJson);
    if (contentString && strlen(contentString) < 4096) {
        xmppLogDebug("%s\n", contentString);
        browserEventSend(contentString, 0);
    } else
        xmppLogError("Content string is Invalid([%d]%s).\n", strlen(contentString), contentString);
    json_object_put(contentJson);

}

void
XmppAndroidControlMessageParser::parseActionOfGetAppStateAndroid(struct json_object *obj,char *result,int len)
{
    const char *stateType = json_object_get_string(json_object_get_object_bykey(obj, "stateType"));
    if (!stateType) {
        xmppLogError("Get stateType error.\n");
        return;
    }
    if (!strncmp(stateType, "playerState", strlen("playerState"))) {
        playerStateEncapsulateAndroid(result,len);
    } else if (!strncmp(stateType, "playerVolume", strlen("playerVolume"))) {
        playerVolumeEncapsulateAndroid(result,len);
    } else {
        xmppLogWarning("Undefined functionType.\n");
    }

}

#if 0 // Just for test.
void
XmppAndroidControlMessageParser::parseActionOftest(struct json_object *obj) //just for test.....
{
    const char *testType = json_object_get_string(json_object_get_object_bykey(obj, "testType"));
    if (!testType) {
        xmppLogError("Get testType error.\n");
        return;
    }
    if (!strncmp(testType, "addRoster", strlen("addRoster"))) {
        JID jid("test5@114.1.3.254");
        m_xmppService->addFriendToRosterByEPG(jid);
    } else if (!strncmp(testType, "delRoster", strlen("delRoster"))) {
        JID jid("test4@114.1.3.254");
        m_xmppService->removeFriendFromRosterByEPG(jid);
    } else if (!strncmp(testType, "getRosterAccout", strlen("getRosterAccout"))) {
        int count = 0;
        m_xmppService->getRosterItemCountByEPG(count);
    } else if (!strncmp(testType, "getRosterList", strlen("getRosterList"))) {
    } else {
        xmppLogWarning("Undefined functionType.\n");
    }
}
#endif

void
XmppAndroidControlMessageParser::startParseMessageAndroid(std::string&  str,char *result,int len)
{
    xmppLogDebug("%s\n", str.c_str());


    //if (gloox::Message::Sysctrl == message.subtype()) {
        struct json_object *obj = json_tokener_parse_string(str.c_str());
        if (!obj) {
            xmppLogError("Json tokener parse error.\n");
            return;
        }
        const char *action = json_object_get_string(json_object_get_object_bykey(obj, "action"));
        if (!action) {
            xmppLogError("Get string error!\n");
            json_object_put(obj);
            return;
        }
        if (!strncmp(action, "functionCall", strlen("functionCall"))) {
            parseActionOfFunctionCallAndroid(obj);
        } else if (!strncmp(action, "eventInfo", strlen("eventInfo"))) {
            parseActionOfEventInfoAndroid(obj);
        } else if (!strncmp(action, "getAppState", strlen("getAppState"))) {
            parseActionOfGetAppStateAndroid(obj,result,len);
#if 0 // Just for test.....
        } else if (!strncmp(action, "test", strlen("test"))) {
            parseActionOftest(obj);
#endif
       // } else {
       //     xmppLogWarning("Undefined action.\n");
       //     json_object_put(obj);
       //     return;
       // }
        json_object_put(obj);
    } else {
        ; // Other message type or....
    }
}
XmppAndroidControlMessageParser* XmppAndroidControlMessageParser::GetInstance(void)
{
    if (!g_Xmpp)
        g_Xmpp = new XmppAndroidControlMessageParser();
    return g_Xmpp;
}

}//namepace gloox


