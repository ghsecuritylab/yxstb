#include "Hippo_OS.h"
#include "Hippo_Debug.h"
#include "Hippo_HString.h"
#include "Hippo_Context.h"
#include "Hippo_MediaPlayer.h"


using namespace std;

namespace Hippo
{

Player::Player(int aPlayerId):
    m_InstatncdId(aPlayerId),
    m_bPlaylistMode(false),
    m_bCycleFlag(true),
    m_bAvailable(false)
{
    m_eVideoMode = PlayerVideoMode_eFullScreen ;
    m_bAllowTrickPlay = 0;
    m_bNativeUIFlag = false;
    m_bAudioTrackUIFlag = false;
    m_bMuteUIFlag = false;
    m_bAudioVolumeUIFlag = false;
    m_bChnlNoUIFlag = false;
    m_VideoArea.m_x = 0;
    m_VideoArea.m_y = 0;
    m_VideoArea.m_w = 0;
    m_VideoArea.m_h = 0;
    m_pPlayerMgr = NULL;
    m_listener = NULL;
    m_client = NULL;
    m_browserHandler = NULL;
}

int Player::play(int aStartTime, time_type_e aType)
{
    HIPPO_DEBUG("run here.aStartTime=%d, aType=%d\n", aStartTime, aType);
    return -1;
}

int Player::stop()
{
    //HIPPO_DEBUG( "run here\n" );
    return -1;
}

int Player::close()
{
    //HIPPO_DEBUG( "run here\n" );
    return -1;
}

int Player::seekTo(unsigned long aPlayTime, time_type_e eType)
{
    return 0;
}

int Player::seekTo(const char* aPlayTime, time_type_e eType)
{

#if defined( OS_WIN32)
    play(atoi(aPlayTime), eType);
//	ipanel_porting_program_seek_ex( (int)eType, aPlayTime );
#endif

    return 0;
}

int Player::setProperty(player_property_type_e aType, HPlayerProperty& aValue)
{
    if(aType != aValue.m_eType)
        HIPPO_WARN("why not same.aType=%d,m_Type=%d\n", aType, aValue.m_eType);

    HIPPO_DEBUG("run here.aType=%s,aValue=%d\n", getProertyString(aType), aValue.m_Value.m_intVal);

    switch(aType) {
    case PlayerPropertyType_eVideoDisplayArea:
        HIPPO_DEBUG("PlayerPropertyType_eVideoDisplayArea x:%d, y:%d, w:%d, h:%d\n", aValue.m_Value.rect.m_x, aValue.m_Value.rect.m_y, aValue.m_Value.rect.m_w, aValue.m_Value.rect.m_h);
        m_VideoArea = aValue.m_Value.rect;
        break;
    case PlayerPropertyType_eVideoDisplayMode:
        m_eVideoMode = aValue.m_Value.vMode;
        break;
    case PlayerPropertyType_eNativeUIFlag:
        if(0 == aValue.m_Value.m_intVal)
            m_bNativeUIFlag = false;
        else
            m_bNativeUIFlag = true;
        break;
    case PlayerPropertyType_eAllowTrickPlayFlag:
        m_bAllowTrickPlay = aValue.m_Value.m_intVal;
        break;
    case PlayerPropertyType_eMuteUIFlag:
        if(0 == aValue.m_Value.m_intVal)
            m_bMuteUIFlag = false;
        else
            m_bMuteUIFlag = true;
        break;
    case PlayerPropertyType_eMuteFlag: {
        HString tFieldName("mp_MuteFlag");
        HString fieldValue;
        char buf[4] = {0};

        snprintf(buf, 4, "%d", aValue.m_Value.m_intVal);
        fieldValue += buf;
        HippoContext::getContextInstance()->ioctlWrite(tFieldName, fieldValue);
        break;
    }
    case PlayerPropertyType_eAudioVolume: {
        //ipanel_porting_set_volume( aValue.m_Value.value );
        HString tFieldName("mp_AudioVolume");
        HString fieldValue;
        char buf[4] = {0};
        snprintf(buf, 4, "%d", aValue.m_Value.m_intVal);
        fieldValue += buf;
        HippoContext::getContextInstance()->ioctlWrite(tFieldName, fieldValue);
        break;
    }
    case PlayerPropertyType_eAudioChannel:
        //ipanel_porting_program_switchMode( 1 );
        break;
    case PlayerPropertyType_eAudioTrackPID:
        //ipanel_porting_program_switchMode( 2 );
        break;
    case PlayerPropertyType_eSubtitlePID:
        //ipanel_porting_program_switchMode( 3 );
        break;
    case PlayerPropertyType_eCycleFlag:
        if(0 == aValue.m_Value.m_intVal)
            m_bCycleFlag = false;
        else
            m_bCycleFlag = true;
        break;
    case PlayerPropertyType_eChnlNoUIFlag:
        if(0 == aValue.m_Value.m_intVal)
            m_bChnlNoUIFlag = false;
        else
            m_bChnlNoUIFlag = true;
        break;
    case PlayerPropertyType_eSingleOrPlaylistMode:
        if(0 == aValue.m_Value.m_intVal)
            m_bPlaylistMode = false;
        else
            m_bPlaylistMode = true;
        break;
    default:
        HIPPO_DEBUG("unknown Type.aType=%s\n", getProertyString(aType));
        return -1;
    }
    return 0;
}

int Player::getProperty(player_property_type_e aType, HPlayerProperty& aResult)
{
    HString tFieldName;
    HString out;

    switch(aType) {
    case PlayerPropertyType_eVideoDisplayMode:
        break;
    case PlayerPropertyType_eCurrentPlayTime:
        tFieldName = "CurrentPlayTime";
        HippoContext::getContextInstance()->ioctlRead(tFieldName, aResult.m_PlayTime.m_strTimeStamp);
        aResult.m_PlayTime.m_TimeFormat = TimeType_eNPT;
        break;
    case PlayerPropertyType_eCurrentMediaDuration:
        tFieldName = "DurationTime";
        //aResult.m_PlayTime.m_strTimeStamp = ipanel_porting_media_duration( );
        HippoContext::getContextInstance()->ioctlRead(tFieldName, aResult.m_PlayTime.m_strTimeStamp);
        aResult.m_PlayTime.m_TimeFormat = TimeType_eNPT;
        break;
    case PlayerPropertyType_eNativeUIFlag:
        aResult.m_Value.m_intVal = m_bNativeUIFlag;
        break;
    case PlayerPropertyType_eMuteFlag: {
        //aResult.m_Value.m_intVal = ipanel_porting_get_mute( );
        tFieldName = "mp_MuteFlag";
        HippoContext::getContextInstance()->ioctlRead(tFieldName, aResult.m_strVal);  // aResult.m_Value.m_intVal );
        aResult.m_Value.m_intVal = atoi(aResult.m_strVal.c_str());
        break;
    }
    case PlayerPropertyType_eMuteUIFlag:
        aResult.m_Value.m_intVal = m_bMuteUIFlag;
        break;
    case PlayerPropertyType_eAudioVolume: {
        //aResult.m_Value.value = ipanel_porting_get_volume( );
        tFieldName = "mp_AudioVolume";
        HippoContext::getContextInstance()->ioctlRead(tFieldName, aResult.m_strVal);  // aResult.m_Value.m_intVal );
        aResult.m_Value.m_intVal = atoi(aResult.m_strVal.c_str());
        break;
    }
    case PlayerPropertyType_eChnlNoUIFlag:
        aResult.m_Value.m_intVal = m_bChnlNoUIFlag;
        break;
    case PlayerPropertyType_eCurrentPlayBackMode: {
        tFieldName = "PlayBackMode";
        HippoContext::getContextInstance()->ioctlRead(tFieldName, aResult.m_strVal);
        break;
    }
    default:
        HIPPO_DEBUG("unknown Type.aType=%s\n", getProertyString(aType));
        return -1;
    }
    HIPPO_DEBUG("run here.aType=%s\n", getProertyString(aType));
    return 0;
}

int Player::setSingleMedia(media_info_type_e aType, const char *aInfo)
{
    /*
     */
    return 0;
}

int Player::playSelectedNode(PlayerNode*, playlist_op_type_e eType, int aValue)
{
    return -1;
}

#define player_mkstr( x ) #x
const char* Player::getProertyString(player_property_type_e e) const
{
    static vector<const char*> Str(PlayerPropertyType_eMax);
    static int isInited = false;
    if(!isInited) {
        isInited = true;
        Str[PlayerPropertyType_eUnknown] = player_mkstr(PlayerPropertyType_eUnknown);
        Str[PlayerPropertyType_eSingleOrPlaylistMode] = player_mkstr(PlayerPropertyType_eSingleOrPlaylistMode);
        Str[PlayerPropertyType_eCycleFlag] = player_mkstr(PlayerPropertyType_eCycleFlag);
        Str[PlayerPropertyType_eRandomFlag] = player_mkstr(PlayerPropertyType_eRandomFlag);
        Str[PlayerPropertyType_eAutoDeleteFlag] = player_mkstr(PlayerPropertyType_eAutoDeleteFlag);

        Str[PlayerPropertyType_eAllowTrickPlayFlag] = player_mkstr(PlayerPropertyType_eAllowTrickPlayFlag);
        Str[PlayerPropertyType_eMuteFlag] = player_mkstr(PlayerPropertyType_eMuteFlag);
        Str[PlayerPropertyType_eSubtitleFlag] = player_mkstr(PlayerPropertyType_eSubtitleFlag);
        Str[PlayerPropertyType_eVideoDisplayMode] = player_mkstr(PlayerPropertyType_eVideoDisplayMode);
        Str[PlayerPropertyType_eVideoDisplayArea] = player_mkstr(PlayerPropertyType_eVideoDisplayArea);
        Str[PlayerPropertyType_eVideoAlpha] = player_mkstr(PlayerPropertyType_eVideoAlpha);
        Str[PlayerPropertyType_eNativeUIFlag] = player_mkstr(PlayerPropertyType_eNativeUIFlag);
        Str[PlayerPropertyType_eMuteUIFlag] = player_mkstr(PlayerPropertyType_eMuteUIFlag);
        Str[PlayerPropertyType_eChnlNoUIFlag] = player_mkstr(PlayerPropertyType_eChnlNoUIFlag);
        Str[PlayerPropertyType_eProgressBarUIFlag] = player_mkstr(PlayerPropertyType_eProgressBarUIFlag);
        Str[PlayerPropertyType_eAudioTrackUIFlag] = player_mkstr(PlayerPropertyType_eAudioTrackUIFlag);
        Str[PlayerPropertyType_eAudioVolumeUIFlag] = player_mkstr(PlayerPropertyType_eAudioVolumeUIFlag);
        Str[PlayerPropertyType_eAudioVolume] = player_mkstr(PlayerPropertyType_eAudioVolume);
        Str[PlayerPropertyType_eAudioChannel] = player_mkstr(PlayerPropertyType_eAudioChannel);
        Str[PlayerPropertyType_eAudioTrackPID] = player_mkstr(PlayerPropertyType_eAudioTrackPID);
        Str[PlayerPropertyType_eSubtitlePID] = player_mkstr(PlayerPropertyType_eSubtitlePID);
        Str[PlayerPropertyType_eAudioTrackInfo] = player_mkstr(PlayerPropertyType_eAudioTrackInfo);
        Str[PlayerPropertyType_eSubtitleInfo] = player_mkstr(PlayerPropertyType_eSubtitleInfo);

        Str[PlayerPropertyType_eMediaCount] = player_mkstr(PlayerPropertyType_eMediaCount);
        Str[PlayerPropertyType_eMediaPlaylist] = player_mkstr(PlayerPropertyType_eMediaPlaylist);

        Str[PlayerPropertyType_eCurrentMediaCode] = player_mkstr(PlayerPropertyType_eCurrentMediaCode);
        Str[PlayerPropertyType_eCurrentMediaIdx] = player_mkstr(PlayerPropertyType_eCurrentMediaIdx);
        Str[PlayerPropertyType_eCurrentMediaEntryId] = player_mkstr(PlayerPropertyType_eCurrentMediaEntryId);
        Str[PlayerPropertyType_eCurrentPlayChnlNum] = player_mkstr(PlayerPropertyType_eCurrentPlayChnlNum);
        Str[PlayerPropertyType_eCurrentAudioChannel] = player_mkstr(PlayerPropertyType_eCurrentAudioChannel);
        Str[PlayerPropertyType_eCurrentAudioTrack] = player_mkstr(PlayerPropertyType_eCurrentAudioTrack);
        Str[PlayerPropertyType_eCurrentSubtitle] = player_mkstr(PlayerPropertyType_eCurrentSubtitle);
        Str[PlayerPropertyType_eCurrentPlayTime] = player_mkstr(PlayerPropertyType_eCurrentPlayTime);
        Str[PlayerPropertyType_eCurrentPlayBackMode] = player_mkstr(PlayerPropertyType_eCurrentPlayBackMode);
        Str[PlayerPropertyType_eCurrentMediaDuration] = player_mkstr(PlayerPropertyType_eCurrentMediaDuration);
    }
    if(Str[e] == 0)
        return "no string";

    return Str[e];
}

int Player::set(const char * ioStr, const char * wrStr)
{
    printf("_____________ioStr = [%s], wrStr = [%s]\n", ioStr, wrStr);
    char    temp[1024];
    snprintf(temp, 1024, "mp_%s", ioStr);
    HString tFieldName(temp);
    HString fieldValue(wrStr);
    HippoContext::getContextInstance()->ioctlWrite(tFieldName, fieldValue);
    return 0;
}

const char * Player::get(const char * ioStr)
{
    printf("_____________ioStr = [%s]\n", ioStr);
    return NULL;
}

HEventDispatcher::event_status_e
Player::HandleEvent(HEvent& event)
{
    return HEventDispatcher::EventStatus_eContinue;
}
HEventDispatcher::event_status_e
Player::onEvent(int key)
{
    return HEventDispatcher::EventStatus_eContinue;
}
}

