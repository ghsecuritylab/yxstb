
#include "BrowserAgent.h"
#include "BrowserPlayerC10.h"
#include "BrowserAssertions.h"
#include "BrowserPlayerReporterUtility.h"

#include "SystemManager.h"
#include "ProgramParser.h"
#include "ProgramChannel.h"
#include "UltraPlayer.h"
#include "UltraPlayerVod.h"
#include "UltraPlayerVodList.h"
#include "UltraPlayerMultiple.h"
#ifdef PLAY_BGMUSIC
#include "UltraPlayerBGMusic.h"
#endif
#include "UltraPlayerUtility.h"

#include "SysSetting.h"
#include "AppSetting.h"

#include "config.h"

#include "codec.h"
#include "mid_sys.h"

#include "Session.h"
#include "app_sys.h"

#include "sdk/sdk.h"


namespace Hippo {

BrowserPlayerC10::BrowserPlayerC10(int id, player_type_e playerInstanceType)
	: BrowserPlayer(id, playerInstanceType)
{
    mProgramList = new ProgramList();
}

BrowserPlayerC10::~BrowserPlayerC10()
{
    mProgramList->clearAndRelease();
    mProgramList->safeUnref();
}

int
BrowserPlayerC10::joinChannel(int channelNumber)
{
    BROWSER_LOG("JoinChannel:%d\n", channelNumber);
    SystemManager &sysManager = systemManager();

    Program *program = sysManager.channelList().getProgramByNumberID(channelNumber);
    if (program) {
        UltraPlayer *player = sysManager.obtainMainPlayer();
        if (player && mPlayerMagic != player->magicNumber()) {
            player->stop();
        }
        sysManager.releaseMainPlayer(player);
        sysManager.detachMainPlayer(player);

        BrowserPlayerReporter *reporter = BrowserPlayerReporterCreate(program);

        player = UltraPlayerUtility::createPlayerByProgram(this, reporter, program);
        reporter->safeUnref();
        if (!player)
            return -1;

        player->mPlaylistMode = mPlaylistMode;
        player->mPlayCycleFlag = mPlaylistCycleFlag;
        player->mPlayRandomFlag = mPlaylistRandomFlag;
        player->mDisplayMode = m_eVideoMode;

        mPlayerMagic = player->magicNumber();
        player->play(0);
        if (!player->isFake()) {
            sysManager.attachMainPlayer(player);
        }
        UltraPlayer::setVideoClearFlag(0);
        appSettingSetInt("lastChannelID", channelNumber);
        player->SetCurrentChannelNum(channelNumber);
        player->removeMessages(MessageType_ConfigSave);
        Message *message = player->obtainMessage(MessageType_ConfigSave, 0, 0);

        player->sendMessageDelayed(message, 3000);

        //UltraPlayerMultiple *multiplePlayer = static_cast<UltraPlayerMultiple *>(player);
        player->unref();
        return 0;
    }
    else {
		sendMessageToEPGBrowser(MessageType_KeyDown, INVAILD_CHANNEL, 0, 0);
        return -1;
    }
}

int
BrowserPlayerC10::leaveChannel()
{
    SystemManager &sysManager = systemManager();
    BROWSER_LOG("leaveChannel\n");
    UltraPlayer *player = sysManager.obtainMainPlayer();
    if (player && player->magicNumber() == mPlayerMagic) {
        UltraPlayerMultiple *multiplePlayer = static_cast<UltraPlayerMultiple *>(player);
		int changeVideoMode = 0;
	    sysSettingGetInt("changevideomode", &changeVideoMode, 0);
        if(!changeVideoMode){            //0 is black screen 1 is last frame 2 is smooth change
            multiplePlayer->close(UltraPlayer::BlackScreenMode);
        }
        else {
            if(UltraPlayer::getVideoClearFlag()){
                multiplePlayer->close(UltraPlayer::BlackScreenMode);
                UltraPlayer::setVideoClearFlag(0);
            }
            else{
                multiplePlayer->close(UltraPlayer::LastFrameMode);
            }
        }
        sysManager.releaseMainPlayer(player);
        sysManager.detachMainPlayer(player);

#ifdef PLAY_BGMUSIC
        player = new UltraPlayerBGMusic(0, 0, 0, 5);
        if (!player)
            return -1;
		player->play(5);
        sysManager.attachMainPlayer(player);
        player->unref();
#endif
    }
    else
        sysManager.releaseMainPlayer(player);

    return 0;
}

int
BrowserPlayerC10::play(int startTime, time_type_e timeType)
{
    BROWSER_LOG("Play from %d by time type(%d) !\n", startTime, timeType);
	return BrowserPlayer::play(startTime, timeType);
}

int
BrowserPlayerC10::fastForward(int speed, unsigned long playTime, time_type_e timeType)
{
    BROWSER_LOG("FastForward %d !\n", speed);
	return BrowserPlayer::fastForward(speed, playTime, timeType);
}

int
BrowserPlayerC10::fastRewind(int speed, unsigned long playTime, time_type_e timeType)
{
    BROWSER_LOG("FastRewind %d !\n", speed);
	return BrowserPlayer::fastRewind(speed, playTime, timeType);
}

int
BrowserPlayerC10::seekTo(unsigned long playTime, time_type_e timeType)
{
    BROWSER_LOG("Seek to %d by type(%d) !\n", playTime, timeType);
	return BrowserPlayer::seekTo(playTime, timeType);
}

int
BrowserPlayerC10::seekTo(const char *playTime, time_type_e timeType)
{
    BROWSER_LOG("Seek to %s by type(%d) !\n", playTime, timeType);
	return BrowserPlayer::seekTo(playTime, timeType);
}

int
BrowserPlayerC10::pause()
{
    BROWSER_LOG("Pause !\n");
	return BrowserPlayer::pause();
}

int
BrowserPlayerC10::resume()
{
    BROWSER_LOG("Resume !\n");
	return BrowserPlayer::resume();
}

int
BrowserPlayerC10::stop()
{
    BROWSER_LOG("Stop !\n");
	return BrowserPlayer::stop();
}

int
BrowserPlayerC10::close()
{
	return BrowserPlayer::close();
}

int
BrowserPlayerC10::setProperty(player_property_type_e aType, HPlayerProperty& aValue)
{
    switch(aType) {
#ifdef Liaoning
    case PlayerPropertyType_eNativeUIFlag: {
        if (session().getPlatform() == PLATFORM_HW) {
            if(aValue.m_Value.m_intVal){
                UltraPlayer::enableUI(true);
                UltraPlayer::setUIFlags(UltraPlayer::PlayState_Mask);
            }else{
                UltraPlayer::enableUI(false);
                UltraPlayer::clearUIFlags(UltraPlayer::PlayState_Mask);
            }
        } else { //PLATFORM_ZTE
            UltraPlayer::enableUI(true);
        }
        break;
    }
#endif
    case PlayerPropertyType_eVideoAlpha: { // 电信3.0规范，0是不透明，100是全透。
        unsigned char videoAlpha = 0xff * (100 - aValue.m_Value.m_intVal) / 100;
        HYW_vout_setVideoAlpha(0, videoAlpha);
        break;
    }
    default: {
        BrowserPlayer::setProperty(aType, aValue);
        break;
    }
    }
    return 0;
}

int
BrowserPlayerC10::getProperty(player_property_type_e aType, HPlayerProperty& aResult)
{
    SystemManager &sysManager = systemManager();
    UltraPlayer *player = sysManager.obtainMainPlayer();

    BROWSER_LOG("GetProperty aType=%s\n", getProertyString(aType));
    switch(aType) {
    case PlayerPropertyType_eVideoAlpha: {
        unsigned char videoAlpha = 0;
        HYW_vout_getVideoAlpha(0, &videoAlpha);
        aResult.m_Value.m_intVal = 100 - (videoAlpha / 0xff * 100);
        break;
    }
    case PlayerPropertyType_eAudioTrackPIDs: {
        if(player) {
            char *tReturnStr = 0;
            int tAudioNum = 0;
            int tIndex = 0;

            codec_audio_track_num(&tAudioNum);
            if(tAudioNum <= 0) {
                break;
            }
            tReturnStr = (char *)malloc(64 * tAudioNum);
            if(tReturnStr == 0)
                break;
            memset(tReturnStr, 0, 64 * tAudioNum);
            sprintf(tReturnStr,"{\"Ipids\":\"%d\",\"Ipidaddr\":\"[", tAudioNum);
            for(tIndex = 0; tIndex < tAudioNum; tIndex++) {
                int tPID = 0;
                char tLanguage[8] = {0};
                char tBuff[64] = {0};

                codec_audio_track_get_pid(tIndex, &tPID);
                codec_subtitle_lang(tIndex, tLanguage);
                sprintf(tBuff, "{\"AudioPID\":\"%d\",\"AudioLanguage\":\"%s\"}", tPID, tLanguage);
                strcat(tReturnStr, tBuff);
                if(tIndex < tAudioNum - 1) {
                    strcat(tReturnStr, ",");
                }
            }
            strcat(tReturnStr, "]\"}");
            aResult.m_strVal = tReturnStr;
            if(tReturnStr)
                free(tReturnStr);
        }
        break;
    }
    case PlayerPropertyType_eSubtitlePIDs:{
        if(player) {
            char *tReturnStr = 0;
            int tSubtitleNum = 0;
            int tIndex = 0;

            codec_subtitle_num(&tSubtitleNum);
            if(tSubtitleNum <= 0) {
                break;
            }
            tReturnStr = (char *)malloc(64 * tSubtitleNum);
            if(tReturnStr == 0)
                break;
            memset(tReturnStr, 0, 64 * tSubtitleNum);
            sprintf(tReturnStr,"{\"Ipids\":\"%d\",\"Ipidaddr\":\"[", tSubtitleNum);
            for(tIndex = 0; tIndex < tSubtitleNum; tIndex++) {
                unsigned short tPID = 0;
                char tLanguage[8] = {0};
                char tBuff[64] = {0};

                codec_subtitle_pid(tIndex, &tPID);
                codec_subtitle_lang(tIndex, tLanguage);
                sprintf(tBuff, "{\"SubtitlePID\":\"%d\",\"SubtitleLanguage\":\"%s\"}", tPID, tLanguage);
                strcat(tReturnStr, tBuff);
                if(tIndex < tSubtitleNum - 1) {
                    strcat(tReturnStr, ",");
                }
            }
            strcat(tReturnStr, "]\"}");
            aResult.m_strVal = tReturnStr;
            if(tReturnStr)
                free(tReturnStr);
        }
        break;
    }
    case PlayerPropertyType_eMediaCount: {
        if(this) {
            aResult.m_Value.m_intVal = this->getMediaCount();
        }
        break;
    }
    case PlayerPropertyType_eCurrentMediaIdx: {
        if(this) {
            aResult.m_Value.m_intVal = this->getCurrentIndex();
        }
        break;
    }
    case PlayerPropertyType_eCurrentMediaEntryId: {
        if(this) {
            aResult.m_strVal = this->getCurrentEntryID();
        }
        break;
    }
    case PlayerPropertyType_eMediaPlaylist:{
        int tIndex = 0;
        int tProgramCount = mProgramList->getProgramCount();
        char *tPlaylist = 0;
        char *mPlaylist = 0;

        mPlaylist = (char *)malloc(1024 * tProgramCount);
        if(!mPlaylist)
            return -1;
        strcpy(mPlaylist, "[");
        tPlaylist = (char *)malloc(1024);
        if(!tPlaylist){
            free(mPlaylist);
            mPlaylist = 0;
            return -1;
        }
        for(tIndex = 0; tIndex < tProgramCount; tIndex ++){
            Program *tProgram = mProgramList->getProgramByIndex(tIndex);
            VodSource *tVodsource = tProgram->getVodSource(0);

            memset(tPlaylist, 0, 1024);
            sprintf(tPlaylist,
                    "{mediaUrl:\"%s\",mediaCode: \"%s\",mediaType:%d,audioType:%d,videoType:%d,streamType:%d,drmType:%d,fingerPrint:%d,copyProtection:%d,allowTrickmode:%d,startTime:%s,endTime:%s,entryID:\"%s\"}",
                    tVodsource->GetUrl().c_str(),
                    tVodsource->GetMediaCode().c_str(),
                    tVodsource->GetMediaType(),
                    tVodsource->GetAudioType(),
                    tVodsource->GetVideoType(),
                    tVodsource->GetStreamType(),
                    tVodsource->GetDrmType(),
                    tVodsource->GetFingerPrint(),
                    tVodsource->GetCopyProtection(),
                    tVodsource->GetAllowTrickmode(),
                    tVodsource->GetStartTime().c_str(),
                    tVodsource->GetEndTime().c_str(),
                    tVodsource->GetEntryID().c_str());
            strcat(mPlaylist, tPlaylist);
            if(tIndex < tProgramCount - 1){
                strcat(mPlaylist, ",");
            }
        }
        strcat(mPlaylist, "]");
        if(tPlaylist)
            free(tPlaylist);
        BROWSER_LOG("\nThe PlayerPropertyType_eMediaPlaylist get :\n");
        BROWSER_LOG("%s\n\n", mPlaylist);
        aResult.m_strVal = mPlaylist;
        if(mPlaylist) {
            free(mPlaylist);
            mPlaylist = 0;
        }
        BROWSER_LOG("%s\n\n", aResult.m_strVal.c_str());
        break;
    }
    default: {
        sysManager.releaseMainPlayer(player);
	    return BrowserPlayer::getProperty(aType, aResult);
	}
    }
    sysManager.releaseMainPlayer(player);
    return 0;
}

int
BrowserPlayerC10::addSingleMedia(media_info_type_e pType, int pIndex, const char *mediaString)
{
    int ret = -1;
    Program *tProgram = 0;

    if(pType != Player::MediaInfoType_eJson){
        return ret;
    }
    BROWSER_LOG("Media(%s) by index(%d)!\n", mediaString, pIndex);

    //ret = programParser().parseMediaList(&mProgramList, mediaString);
    if(pIndex == 0){ // Hippo glue 将add single media 和 add batch media 都放到了这里处理，因此这么改
        ret = programParser().parseMediaList(mProgramList, mediaString);
    }else{
        tProgram = programParser().parseSingleMedia(mediaString);

        if(tProgram == 0)
            return -1;
        ret = mProgramList->addProgramToPosition(tProgram, pIndex);
    }

    if(!ret){
        SystemManager &sysManager = systemManager();
        UltraPlayer *player = sysManager.obtainMainPlayer();

        if(player && mPlayerMagic != player->magicNumber()){
            player->stop();
        }
        sysManager.releaseMainPlayer(player);
        sysManager.detachMainPlayer(player);
        BrowserPlayerReporter *reporter = BrowserPlayerReporterCreate(mProgramList->getProgramByIndex(pIndex));

        player = new UltraPlayerVodList(this, reporter, mProgramList, pIndex);

        reporter->unref();
        if(!player){
            return -1;
        }
        player->mPlaylistMode = mPlaylistMode;
        player->mPlayCycleFlag = mPlaylistCycleFlag;
        player->mPlayRandomFlag = mPlaylistRandomFlag;
        player->mDisplayMode = m_eVideoMode;
        mPlayerMagic = player->magicNumber();
        if (!player->isFake()){
            sysManager.attachMainPlayer(player);
        }
        player->unref();
        return 0;
    }
    return ret;
}

int
BrowserPlayerC10::addBatchMedia(const char* mediaString)
{
    return -1;
}

int
BrowserPlayerC10::selectPlayNode(playlist_op_type_e pType, HPlaylistProperty& aValue)
{
    SystemManager &sysManager = systemManager();
    UltraPlayer *player = sysManager.obtainMainPlayer();

    BROWSER_LOG("mPlaylistMode(%d) player(%p)<->this(%p) %d<->%d\n", mPlaylistMode, player, this, player->magicNumber(), mPlayerMagic);
    if(player && player->magicNumber() == mPlayerMagic) {
        if(player->mPlaylistMode) {
            switch(pType){
        	case PlaylistOpType_eIdxSelectByIndex:{
                printf("CTC select media by index %d !\n", aValue.m_oldValue.m_intVal);
                player->PlayByIndex(aValue.m_oldValue.m_intVal);
                break;
            }
        	case PlaylistOpType_eIdxSelectOffset:{
        	    printf("CTC select media by offset %d !\n", aValue.m_oldValue.m_intVal);
                player->PlayByOffset(aValue.m_oldValue.m_intVal);
                break;
            }
        	case PlaylistOpType_eIdxSelectByEntryId:{
        	    printf("CTC select media by entryID %s !\n", aValue.m_oldStrVal.c_str());
                player->PlayByEntryId(aValue.m_oldStrVal.c_str());
                break;
            }
        	case PlaylistOpType_eIdxSelectNext:{
        	    printf("CTC select next media !\n");
                player->PlayNext();
                break;
            }
        	case PlaylistOpType_eIdxSelectPrevious:{
        	    printf("CTC select previous media !\n");
                player->PlayPrevious();
                break;
            }
        	case PlaylistOpType_eIdxSelectFirst:{
        	    printf("CTC select first media !\n");
                player->PlayFirst();
                break;
            }
        	case PlaylistOpType_eIdxSelectLast:{
        	    printf("CTC select last media !\n");
                player->PlayLast();
                break;
            }
            case PlaylistOpType_eUnknown:
            default:{
                break;
            }
            }
        }
    }
    sysManager.releaseMainPlayer(player);
    return 0;
}

int
BrowserPlayerC10::removePlayNode(playlist_op_type_e pType, HPlaylistProperty& aValue)
{
    BROWSER_LOG("CTC removePlayNode %p\n", mProgramList);

    switch(pType){
    case PlaylistOpType_eIdxRemoveAll:{
        BROWSER_LOG("CTC remove all media !\n");
        mProgramList->clearAndRelease();
        break;
    }
    case PlaylistOpType_eIdxRemoveByOffset:{
        break;
    }
    case PlaylistOpType_eIdxRemoveByEntryId:{
        BROWSER_LOG("CTC remove media %s !\n", aValue.m_oldStrVal.c_str());
        mProgramList->removeProgramByStringID(aValue.m_oldStrVal.c_str());
        break;
    }
    case PlaylistOpType_eIdxRemoveByIndex:{
        BROWSER_LOG("CTC remove media %d !\n", aValue.m_oldValue.m_intVal);
        mProgramList->removeProgramByIndex(aValue.m_oldValue.m_intVal);
        break;
    }
    case PlaylistOpType_eUnknown:
    default:{
        return -1;
    }
    }
    return 0;
}

int
BrowserPlayerC10:: movePlayNode(playlist_op_type_e pType, HPlaylistProperty& aValue)
{
    Program *program = 0;

    switch(pType){
    case PlaylistOpType_eIdxMoveByIndex:{
        BROWSER_LOG("CTC move media(%s) to %d !\n", aValue.m_oldStrVal.c_str(), aValue.m_newValue.m_intVal);
	    program = mProgramList->getProgramByStringID(aValue.m_oldStrVal.c_str());

        if(program != 0) {
            mProgramList->moveToPosition(program, aValue.m_newValue.m_intVal);
        }
        break;
    }
	case PlaylistOpType_eIdxMoveByIndex1:{
	    BROWSER_LOG("CTC move media(%d) to %d !\n", aValue.m_oldValue.m_intVal, aValue.m_newValue.m_intVal);
	    program = mProgramList->getProgramByIndex(aValue.m_oldValue.m_intVal);

        if(program != 0) {
            mProgramList->moveToPosition(program, aValue.m_newValue.m_intVal);
        }
        break;
    }
    case PlaylistOpType_eIdxMoveByOffset:{
        BROWSER_LOG("CTC move media(%s) offset %d !\n", aValue.m_oldStrVal.c_str(), aValue.m_newValue.m_intVal);
	    program = mProgramList->getProgramByStringID(aValue.m_oldStrVal.c_str());
        int toIndex = 0;

        if(program != 0) {
            toIndex = mProgramList->getProgramIndex(program);
            toIndex += aValue.m_newValue.m_intVal;
            mProgramList->moveToPosition(program, toIndex);
        }
        break;
    }
    case PlaylistOpType_eIdxMoveByOffset1:{
        BROWSER_LOG("CTC move media(%d) offset %d !\n", aValue.m_oldValue.m_intVal, aValue.m_newValue.m_intVal);
        program = mProgramList->getProgramByIndex(aValue.m_oldValue.m_intVal);
        int toIndex = aValue.m_oldValue.m_intVal + aValue.m_newValue.m_intVal;

        if(program != 0) {
            mProgramList->moveToPosition(program, toIndex);
        }
        break;
    }
    case PlaylistOpType_eIdxMoveToNext:{
        BROWSER_LOG("CTC move media(%s) to next !\n", aValue.m_oldStrVal.c_str());
        program = mProgramList->getProgramByStringID(aValue.m_oldStrVal.c_str());

        if(program != 0) {
            mProgramList->moveToNext(program);
        }
        break;
    }
    case PlaylistOpType_eIdxMoveToNext1:{
        BROWSER_LOG("CTC move media(%d) to next !\n", aValue.m_oldValue.m_intVal);
        program = mProgramList->getProgramByIndex(aValue.m_oldValue.m_intVal);

        if(program != 0) {
            mProgramList->moveToNext(program);
        }
        break;
    }
    case PlaylistOpType_eIdxMoveToPrevious:{
        BROWSER_LOG("CTC move media(%s) to Previous !\n", aValue.m_oldStrVal.c_str());
        program = mProgramList->getProgramByStringID(aValue.m_oldStrVal.c_str());

        if(program != 0) {
            mProgramList->moveToPrevious(program);
        }
        break;
    }
    case PlaylistOpType_eIdxMoveToPrevious1:{
        BROWSER_LOG("CTC move media(%d) to Previous !\n", aValue.m_oldValue.m_intVal);
        program = mProgramList->getProgramByIndex(aValue.m_oldValue.m_intVal);

        if(program != 0) {
            mProgramList->moveToPrevious(program);
        }
        break;
    }
    case PlaylistOpType_eIdxMoveToFirst:{
        BROWSER_LOG("CTC move media(%s) to first !\n", aValue.m_oldStrVal.c_str());
        program = mProgramList->getProgramByStringID(aValue.m_oldStrVal.c_str());

        if(program != 0) {
            mProgramList->moveToFirst(program);
        }
        break;
    }
    case PlaylistOpType_eIdxMoveToFirst1:{
        BROWSER_LOG("CTC move media(%d) to first !\n", aValue.m_oldValue.m_intVal);
        program = mProgramList->getProgramByIndex(aValue.m_oldValue.m_intVal);

        if(program != 0) {
            mProgramList->moveToFirst(program);
        }
        break;
    }
    case PlaylistOpType_eIdxMoveToLast:{
        BROWSER_LOG("CTC move media(%s) to last !\n", aValue.m_oldStrVal.c_str());
        program = mProgramList->getProgramByStringID(aValue.m_oldStrVal.c_str());

        if(program != 0) {
            mProgramList->moveToPrevious(program);
        }
        break;
    }
    case PlaylistOpType_eIdxMoveToLast1:{
        BROWSER_LOG("CTC move media(%d) to last !\n", aValue.m_oldValue.m_intVal);
        program = mProgramList->getProgramByIndex(aValue.m_oldValue.m_intVal);

        if(program != 0) {
            mProgramList->moveToPrevious(program);
        }
        break;
    }
    case PlaylistOpType_eUnknown:
    default:{
        return -1;
    }
    }
    return 0;
}

int
BrowserPlayerC10::getMediaCount()
{
    return mProgramList->getProgramCount();
}

int
BrowserPlayerC10::getCurrentIndex()
{
    int mCurrentIndex = 0;
    SystemManager &sysManager = systemManager();
    UltraPlayer *player = sysManager.obtainMainPlayer();

    if(player && player->magicNumber() == mPlayerMagic){
        if(player->mPlaylistMode){
            mCurrentIndex = player->getVodListCurIndex();
        }
    }
    sysManager.releaseMainPlayer(player);
    return mCurrentIndex;
}

const char*
BrowserPlayerC10::getCurrentEntryID()
{
    int mCurrentIndex = 0;
    Program *program = NULL;
    SystemManager &sysManager = systemManager();
    UltraPlayer *player = sysManager.obtainMainPlayer();

    if(player && player->magicNumber() == mPlayerMagic){
        if(player->mPlaylistMode){
            mCurrentIndex = player->getVodListCurIndex();
        }
    }
    sysManager.releaseMainPlayer(player);
    program = mProgramList->getProgramByIndex(mCurrentIndex);
    if(program == 0){
        return 0;
    }
    return program->getStringID();
}

int
BrowserPlayerC10::getPlaylist()
{
    return 0;
}

int
BrowserPlayerC10::playFromStart()
{
    if(!mPlaylistMode)
        return -1;
    return 0;
}

void
BrowserPlayerC10::clearForRecycle()
{
    mProgramList->clearAndRelease();
    BrowserPlayer::clearForRecycle();
}

extern "C" void mid_plane_browser_positionOffset(int x, int y, int w, int h, int *my_x, int *my_y, int *my_w, int *my_h);

int
BrowserPlayerC10::refreshVideoDisplay()
{
	SystemManager &sysManager = systemManager();
	UltraPlayer *player = NULL;

	if(mPlayerInstanceType == PlayerType_eMain){
		player = sysManager.obtainMainPlayer();
	} else if(mPlayerInstanceType == PlayerType_ePIP) {
		player = sysManager.obtainPipPlayer(0);
	} else {
		player = mActualPlayer;
	}
	if(player){
        mid_plane_browser_positionOffset( m_VideoArea.m_x,
                                          m_VideoArea.m_y,
                                          m_VideoArea.m_w,
                                          m_VideoArea.m_h,
                                          &player->m_VideoDisplaytop,
                                          &player->m_VideoDisplayleft,
                                          &player->m_VideoDisplaywidth,
                                          &player->m_VideoDisplayheight);
		player->mDisplayMode = m_eVideoMode;
		if(mPlayerInstanceType == PlayerType_eMosaic) {
			player->close(0);
			player->play(0);
			return 0;
		}
		player->refreshVideoDisplay();
	} else { // Domestic EPG often setting this value before create player.
        //PlayerHWBase::refreshVideoDisplay();
        int tVideoX = 0, tVideoY = 0, tVideoW = 0, tVideoH = 0;

        mid_plane_browser_positionOffset( m_VideoArea.m_x,
                                          m_VideoArea.m_y,
                                          m_VideoArea.m_w,
                                          m_VideoArea.m_h,
                                          &tVideoX,
                                          &tVideoY,
                                          &tVideoW,
                                          &tVideoH);

        if((tVideoX < 0) || (tVideoY < 0) || (tVideoW < 0) || (tVideoH < 0))
            return 0;

    	int s_width = 0;
    	int s_height = 0;

        BROWSER_LOG("Index(0)Mode(%d)x(%d)y(%d)w(%d)h(%d)\n", m_eVideoMode, tVideoX, tVideoY, tVideoW, tVideoH);
    	if(PlayerVideoMode_eFullScreen == m_eVideoMode){
    		mid_stream_rect(0, 0, 0, 0, 0);
    		mid_sys_aspectRatioMode_set(2);
        	mid_sys_video_show(1);
    	} else if(PlayerVideoMode_eByWidth == m_eVideoMode) {
    		mid_stream_rect(0, 0, 0, 0, 0);
    		mid_sys_aspectRatioMode_set(1);
        	mid_sys_video_show(1);
    	} else if(PlayerVideoMode_eByHeight == m_eVideoMode){
    		mid_stream_rect(0, 0, 0, 0, 0);
    		mid_sys_aspectRatioMode_set(0);
        	mid_sys_video_show(1);
    	} else if (PlayerVideoMode_eVideoByArea == m_eVideoMode){
    		ygp_layer_getScreenSize(&s_width, &s_height);
    		BROWSER_LOG("Screen width(%d) height(%d)\n", s_width, s_height);
    		BROWSER_LOG("Screen max width(%d) height(%d)\n", SCREEN_MAX_WIDTH, SCREEN_MAX_HEIGHT);
    		if(tVideoW >= s_width || tVideoH >= s_height) {
    		    mid_stream_rect(0, 0, 0, 0, 0);
    		} else {
        		mid_stream_rect(0, ((tVideoX * s_width)/SCREEN_MAX_WIDTH), ((tVideoY * s_height)/SCREEN_MAX_HEIGHT), ((tVideoW * s_width)/SCREEN_MAX_WIDTH), ((tVideoH * s_height)/SCREEN_MAX_HEIGHT));
        	}
        	mid_sys_video_show(1);
    	} else if(PlayerVideoMode_eVideoHide == m_eVideoMode){
    	    mid_sys_video_show(0);
    	}
	}
	if(mPlayerInstanceType == PlayerType_eMain)
		sysManager.releaseMainPlayer(player);
	else if(mPlayerInstanceType == PlayerType_ePIP)
		sysManager.releasePipPlayer(player, 0);
    //BROWSER_LOG("RefreshVideoDisplay x(%d)y(%d)w(%d)h(%d)\n", m_VideoArea.m_x, m_VideoArea.m_y, m_VideoArea.m_w, m_VideoArea.m_h);
	return 0;
}

void *createBrowserPlayerC10(int id)
{
    return new BrowserPlayerC10(id, BrowserPlayer::PlayerType_eMain);
}

} // namespace Hippo
