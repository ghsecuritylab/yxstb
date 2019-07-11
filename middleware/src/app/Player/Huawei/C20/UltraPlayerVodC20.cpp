
#include "UltraPlayerVodC20.h"
#include "UltraPlayerAssertions.h"


namespace Hippo {

UltraPlayerVodC20::UltraPlayerVodC20(UltraPlayerClient *client, BrowserPlayerReporter *reporter, ProgramVOD *program)
	: UltraPlayerVodHuawei(client, reporter, program)
{
}

UltraPlayerVodC20::~UltraPlayerVodC20()
{
}

int
UltraPlayerVodC20::seekEnd() 
{ 
    PLAYER_LOG("UltraPlayerVodC20 seek end\n"); 
    mid_stream_seek(mIndex, UltraPlayerVodHuawei::getTotalTime() - 3); 
    return 0; 
}

void 
UltraPlayerVodC20::handleMessage(Message *msg)
{
    PLAYER_LOG("UltraPlayerVodC20 receive message what(0x%x) info(0x%x & 0x%x)\n", msg->what, msg->arg1, msg->arg2);
    if (msg->what == MessageType_Timer && msg->arg1 == 0x100) {
        return UltraPlayerVodHuawei::handleMessage(msg);
    }    
    if(msg->what >= MessageType_ConfigSave && msg->what <= MessageType_ClearAllIcon){
        return UltraPlayer::handleMessage(msg);
    }
    if(msg->arg1 >= 0x1000){
        switch(msg->arg1 - 0x1000){
            case STRM_STATE_PAUSE:{ 
                break;
            }
            case STRM_STATE_PLAY:{ 
                break;
            }
            case STRM_STATE_FAST:{
                switch(msg->arg2){
                    case 2:{ 
                        break;
                    }
                    case 4:{ 
                        break;
                    }
                    case 8:{ 
                        break;
                    }
                    case 16:{ 
                        break;
                    }
                    case 32:{ 
                        break;
                    }
                    case -2:{ 
                        break;
                    }
                    case -4:{ 
                        break;
                    }
                    case -8:{ 
                        break;
                    }
                    case -16:{ 
                        break;
                    }
                    case -32:{ 
                        break;
                    }
                    default:{
                        break;
                    }        
                }
                break;
            }
            case STRM_STATE_CLOSE:{
                break;
            }
            default:{
                break;
            }
        }
        mCurrentStatus = msg->arg1 - 0x1000;
        mCurrentSpeed = msg->arg2;
	
        reporter()->reportState((STRM_STATE)mCurrentStatus, mCurrentSpeed);
    }
    else{
        switch(msg->arg1){
            case STRM_MSG_STREAM_VIEW:{
                break;
            }
            case STRM_MSG_STREAM_END:{
                if(!mPlayCycleFlag){
                    close(LastFrameMode);
                    play(0);
                    return ;
                }
                break;
            }
            case STRM_MSG_ADVERTISE_BEGIN:{
                break;
            }
            default:{
                break;
            }
        }
        reporter()->reportMessage((STRM_MSG)msg->arg1, msg->arg2);
    }
}


} // namespace Hippo
