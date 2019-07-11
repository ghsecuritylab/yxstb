#include "DMRPlayer.h"
#include "SystemManager.h"
#include "UltraPlayer.h"
#include "DlnaAssertions.h"

#include <stdio.h>

#include "json/json_public.h"

namespace Hippo {
DMRPlayer::DMRPlayer(int dmrType, DMRManager* dmrManager)
    : m_DmrType(dmrType)
    , m_DmrManager(dmrManager)
    , m_isFinished(false)
    , m_Object(0)
    , m_Result(0)
{

}

DMRPlayer::~DMRPlayer()
{
    if (m_Result)
        json_object_put(m_Result);
    if (m_Object)
        json_object_put(m_Object);

}

int
DMRPlayer::setDmrEventArg(char *eventArg)
{
    DLNA_LOG("setDmrEventArg\n");
    DLNA_LOG("eventArg = %s\n", eventArg);

    if (m_Object) {
        json_object_put(m_Object);
        m_Object = NULL;
    }
    m_Object = json_tokener_parse(eventArg);
    if (!m_Object)
        return -1;
    return 0;
}

struct json_object *
DMRPlayer::GetResultDrm()
{
    std::string key, value;
    std::map<std::string, std::string>::iterator it;

    if (m_Result)
        json_object_put(m_Result);

    m_Result = json_object_new_object();
    for (it = m_playInfo.begin(); it != m_playInfo.end(); ++it) {
        key = it->first;
        value = it->second;
        DLNA_LOG("key = %s, value = %s\n", key.c_str(),value.c_str());
        json_object_object_add(m_Result, key.c_str(), json_object_new_string(value.c_str()));
    }
    m_playInfo.clear();

    return m_Result;
}

int
DMRPlayer::parseString(std::string str)
{
    return 0;
}

int
DMRPlayer::GetUltraPlayerInfo(int type, unsigned int *value)
{
    SystemManager &sysManager = systemManager();
    UltraPlayer *player = sysManager.obtainMainPlayer();
    if(!player)
        return -1;

    switch(type) {
    case DLNA_GETVOLUME:
        *value = player->GetVolume();
        break;
    case DLNA_GETMUTE:
        *value = player->GetMute();
        break;
    case DLNA_GETPOSITION:
        *value = player->getCurrentTime();
        break;
    case DLNA_GETDURATION:
        *value = player->getTotalTime();
        break;
    case DLNA_PLAYSTATE:
        *value = player->mCurrentStatus;
    default:
        break;
    }

    sysManager.releaseMainPlayer(player);
    return 0;
}

void
DMRPlayer::handleDmrMessage()
{
    return;
}

}

