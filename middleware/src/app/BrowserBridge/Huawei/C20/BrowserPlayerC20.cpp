
#include "BrowserPlayerC20.h"
#include "BrowserPlayerReporterUtility.h"
#include "BrowserPlayerReporter.h"
#include "BrowserAssertions.h"

#include "UltraPlayerMultipleC20.h"
#include "SystemManager.h"
#include "ProgramParser.h"
#include "ProgramChannel.h"
#include "UltraPlayerVod.h"
#include "UltraPlayerMultiple.h"
#include "UltraPlayerUtility.h"
#include "ProgramVOD.h"
#include "AppSetting.h"
#include "SysSetting.h"

#include "Assertions.h"
#include "codec.h"

namespace Hippo {

BrowserPlayerC20::BrowserPlayerC20(int id, player_type_e playerInstanceType)
	: BrowserPlayer(id, playerInstanceType)
{
}

BrowserPlayerC20::~BrowserPlayerC20()
{
}

int
BrowserPlayerC20:: getSwitchChannelMode()
{
	int mode = 0;
    sysSettingGetInt("changevideomode", &mode, 0);

	if(mode== 0){
		return UltraPlayer::BlackScreenMode;
	} else {
		return UltraPlayer::LastFrameMode;
	}

}

int
BrowserPlayerC20::joinChannel(int channelNumber)
{
	SystemManager &sysManager = systemManager();

    //Program *program = sysManager.channelList().getProgramByNumberID(channelNumber);
    Program *program = sysManager.channelList().getProgramByNumberKey(channelNumber);

    if (program) {
		UltraPlayer *player = NULL;
		if(mPlayerInstanceType == PlayerType_eMain){
		    player = sysManager.obtainMainPlayer();
		    if(player){
		        player->close(getSwitchChannelMode());
		    }
		    sysManager.releaseMainPlayer(player);
		    sysManager.detachMainPlayer(player);
		    player = sysManager.obtainPipPlayer(1);
		    if( player ){
		    	sysManager.destoryPipPlayer(1);
		    }
		} else if(mPlayerInstanceType == PlayerType_ePIP) {
			player = sysManager.obtainPipPlayer(0);
			if(player){
				player->close(getSwitchChannelMode());
			}
			sysManager.releasePipPlayer(player, 0);
			sysManager.detachPipPlayer(player, 0);
			player = sysManager.obtainPipPlayer(1);
			if( player ){
	    		sysManager.destoryPipPlayer(1);
	    	}
		} else {
			player = sysManager.obtainMainPlayer();
		    if(player){
		        player->close(getSwitchChannelMode());
	    	}
		    sysManager.releaseMainPlayer(player);
		    sysManager.detachMainPlayer(player);
		    player = sysManager.obtainPipPlayer(0);
			if(player){
				player->close(getSwitchChannelMode());
			} else {
				BROWSER_LOG_ERROR("BrowserPlayerC20::joinChannel player is NULL\n");
			}
			sysManager.releasePipPlayer(player, 0);
			sysManager.detachPipPlayer(player, 0);
		}

        BrowserPlayerReporter *reporter = BrowserPlayerReporterCreate(program);
        player = UltraPlayerUtility::createPlayerByProgram(this, reporter, program);
        reporter->safeUnref();
        if (!player)
            return -1;
		player->m_PlayerInstanceType = mPlayerInstanceType;
		player->mPlaylistMode = mPlaylistMode;
		player->mPlayCycleFlag = mPlaylistCycleFlag;
		player->mPlayRandomFlag = mPlaylistRandomFlag;
		player->m_subtitleFlag = mSubtitleFlag;
		player->m_muteFlag = mMuteFlag;
		player->m_VideoDisplaytop = m_VideoArea.m_x;
		player->m_VideoDisplayleft = m_VideoArea.m_y;
		player->m_VideoDisplaywidth = m_VideoArea.m_w;
		player->m_VideoDisplayheight = m_VideoArea.m_h;
		player->mDisplayMode = m_eVideoMode;
        mPlayerMagic = player->magicNumber();
		if (!player->isFake()){
			if(mPlayerInstanceType == PlayerType_eMain){
		    	sysManager.attachMainPlayer(player);
		    	player->setIndex(0);
		    } else if(mPlayerInstanceType == PlayerType_ePIP){
		    	BROWSER_LOG("BrowserPlayerC20::joinChannel this is pip's instance\n");
		    	sysManager.attachPipPlayer(player, 0);
		    	player->setIndex(1);
		    } else {
		    	BROWSER_LOG("BrowserPlayerC20::joinChannel this is mosaic's instance, mosaic num(%d)\n", sysManager.getMosaicPlayerNum());
                    player->setIndex(sysManager.getMosaicPlayerNum() + 2);
                    sysManager.attachPipPlayer(player, 1);
		    }
	  	}
	  	player->SetMacrovisionFlag(mMacrovisionFlag);
	  	player->SetHDCPFlag(mHDCPFlag);
	  	player->SetCGMSAFlag(mCGSMAFlag);
		player->play(0);
		BROWSER_LOG("BrowserPlayerC20::joinChannel by player(%p)\n", player);
		if(mPlayerInstanceType == PlayerType_eMain || mPlayerInstanceType == PlayerType_ePIP)
	 		player->unref();
        return 0;
    }
    else {
        return -1;
    }
}

int
BrowserPlayerC20::leaveChannel()
{
	SystemManager &sysManager = systemManager();

	UltraPlayer *player = NULL;

	if(mPlayerInstanceType == PlayerType_eMain){
		player = sysManager.obtainMainPlayer();
	} else if(mPlayerInstanceType == PlayerType_ePIP) {
		player = sysManager.obtainPipPlayer(0);
	} else {
		player = sysManager.obtainPipPlayer(1);
	}
	if (player && player->magicNumber() == mPlayerMagic) {
		UltraPlayerMultiple *multiplePlayer = static_cast<UltraPlayerMultiple *>(player);
		multiplePlayer->close(getSwitchChannelMode());
	}
    if(mPlayerInstanceType == PlayerType_eMain){
		sysManager.releaseMainPlayer(player);
        sysManager.detachMainPlayer(player);
	} else if(mPlayerInstanceType == PlayerType_ePIP) {
		sysManager.releasePipPlayer(player, 0);
        sysManager.detachPipPlayer(player, 0);
	} else {
		sysManager.destoryPipPlayer(1);
	}
    return 0;
}

int
BrowserPlayerC20::play(int startTime, time_type_e timeType)
{
	SystemManager &sysManager = systemManager();
	std::string m_channelID;

	BROWSER_LOG("BrowserPlayerC20::play mPlayerInstanceType = %d\n", mPlayerInstanceType);
	if(mProgramToPlay){
		UltraPlayer *player = NULL;
		if(mPlayerInstanceType == PlayerType_eMain){
		    player = sysManager.obtainMainPlayer();
		    if(player){
		        player->close(getSwitchChannelMode());
		    }
		    sysManager.releaseMainPlayer(player);
		    sysManager.detachMainPlayer(player);
		    player = sysManager.obtainPipPlayer(1);
		    if( player ){
		    	BROWSER_LOG("BrowserPlayerC20::play destroy mosaic\n");
		    	sysManager.destoryPipPlayer(1);
		    }
		} else if(mPlayerInstanceType == PlayerType_ePIP) {
			player = sysManager.obtainPipPlayer(0);
			if(player){
				player->close(getSwitchChannelMode());
			}
			sysManager.releasePipPlayer(player, 0);
			sysManager.detachPipPlayer(player, 0);
			player = sysManager.obtainPipPlayer(1);
			if( player ){
	    		sysManager.destoryPipPlayer(1);
	    	}
		} else {
			player = sysManager.obtainMainPlayer();
			if(player){
				player->close(getSwitchChannelMode());
			}
			sysManager.releaseMainPlayer(player);
			sysManager.detachMainPlayer(player);
			player = sysManager.obtainPipPlayer(0);
			if(player){
				player->close(getSwitchChannelMode());
			} else {
				BROWSER_LOG_ERROR("BrowserPlayerC20::play player is NULL\n");
			}
			sysManager.releasePipPlayer(player, 0);
			sysManager.detachPipPlayer(player, 0);
		}

		Program::ProgramType type = mProgramToPlay->getType();
		BROWSER_LOG("BrowserPlayerC20::play ProgramType(%d)\n", type);
		BrowserPlayerReporter *reporter = BrowserPlayerReporterCreate(mProgramToPlay);
		player = UltraPlayerUtility::createPlayerByProgram(this,reporter,mProgramToPlay);
		reporter->safeUnref();
		if(type == Program::PT_CHANNEL ){
			if(mPlayerInstanceType == PlayerType_eMosaic){
				mActualPlayer = player;
			}
		}
		if(player == 0){
			if(type != Program::PT_CHANNEL){
				mProgramToPlay->unref();
			}
			mProgramToPlay = 0;
		    return -1;
		}

		if(type == Program::PT_CHANNEL){
			player->SetCurrentChannelNum(((ProgramChannel *)mProgramToPlay)->GetChanKey());
		} else {
			player->SetCurrentChannelNum(-1);
			mProgramToPlay->unref();
		}
		mProgramToPlay = 0;

		player->m_PlayerInstanceType = mPlayerInstanceType;
		player->mPlaylistMode = mPlaylistMode;
		player->mPlayCycleFlag = mPlaylistCycleFlag;
		player->mPlayRandomFlag = mPlaylistRandomFlag;
		player->m_subtitleFlag = mSubtitleFlag;
		player->m_muteFlag = mMuteFlag;
		player->m_VideoDisplaytop = m_VideoArea.m_x;
		player->m_VideoDisplayleft = m_VideoArea.m_y;
		player->m_VideoDisplaywidth = m_VideoArea.m_w;
		player->m_VideoDisplayheight = m_VideoArea.m_h;
		player->mDisplayMode = m_eVideoMode;


		mPlayerMagic = player->magicNumber();
		if (!player->isFake()){
			if(mPlayerInstanceType == PlayerType_eMain){
				sysManager.attachMainPlayer(player);
				player->setIndex(0);
			} else if(mPlayerInstanceType == PlayerType_ePIP){
				BROWSER_LOG("BrowserPlayerC20::play this is pip's instance\n");
				sysManager.attachPipPlayer(player, 0);
				player->setIndex(1);
			} else {
				BROWSER_LOG("BrowserPlayerC20::play this is mosaic's instance, mosaic nums(%d)\n", sysManager.getMosaicPlayerNum());
				player->setIndex(sysManager.getMosaicPlayerNum() + 2);
                sysManager.attachPipPlayer(player, 1);
			}
	  	}

	  	player->SetMacrovisionFlag(mMacrovisionFlag);
	  	player->SetHDCPFlag(mHDCPFlag);
	  	player->SetCGMSAFlag(mCGSMAFlag);
		player->play(0);
		BROWSER_LOG("BrowserPlayerC20::play by player(%p)\n", player);
		if(mPlayerInstanceType == PlayerType_eMain || mPlayerInstanceType == PlayerType_ePIP)
			player->unref();
  	} else {
  		return -1;
  	}
  	return 0;
}

int BrowserPlayerC20::refreshVideoDisplay()
{
	SystemManager &sysManager = systemManager();
	UltraPlayer *player = NULL;

	if(mPlayerInstanceType == PlayerType_eMain){
		player = sysManager.obtainMainPlayer();

	} else if(mPlayerInstanceType == PlayerType_ePIP) {
		player = sysManager.obtainPipPlayer(0);
	} else {
		player = sysManager.obtainMosaicPlayer(mActualPlayer);
	}
	if(player){
		player->m_VideoDisplaytop = m_VideoArea.m_x;
		player->m_VideoDisplayleft = m_VideoArea.m_y;
		player->m_VideoDisplaywidth = m_VideoArea.m_w;
		player->m_VideoDisplayheight = m_VideoArea.m_h;
		player->mDisplayMode = m_eVideoMode;
		if(mPlayerInstanceType == PlayerType_eMosaic){
			player->close(0);
			player->play(0);
			return 0;
		}
		player->refreshVideoDisplay();
	}
	if(mPlayerInstanceType == PlayerType_eMain)
		sysManager.releaseMainPlayer(player);
	else if(mPlayerInstanceType == PlayerType_ePIP)
		sysManager.releasePipPlayer(player, 0);

	return 0;
}

int
BrowserPlayerC20::stop()
{
	SystemManager &sysManager = systemManager();
	UltraPlayer *player = NULL;

	BROWSER_LOG("BrowserPlayerC20::stop mPlayerInstanceType(%d)\n", mPlayerInstanceType);
	if(mPlayerInstanceType == PlayerType_eMain){
		player = sysManager.obtainMainPlayer();
	} else if(mPlayerInstanceType == PlayerType_ePIP) {
		player = sysManager.obtainPipPlayer(0);
	} else {
		player = sysManager.obtainMosaicPlayer(mActualPlayer);
	}
      BROWSER_LOG("BrowserPlayerC20::stop by player(%p)\n", player);
    if (player && player->magicNumber() == mPlayerMagic) {
        player->close(UltraPlayer::BlackScreenMode);
       	if(mPlayerInstanceType == PlayerType_eMain){
	        sysManager.releaseMainPlayer(player);
	        sysManager.detachMainPlayer(player);
		}else if(mPlayerInstanceType == PlayerType_ePIP){
			sysManager.releasePipPlayer(player, 0);
			sysManager.detachPipPlayer(player, 0);
		} else {
			sysManager.detachPipPlayer(player, 1);
                    mActualPlayer = NULL;
		}
    }
    else{
       	if(mPlayerInstanceType == PlayerType_eMain){
	        sysManager.releaseMainPlayer(player);
		}else if(mPlayerInstanceType == PlayerType_ePIP){
			sysManager.releasePipPlayer(player, 0);
		}else{
                    sysManager.releasePipPlayer(player, 1);
                    mActualPlayer = NULL;
        }
    }
    return 0;
}
int
BrowserPlayerC20::setProperty(player_property_type_e aType, HPlayerProperty& aValue)
{
    SystemManager &sysManager = systemManager();
    UltraPlayer *player = NULL;

    if(mPlayerInstanceType == PlayerType_eMain){
    	player = sysManager.obtainMainPlayer();
    } else if(mPlayerInstanceType == PlayerType_ePIP){
    	player = sysManager.obtainPipPlayer(0);
    } else {
    	player = sysManager.obtainMosaicPlayer(mActualPlayer);
    }

    BROWSER_LOG("BrowserPlayerC20::setProperty aType(%s)\n", getProertyString(aType));
    if(aType != aValue.m_eType){
        BROWSER_LOG_WARNING("BrowserPlayerC20::setProperty why not same.aType(%d), m_Type(%d)\n", aType, aValue.m_eType );
    }

    switch( aType ){
        case PlayerPropertyType_eSingleOrPlaylistMode:{
            mPlaylistMode = aValue.m_Value.m_intVal;
            break;
        }
        case PlayerPropertyType_eCycleFlag:{
            mPlaylistCycleFlag = aValue.m_Value.m_intVal;
            break;
        }
        case PlayerPropertyType_eRandomFlag:{
            mPlaylistRandomFlag = aValue.m_Value.m_intVal;
            break;
        }
        case PlayerPropertyType_eVideoDisplayArea:{
            m_VideoArea = aValue.m_Value.rect;
	          break;
        }
        case PlayerPropertyType_eVideoAlpha:{
            break;
        }
        case PlayerPropertyType_eMuteFlag:{
            if(player){
            	if(mPlayerInstanceType == PlayerType_eMosaic){
            		((UltraPlayerMultipleC20 *)player)->SetMosaicMute(aValue.m_Value.m_intVal);
            	} else {
                	player->SetMute(aValue.m_Value.m_intVal);
                }
            } else {
                if(mPlayerInstanceType == PlayerType_eMain){
                    int mMuteFlag = aValue.m_Value.m_intVal;

                    appSettingSetInt("mute", mMuteFlag);
                }
            }
            break;
        }
        case PlayerPropertyType_eAudioVolume:{
            if(player){
                player->SetVolume(aValue.m_Value.m_intVal);
                BROWSER_LOG("Volume= 0x%x\n", aValue.m_Value.m_intVal);
            } else {
                if(mid_audio_volume_set(aValue.m_Value.m_intVal) != 0)
                    BROWSER_LOG_ERROR("SET volume ERROR.\n");
                if(appSettingSetInt("volume", aValue.m_Value.m_intVal) != 0)
                    BROWSER_LOG_ERROR("Write volume flash ERROR.\n");
            }
            break;
        }
        case PlayerPropertyType_eAudioChannel:{
            if(player){
            	BROWSER_LOG("BrowserPlayerC20::setProperty PlayerPropertyType_eAudioChannel(%s)", aValue.m_strVal.c_str());
 				player->SwitchAudioChannel(aValue.m_strVal);
            }
            break;
        }
        case PlayerPropertyType_eAudioTrackPID:{
            if(player){
            	if(aValue.m_Value.m_intVal == -1){
                	player->SwitchAudioTrack();
                } else {
                	player->SelectAudioTrack(aValue.m_Value.m_intVal);
					BROWSER_LOG("BrowserPlayerC20::setProperty  PlayerPropertyType_eAudioTrackPID(0x%x)\n", aValue.m_Value.m_intVal);
                }
            }
            break;
        }
        case PlayerPropertyType_eSubtitlePID:{
        	if(player){
        		if(aValue.m_Value.m_intVal == -1){
        			player->SwitchSubtitle();
        		} else {
        			player->SelectSubtitle(aValue.m_Value.m_intVal);
        		}
        	}
            break;
        }
        case PlayerPropertyType_eTeletext:{
        	if(player){
        		player->selectTeletext(aValue.m_Value.m_intVal);
        	}
        	break;
        }
        case PlayerPropertyType_eSubtitleFlag:{
        	if(player){
        		player->SetSubtitileFlag(aValue.m_Value.m_intVal);
        	} else {
        		mSubtitleFlag = aValue.m_Value.m_intVal;
            }
            break;
        }
        case PlayerPropertyType_eMacrovisionFlag:{
			mMacrovisionFlag = aValue.m_Value.m_intVal;
			break;
        }
        case PlayerPropertyType_eHDCPFlag:{
			mHDCPFlag = aValue.m_Value.m_intVal;
			break;
        }
        case PlayerPropertyType_eCGMSAFlag:{
			mCGSMAFlag = aValue.m_Value.m_intVal;
        }
        default:{
            BrowserPlayer::setProperty(aType, aValue);
            break;
        }
    }
    if(mPlayerInstanceType == PlayerType_eMain){
    	sysManager.releaseMainPlayer(player);
  	} else if(mPlayerInstanceType == PlayerType_ePIP) {
  		sysManager.releasePipPlayer(player, 0);
  	}
    return 0;
}


int
BrowserPlayerC20::getProperty(player_property_type_e aType, HPlayerProperty& aResult)
{
    SystemManager &sysManager = systemManager();
    UltraPlayer *player = NULL;

    if(mPlayerInstanceType == PlayerType_eMain){
    	player = sysManager.obtainMainPlayer();
    } else if(mPlayerInstanceType == PlayerType_ePIP){
    	player = sysManager.obtainPipPlayer(0);
    } else {
    	player = sysManager.obtainMosaicPlayer(mActualPlayer);
    }

    BROWSER_LOG("BrowserPlayerC20::getProperty aType(%s)\n", getProertyString(aType));
    switch(aType){
        case PlayerPropertyType_eCurrentPlayTime:{
            GetCurTime(aResult.m_PlayTime.m_strTimeStamp);
            aResult.m_PlayTime.m_TimeFormat = TimeType_eNPT;
            BROWSER_LOG("BrowserPlayerC20::getProperty currentPlayTime(%s)\n", aResult.m_PlayTime.m_strTimeStamp.c_str());
            break;
        }
        case PlayerPropertyType_eCurrentMediaDuration:{
            aResult.m_PlayTime.m_strTimeStamp = GetDuration();
            aResult.m_PlayTime.m_TimeFormat = TimeType_eNPT;
            break;
        }
        case PlayerPropertyType_eCurrentMediaCode:{
        	if(player){
        		aResult.m_strVal = player->getMediaCode();
        		BROWSER_LOG("BrowserPlayerC20::getProperty MediaCode(%s)\n", aResult.m_strVal.c_str());
        	}
        	break;
        }
        case PlayerPropertyType_eCurrentPlayBackMode: {
        	if(player){
        		player->GetPlayBackMode(aResult.m_strVal);
                BROWSER_LOG("BrowserPlayerC20::getProperty PlayBackMode(%s)\n", aResult.m_strVal.c_str());
        	}
        	break;
        }
        case PlayerPropertyType_eCurrentPlayChnlNum: {
        	if(player){
        		aResult.m_Value.m_intVal = player->GetCurrentChannelNum();
        		BROWSER_LOG("BrowserPlayerC20::getProperty CurrentChannelNum(%d)", aResult.m_Value.m_intVal);
        	}
        }
        case PlayerPropertyType_eMuteFlag:{
            if(player){
            	if(mPlayerInstanceType == PlayerType_eMosaic){
                	aResult.m_Value.m_intVal = ((UltraPlayerMultipleC20 *)player)->GetMosaicMute();
	            } else {
	            	aResult.m_Value.m_intVal = player->GetMute();
	            }
            }else{
                if(mPlayerInstanceType == PlayerType_eMain){
                    appSettingGetInt("mute", &aResult.m_Value.m_intVal, 0);
                    }
                }
            break;
        }
        case PlayerPropertyType_eAudioVolume:{
            if(player){
                aResult.m_Value.m_intVal = player->GetVolume();
                BROWSER_LOG("Volume= 0x%x\n", aResult.m_Value.m_intVal);
            }else
                appSettingGetInt("volume", &aResult.m_Value.m_intVal, 0);
            break;
        }
        case PlayerPropertyType_eSubtitleFlag:{
        	if(player){
        		aResult.m_Value.m_intVal = player->GetSubtitileFlag();
        	}
        	break;
        }
        case PlayerPropertyType_eAudioTrackInfo:{
        	if(player){
        		player->GetCurrentAudioTrackInfo(aResult.m_strVal);
				BROWSER_LOG("BrowserPlayerC20::getProperty CurrentAudioTrackInfo(%s)\n",aResult.m_strVal.c_str());
        	}
        	break;
        }
        case PlayerPropertyType_eSubtitleInfo:{
        	if(player){
        		player->GetCurrentSubtitleInfo(aResult.m_strVal);
        	}
        	break;
        }
        default:{
            BrowserPlayer::getProperty(aType, aResult);
            break;
        }
    }
    if(mPlayerInstanceType == PlayerType_eMain){
    	sysManager.releaseMainPlayer(player);
  	} else if(mPlayerInstanceType == PlayerType_ePIP) {
  		sysManager.releasePipPlayer(player, 0);
  	}
    return 0;
}

const char *
BrowserPlayerC20::get(const char * ioStr)
{
    SystemManager &sysManager = systemManager();
    UltraPlayer *player = NULL;
    if(mPlayerInstanceType == PlayerType_eMain){
    	player = sysManager.obtainMainPlayer();
    } else if(mPlayerInstanceType == PlayerType_ePIP){
    	player = sysManager.obtainPipPlayer(0);
    } else {
    	player = sysManager.obtainMosaicPlayer(mActualPlayer);
    }
    mChannelInfo.clear();
    if(player){
		if(!strcmp(ioStr, "getAllTeletextInfo")){
			player->GetAllTeletextInfo(mChannelInfo);
		} else if(!strcmp(ioStr, "getAllAudioTrackInfo")) {
			player->GetAllAudioTrackInfo(mChannelInfo);
		} else if(!strcmp(ioStr, "getAllSubtitleInfo")){
			player->GetAllSubtitleInfo(mChannelInfo);
		} else {
			BROWSER_LOG("BrowserPlayerC20::get ioStr(%s)\n", ioStr);
		}
	}
    if(mPlayerInstanceType == PlayerType_eMain){
    	sysManager.releaseMainPlayer(player);
  	} else if(mPlayerInstanceType == PlayerType_ePIP) {
  		sysManager.releasePipPlayer(player, 0);
  	}
	return mChannelInfo.c_str();
}

int
BrowserPlayerC20::setSingleMedia(media_info_type_e, const char *mediaString)
{
    Program *program = programParser().parseSingleMedia(mediaString);

    if(program == 0){
        return -1;
    }
    if(mProgramToPlay != NULL && mProgramToPlay->getType() == Program::PT_CHANNEL){
    	mProgramToPlay = NULL;
    } else {
   		mProgramToPlay->safeUnref();
   	}
    mProgramToPlay = program;
    return 0;
}

} // namespace Hippo
