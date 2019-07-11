#include "DMRPlayer.h"
#include "DMRPlayerCTC.h"
#include "SystemManager.h"
#include "UltraPlayer.h"
#include "BrowserAgent.h"
#include "DMRManager.h"
#include "UltraPlayerAssertions.h"
#include "BrowserBridge/Huawei/BrowserEventQueue.h"
#include "DlnaAssertions.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "json/json_public.h"
#include "include/dlna/dlna_type.h"
#include "mid_stream.h"
#include "mid/mid_time.h"
#include "mid/mid_tools.h"
#include "ipanel_event.h"

namespace Hippo {
DMRPlayerCTC::DMRPlayerCTC(int dmrType, DMRManager* dmrManager)
    : DMRPlayer(dmrType, dmrManager)

{

}

DMRPlayerCTC::~DMRPlayerCTC()
{

}

struct json_object *
DMRPlayerCTC::GetResultDrm()
{
    std::string key, value;
    std::string flag;
    int productNum = 0;
    bool isEnd = false;

    std::map<std::string, std::string>::iterator it;

    if (m_Result)
        json_object_put(m_Result);

    m_Result = json_object_new_object();

    if (m_DmrType == DLNA_EVENT_DMR_GETPRODUCTINFO) {
        struct json_object *eventList = NULL;
        struct json_object *infoArray = NULL;
        infoArray = json_object_new_array();

        std::map<std::string, std::string>::iterator it;
        it = m_playInfo.find("returncode");
        if (it != m_playInfo.end()) {
            value = it->second;
            json_object_object_add(m_Result, "returncode", json_object_new_string(value.c_str()));
        }
        it = m_playInfo.find("errormsg");
        if (it != m_playInfo.end()) {
            value = it->second;
            json_object_object_add(m_Result, "errormsg", json_object_new_string(value.c_str()));
        }
        it = m_playInfo.find("totalcount");
        if (it != m_playInfo.end()) {
            value = it->second;
            json_object_object_add(m_Result, "totalcount", json_object_new_string(value.c_str()));
        }

        for (it = m_playInfo.begin(); it != m_playInfo.end(); ++it) {
            key = it->first;
            if ( !strncasecmp(key.c_str(), "returncode", 10)
                || !strncasecmp(key.c_str(), "errormsg", 8)
                || !strncasecmp(key.c_str(), "totalcount", 10)) {
                continue;
            }
            std::string temp = key.substr(0, 2);
            if (strncasecmp(flag.c_str(), temp.c_str(), 2)) {
                eventList = json_object_create_object();
                flag = key.substr(0, 2);
                isEnd = true;
            }
            key = key.substr(2, key.length() - 1);
            value = it->second;
            DLNA_LOG("key = %s, value = %s\n", key.c_str(),value.c_str());

            json_object_object_add(eventList, key.c_str(), json_object_new_string(value.c_str()));

            if (isEnd) {
                std::string productUrl = m_DmrManager->GetProductUrl();
                std::string::size_type s1;
                if ((s1 = productUrl.find("columnid")) != std::string::npos) {
                    productUrl = productUrl.substr(s1 + 9,  productUrl.length() - 1);
                    if ((s1 = productUrl.find("&")) != std::string::npos) {
                        productUrl = productUrl.substr(0, s1);
                    } else {
                        productUrl = productUrl.substr(0, productUrl.length() - 1);
                    }
                }
                json_object_object_add(eventList, "columnid", json_object_new_string(productUrl.c_str()));
                json_array_add_object(infoArray, eventList);
                isEnd = false;
            }
        }
        json_object_object_add(m_Result, "product_list", infoArray);

    } else {
        for (it = m_playInfo.begin(); it != m_playInfo.end(); ++it) {
            key = it->first;
            value = it->second;
            DLNA_LOG("key = %s, value = %s\n", key.c_str(),value.c_str());
            json_object_object_add(m_Result, key.c_str(), json_object_new_string(value.c_str()));
        }
    }
    m_playInfo.clear();

    return m_Result;
}

int
DMRPlayerCTC::parseString(std::string str)
{
    std::string key, tempKey;
    std::string value;
    std::string tempStr;
    std::string::size_type s1, s2, s0;
    int productNum = 10;
    char product[5] = {0};

    DLNA_LOG("str = [%s]\n", str.c_str());
    if ((s1 = str.find("?")) != std::string::npos)
        tempStr = str.substr(s1 + 1, str.length() - 1);
    else
        tempStr = str;
    while(1) {
        if ((s1 = tempStr.find_first_of("=")) != std::string::npos) {
            if (m_DmrType == DLNA_EVENT_DMR_GETPRODUCTINFO) {
                key.clear();
                tempKey = tempStr.substr(0, s1);
                if ( (s0 = tempKey.find("returncode")) != std::string::npos) {
                    key = tempKey.substr(s0, 10);
                } else {
                    if ( !strncasecmp(tempKey.c_str(), "returncode", 10)
                        || !strncasecmp(tempKey.c_str(), "errormsg", 8)
                        || !strncasecmp(tempKey.c_str(), "totalcount", 10)) {
                        key = tempKey;
                    } else {
                        if (!strncasecmp(tempKey.c_str(), "productid", 9)) {
                            productNum++;
                            sprintf(product, "%d", productNum);
                        }
                        key.append(product);
                        key.append(tempKey);
                    }
                }
            } else {
                key = tempStr.substr(0, s1);
            }
            s2 = tempStr.find_first_of('&');
            if (s2 != std::string::npos) {
                value = tempStr.substr(s1 + 1, s2 - s1 - 1);
                m_playInfo.insert(std::make_pair((key), value));
            } else {
                value = tempStr.substr(s1 + 1, tempStr.length() - 1);
                m_playInfo.insert(std::make_pair((key), value));
                break;
            }
            tempStr = tempStr.substr(s2 + 1, tempStr.length() - 1);
        } else {
            m_playInfo.clear();
            key = "returncode";
            value = "-911";
            m_playInfo.insert(std::make_pair((key), value));
            break;
        }
    }

    return 0;

}

int
DMRPlayerCTC::handleDmrPlayEvent()
{
    struct json_object *obj = NULL;
    struct json_object *eventJson = NULL;
    std::string instanceId;
    std::string playUrl;
    std::string speed;
    std::string action;
    std::string key, value;
    std::string currentPlayUrl;
    std::string mediaType;
    int currentPlayState;

    obj = json_object_get_object_bykey(m_Object,"instanceID");
    if (obj == NULL)
        return -1;

    instanceId = json_object_get_string(obj);
    obj = json_object_get_object_bykey(m_Object,"PlayUrl");
    if (obj == NULL)
        return -1;

    playUrl = json_object_get_string(obj);
    obj = json_object_get_object_bykey(m_Object,"speed");
    if (obj == NULL)
        return -1;

    speed = json_object_get_string(obj);
    obj = json_object_get_object_bykey(m_Object,"mediaType");
    if (obj == NULL)
        return -1;
    mediaType = json_object_get_string(obj);

    DLNA_LOG("cur url = %s\n cur state = %d\n, play url = %s\n", (m_DmrManager->GetPlayUrl()).c_str(), m_DmrManager->GetPlayState(), playUrl.c_str());
    if (speed == "1") {
        currentPlayState = m_DmrManager->GetPlayState();
        currentPlayUrl = m_DmrManager->GetPlayUrl();
        if ((currentPlayState == DMRManager::PauseState
            && currentPlayUrl == playUrl)
            ||(currentPlayState == DMRManager::TickModeState
            && currentPlayUrl == playUrl)) {
            sendMessageToEPGBrowser(MessageType_KeyDown, EIS_IRKEY_PLAY, 0, 0);
            m_DmrManager->SetPlayState(DMRManager::PlayState);
            m_DmrManager->SetPlayUrl(playUrl);
            value = "0";
            m_playInfo.insert(std::make_pair(("returncode"), value));
            m_isFinished = true;
            return 0;
        }
        if (currentPlayState == DMRManager::PlayState
            && currentPlayUrl == playUrl) {
            value = "0";
            m_playInfo.insert(std::make_pair(("returncode"), value));
            m_isFinished = true;
            return 0;
        }
    } else {
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
            m_DmrManager->SetPlayState(DMRManager::TickModeState);
            value = "0";
            m_playInfo.insert(std::make_pair(("returncode"), value));
            m_isFinished = true;
            return 0;
    }

    eventJson = json_object_new_object();
    m_DmrManager->SetPlayUrl(playUrl);
    if (!strncasecmp(mediaType.c_str(), "PICTURE", 7)) {
        json_object_object_add(eventJson, "type", json_object_new_string("EVENT_DLNA_PIC_SHOW"));
        browserEventSend(json_object_to_json_string(eventJson), NULL);
        json_object_put(eventJson);
        m_DmrManager->sendDmrMessage(DLNA_EVENT_DMR_PLAY + 0x100, 0);
        value = "0";
        m_playInfo.insert(std::make_pair(("returncode"), value));
        m_isFinished = true;
    } else {
        json_object_object_add(eventJson, "type", json_object_new_string("EVENT_DLNA_PUSH"));
        json_object_object_add(eventJson, "action", json_object_new_string(playUrl.c_str()));
        browserEventSend(json_object_to_json_string(eventJson), NULL);
        json_object_put(eventJson);
        m_DmrManager->SetPlayState(DMRManager::PlayState);
        m_DmrManager->sendDmrMessage(DLNA_EVENT_DMR_PLAY, 8000);
    }


    return 0;

}

int
DMRPlayerCTC::handleGetProductInfoEvent()
{
    struct json_object *obj = NULL;
    struct json_object *eventJson = NULL;
    std::string productInfo;

    obj = json_object_get_object_bykey(m_Object,"CurrentURI");
    if (obj == NULL)
        return -1;
    productInfo = json_object_get_string(obj);
    eventJson = json_object_new_object();

    json_object_object_add(eventJson, "type", json_object_new_string("EVENT_DLNA_GETPRODUCTINFO"));
    json_object_object_add(eventJson, "action", json_object_new_string(productInfo.c_str()));
    browserEventSend(json_object_to_json_string(eventJson), NULL);
    json_object_put(eventJson);
    m_DmrManager->SetProductUrl(productInfo);
    m_DmrManager->sendDmrMessage(DLNA_EVENT_DMR_GETPRODUCTINFO, 8000);

    return 0;

}

int
DMRPlayerCTC::handleOrderInfoEvent()
{
    struct json_object *obj = NULL;
    struct json_object *eventJson = NULL;
    std::string eventInfo;
    std::string autorenewal;
    int purchasetype = 0;

    obj = json_object_get_object_bykey(m_Object,"Action");
    if (obj == NULL)
        return -1;
    eventInfo.append("ordertype=");
    eventInfo.append(json_object_get_string(obj));
    obj = json_object_get_object_bykey(m_Object,"ProductId");
    if (obj == NULL)
        return -1;
    eventInfo.append("&productid=");
    eventInfo.append(json_object_get_string(obj));
    obj = json_object_get_object_bykey(m_Object,"ServiceId");
    if (obj == NULL)
        return -1;
    eventInfo.append("&serviceid=");
    eventInfo.append(json_object_get_string(obj));
    obj = json_object_get_object_bykey(m_Object,"ContentId");
    if (obj == NULL)
        return -1;
    eventInfo.append("&contentid=");
    eventInfo.append(json_object_get_string(obj));
    obj = json_object_get_object_bykey(m_Object,"ColumnId");
    if (obj == NULL)
        return -1;
    eventInfo.append("&columnid=");
    std::string productUrl = m_DmrManager->GetProductUrl();
    std::string::size_type s1;
    if ((s1 = productUrl.find("columnid")) != std::string::npos) {
        productUrl = productUrl.substr(s1 + 9,  productUrl.length() - 1);
        if ((s1 = productUrl.find("&")) != std::string::npos) {
            productUrl = productUrl.substr(0, s1);
        } else {
            productUrl = productUrl.substr(0, productUrl.length() - 1);
        }
    }
    eventInfo.append(json_object_get_string(obj));
    //eventInfo.append(productUrl);
    obj = json_object_get_object_bykey(m_Object,"PurchaseType");
    if (obj == NULL)
        return -1;
    eventInfo.append("&purchasetype=");
    eventInfo.append(json_object_get_string(obj));
    purchasetype = atoi(json_object_get_string(obj));
    obj = json_object_get_object_bykey(m_Object,"ContentType");
    if (obj == NULL)
        return -1;
    eventInfo.append("&contenttype=");
    eventInfo.append(json_object_get_string(obj));
    obj = json_object_get_object_bykey(m_Object,"AutoRenewal");
    if (obj == NULL)
        return -1;
    autorenewal = json_object_get_string(obj);
    eventInfo.append("&autorenewal=");
    if (autorenewal.empty() && purchasetype == 0)
        eventInfo.append("1");
    else if(autorenewal.empty())
	eventInfo.append("0");
     else
	 eventInfo.append(autorenewal);

    eventJson = json_object_new_object();

    json_object_object_add(eventJson, "type", json_object_new_string("EVENT_DLNA_ORDER"));
    json_object_object_add(eventJson, "action", json_object_new_string(eventInfo.c_str()));
    browserEventSend(json_object_to_json_string(eventJson), NULL);
    json_object_put(eventJson);

    m_DmrManager->SetOrderUrl(eventInfo);
    m_DmrManager->sendDmrMessage(DLNA_EVENT_DMR_ORDER, 8000);

    return 0;
}

int
DMRPlayerCTC::handleGetDmrMediaInfoEvent()
{
    std::string value;
    unsigned int totalTime = 0;
    char timeStr[12] = {0};
    int hh = 0;
    int mm = 0;
    int ss = 0;

    value = "0";
    m_playInfo.insert(std::make_pair(("NrTracks"), value));

    value = m_DmrManager->GetPlayUrl();

    m_playInfo.insert(std::make_pair(("CurrentURI"), value));

    GetUltraPlayerInfo(DLNA_GETDURATION, &totalTime);
    hh = totalTime / (60 * 60);
    mm = (totalTime - hh * 60 * 60) / 60;
    ss = totalTime  -  hh * 60 * 60 - mm * 60;
    sprintf(timeStr, "%2d:%2d:%2d", hh, mm, ss);
    value = timeStr;
    m_playInfo.insert(std::make_pair(("MediaDuration"), value));
    value.clear();
    m_playInfo.insert(std::make_pair(("CurrentURIMetaData"), value));
    m_playInfo.insert(std::make_pair(("NextURIMetaData"), value));
    m_playInfo.insert(std::make_pair(("PlayMedium"), value));
    m_playInfo.insert(std::make_pair(("RecordMedium"), value));
    value = "NOT_IMPLEMENTED";
    m_playInfo.insert(std::make_pair(("WriteStatus"), value));
    m_isFinished = true;

    return 0;
}

int
DMRPlayerCTC::handlePicShowEvent()
{
    struct json_object *eventJson = NULL;

    eventJson = json_object_new_object();
    json_object_object_add(eventJson, "type", json_object_new_string("EVENT_DLNA_PIC_SHOW"));
    browserEventSend(json_object_to_json_string(eventJson), NULL);
    json_object_put(eventJson);

    return 0;
}

int
DMRPlayerCTC::handlePicReleaseEvent()
{
    struct json_object *eventJson = NULL;

    eventJson = json_object_new_object();
    json_object_object_add(eventJson, "type", json_object_new_string("EVENT_DLNA_PIC_RELEASED"));
    browserEventSend(json_object_to_json_string(eventJson), NULL);
    json_object_put(eventJson);

    return 0;
}

int
DMRPlayerCTC::setVolume()
{
    struct json_object *obj = NULL;
    std::string volume;

    obj = json_object_get_object_bykey(m_Object,"Volume");
    if (obj == NULL)
        return -1;
    volume = json_object_get_string(obj);
    SystemManager &sysManager = systemManager();
    UltraPlayer *player = sysManager.obtainMainPlayer();
    if(!player)
        return -1;
    player->SetVolume(atoi(volume.c_str()));
    sysManager.releaseMainPlayer(player);
    return 0;
}

int
DMRPlayerCTC::seek()
{
    struct json_object *obj = NULL;
    std::string seekMode;
    std::string seekTime;
    std::string hh, mm, ss;
    int playTime;
    std::string::size_type s1, s2;

    obj = json_object_get_object_bykey(m_Object,"seek_mode");
    if (obj == NULL)
        return -1;
    seekMode = json_object_get_string(obj);
    obj = json_object_get_object_bykey(m_Object,"seek_target");
    if (obj == NULL)
        return -1;
    seekTime = json_object_get_string(obj);
    if (!strncasecmp(seekMode.c_str(), "REL_TIME", 8)) {
        if ((s1 = seekTime.find_first_of(':')) != std::string::npos) {
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
    } else {
        if ((s1 = seekTime.find_first_of(':')) != std::string::npos) {
            char currentTime[16] = {0};
            char ymd[8] = {0};
            std::string utcTime;
            hh = seekTime.substr(0, s1);
            seekTime = seekTime.substr(s1 + 1, seekTime.length() - 1);
            if ((s2 = seekTime.find_first_of(':')) != std::string::npos) {
                mm = seekTime.substr(0, s2);
                seekTime = seekTime.substr(s2 + 1, seekTime.length() - 1);
                ss = seekTime;
                mid_tool_time2string(mid_time(), currentTime, 0);
                strncpy(ymd, currentTime, 8);
                utcTime = ymd;
                utcTime += "T";
                utcTime = utcTime + hh + mm + ss;
                utcTime += "Z";
                DLNA_LOG("utcTime = %s\n", utcTime.c_str());
                playTime = mid_tool_string2time((char *)utcTime.c_str());
                playTime = playTime - 8 * 60 * 60;
            }
        }
    }
    DLNA_LOG("playTime = %d\n", playTime);
    SystemManager &sysManager = systemManager();
    UltraPlayer *player = sysManager.obtainMainPlayer();
    if(!player)
        return -1;
    player->seekTo(playTime);
    sysManager.releaseMainPlayer(player);
    return 0;
}


void
DMRPlayerCTC::handleDmrMessage()
{
    std::string ret = "0";
    std::string temp, hh, mm, ss;
    unsigned int value = 0;
    char strValue[16] = {0};

    DLNA_LOG("m_DmrType = %d\n", m_DmrType);
    switch (m_DmrType) {
    case DLNA_EVENT_DMR_PLAY:
        handleDmrPlayEvent();
        break;
    case DLNA_EVENT_DMR_PAUSE:
        sendMessageToEPGBrowser(MessageType_KeyDown, EIS_IRKEY_PLAY, 0, 0);
        m_playInfo.insert(std::make_pair(("returncode"), ret));
        m_isFinished = true;
        m_DmrManager->SetPlayState(DMRManager::PauseState);
        break;
    case DLNA_EVENT_DMR_RESUME:
        sendMessageToEPGBrowser(MessageType_KeyDown, EIS_IRKEY_PLAY, 0, 0);
        m_playInfo.insert(std::make_pair(("returncode"), ret));
        m_isFinished = true;
        break;
    case DLNA_EVENT_DMR_SEEK:
        seek();
        m_playInfo.insert(std::make_pair(("returncode"), ret));
        m_isFinished = true;
        break;
    case DLNA_EVENT_DMR_STOP:
        sendMessageToEPGBrowser(MessageType_KeyDown, EIS_IRKEY_BACK, 0, 0);
        m_DmrManager->SetPlayState(DMRManager::StopState);
        m_playInfo.insert(std::make_pair(("returncode"), ret));
        m_isFinished = true;
        break;
    case DLNA_EVENT_DMR_SETMUTE:
        sendMessageToEPGBrowser(MessageType_KeyDown, EIS_IRKEY_VOLUME_MUTE, 0, 0);
        m_playInfo.insert(std::make_pair(("returncode"), ret));
        m_isFinished = true;
        break;
    case DLNA_EVENT_DMR_SETVOLUME:
        setVolume();
        m_playInfo.insert(std::make_pair(("returncode"), ret));
        m_isFinished = true;
        break;
    case DLNA_EVENT_DMR_GETMUTE:
        GetUltraPlayerInfo(DLNA_GETMUTE, &value);
        m_playInfo.insert(std::make_pair(("returncode"), ret));
        sprintf(strValue, "%d", value);
        temp = strValue;
        m_playInfo.insert(std::make_pair(("Mute"), temp));
        m_isFinished = true;
        break;
    case DLNA_EVENT_DMR_GETVOLUME:
        GetUltraPlayerInfo(DLNA_GETVOLUME, &value);
        m_playInfo.insert(std::make_pair(("returncode"), ret));
        sprintf(strValue, "%d", value);
        temp = strValue;
        m_playInfo.insert(std::make_pair(("Volume"), temp));
        m_isFinished = true;
        break;
    case DLNA_EVENT_DMR_ORDER:
        handleOrderInfoEvent();
        break;
    case DLNA_EVENT_DMR_GETPOSITIONINFO:
        m_playInfo.insert(std::make_pair(("returncode"), ret));
        GetUltraPlayerInfo(DLNA_GETPOSITION, &value);
        if (value > 100000) {
            mid_tool_time2string(value, strValue, 0);
            temp = strValue;
            hh = temp.substr(8, 2);
            mm = temp.substr(10, 2);
            ss = temp.substr(12, 2);
            DLNA_LOG("strvalue = %s, hh = %s, mm = %s, ss = %s\n", strValue, hh.c_str(), mm.c_str(), ss.c_str());
            value = (atoi(hh.c_str()) + 8) * 3600 + atoi(mm.c_str()) * 60 + atoi(ss.c_str());
        }
        sprintf(strValue, "%d", value);
        temp = strValue;
        m_playInfo.insert(std::make_pair(("Position"), temp));
        GetUltraPlayerInfo(DLNA_GETDURATION, &value);
        sprintf(strValue, "%d", value);
        temp = strValue;
        m_playInfo.insert(std::make_pair(("Duration"), temp));
        m_isFinished = true;
        break;
    case DLNA_EVENT_DMR_GETMEDIAINFO:
        handleGetDmrMediaInfoEvent();
        break;
    case DLNA_EVENT_DMR_GETPRODUCTINFO:
        handleGetProductInfoEvent();
        break;
    case DLNA_EVENT_DMR_GETTRANSPORTINFO:
        GetUltraPlayerInfo(DLNA_PLAYSTATE, &value);
        switch (value) {
        case STRM_STATE_CLOSE:
            temp = "STOPPED";
            break;
        case STRM_STATE_PAUSE:
            temp = "PAUSED_PLAYBACK";
            break;
		case STRM_STATE_PLAY:
        case STRM_STATE_IPTV:
            temp = "PLAYING";
            break;
        default:
            temp = "TRANSITIONING";
            break;
        }
        m_playInfo.insert(std::make_pair(("CurrentTransportState"), temp));
        temp = "OK";
        m_playInfo.insert(std::make_pair(("CurrentTransportStatus"), temp));
        m_isFinished = true;
        break;
    case DLNA_EVENT_DMR_SETTRANSFORMS_ZOOM:
        break;
    default:
        return;
    }
}

}

