
#include "BrowserPlayerReporterUtility.h"

#include "Program.h"
#include "BrowserPlayerReporterMultipleC10.h"
#include "BrowserPlayerReporterVodC10.h"


namespace Hippo {

BrowserPlayerReporter *BrowserPlayerReporterCreate(Program *program)
{
    if (program == NULL)
        return NULL;

    if (program->getType() == Program::PT_CHANNEL)
        return new BrowserPlayerReporterMultipleC10();
    else if (program->getType() == Program::PT_VOD)
        return new BrowserPlayerReporterVodC10();
    else
        return NULL;
}

} // namespace Hippo
