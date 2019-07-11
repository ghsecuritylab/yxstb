
#include "BrowserPlayerReporterUtility.h"

#include "Program.h"
#include "BrowserPlayerReporterMultipleC20.h"
#include "BrowserPlayerReporterVodC20.h"

namespace Hippo {

BrowserPlayerReporter *BrowserPlayerReporterCreate(Program *program)
{
    if (program == NULL)
        return NULL;
	if (program->getType() == Program::PT_CHANNEL){
		return new BrowserPlayerReporterMultipleC20();
	}else if (program->getType() == Program::PT_VOD){
        return new BrowserPlayerReporterVodC20();
    }else if(program->getType() == Program::PT_CHANNEL_DVB){
    	return new BrowserPlayerReporterMultipleC20();
    } else {
        return NULL;
	}
}

} // namespace Hippo
