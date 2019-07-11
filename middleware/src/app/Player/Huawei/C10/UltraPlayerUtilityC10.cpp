
#include "UltraPlayerUtility.h"
#include "UltraPlayerAssertions.h"
#include "UltraPlayerVodC10.h"
#include "UltraPlayerMultipleC10.h"
#include "UltraPlayerWebPage.h"

#include "ProgramVOD.h"
#include "ProgramChannelC10.h"


namespace Hippo {

UltraPlayer *
UltraPlayerUtility::createPlayerByProgram(UltraPlayerClient *client, BrowserPlayerReporter *reporter, Program *program)
{
    Program::ProgramType type = program->getType();
    
    PLAYER_LOG("CreatePlayerByProgram media type %d\n", type);
    if(type == Program::PT_CHANNEL){
        ProgramChannel *channel = (ProgramChannel *)program;
        if(channel->GetChanType() == ProgramChannel::PCT_WEBPAGE){
            return new UltraPlayerWebPage(client, reporter, channel);
        }
        else{
            return new UltraPlayerMultipleC10(client, reporter, channel);
        }
    }
    else if (type == Program::PT_CHANNEL_NUMBER) {
        ; // ??
    }
    else if (type == Program::PT_VOD) {
        return new UltraPlayerVodC10(client, reporter, (ProgramVOD *)program);
    }
    else if (type == Program::PT_ADV) {
        ;
    }
    return 0;
}

UltraPlayer *
UltraPlayerUtility::createPlayerByType(int)
{
    return 0;
}

} // namespace Hippo
