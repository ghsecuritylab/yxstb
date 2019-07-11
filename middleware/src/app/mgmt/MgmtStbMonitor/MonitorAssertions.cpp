#include "MonitorAssertions.h"

#include "LogModule.h"


static int s_monitorModuleFlag = 0;
int g_monitorModuleLevel = LOG_LEVEL_WARNING;

namespace Hippo {

static LogModule MonitorModule("monitor", s_monitorModuleFlag, g_monitorModuleLevel);

} // namespace Hippo

