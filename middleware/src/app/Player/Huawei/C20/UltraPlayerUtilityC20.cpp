
#include "UltraPlayerUtility.h"

#include "UltraPlayerVodC20.h"
#include "UltraPlayerMultipleC20.h"
#ifdef INCLUDE_DVBS
#include "UltraPlayerDvb.h"
#include "ProgramChannelDvb.h"
#endif
#include "ProgramVOD.h"
#include "ProgramChannelC20.h"
#include "UltraPlayerWebPage.h"

namespace Hippo {

UltraPlayer *
UltraPlayerUtility::createPlayerByProgram(UltraPlayerClient *client, BrowserPlayerReporter *reporter, Program *program)
{
    Program::ProgramType type = program->getType();

    if(type == Program::PT_CHANNEL){
    	if(((ProgramChannel *)program)->GetChanType() == ProgramChannel::PCT_WEBPAGE){
            return new UltraPlayerWebPage(client, reporter, (ProgramChannel *)program);
        } else {
        	if(((ProgramChannel *)program)->GetChanDomain() != 1){
         		return new UltraPlayerMultipleC20(client, reporter, (ProgramChannel *)program);
         	}
#ifdef INCLUDE_DVBS
            else {
         		return new UltraPlayerDvb(client, reporter, (ProgramChannelDvb *)program);
         	}
#endif
        }
    }
    else if (type == Program::PT_CHANNEL_NUMBER) {
        ; // ??
    }
    else if (type == Program::PT_VOD) {
        return new UltraPlayerVodC20(client, reporter, (ProgramVOD *)program);
    }
    else if (type == Program::PT_ADV) {
        ;
    } else if (type == Program::PT_CHANNEL_DVB){
    	//return new UltraPlayerDvb(client, reporter, (ProgramChannelDvb *)program);
    }
    return 0;
}

UltraPlayer *
UltraPlayerUtility::createPlayerByType(int)
{
    return 0;
}

} // namespace Hippo
