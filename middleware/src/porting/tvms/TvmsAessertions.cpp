#include "TvmsAssertions.h"

#include "LogModule.h"


static int s_tvmsModuleFlag = 0;
int g_tvmsModuleLevel = LOG_LEVEL_ERROR;

namespace Hippo {

static LogModule MonitorModule("tvms", s_tvmsModuleFlag, g_tvmsModuleLevel);

} // namespace Hippo