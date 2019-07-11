#include "NetworkAssertions.h"
#include "LogModule.h"

static int s_NetworkModuleFlag = 0;
int gNetworkModuleLevel = LOG_LEVEL_WARNING;

namespace Hippo {

static LogModule NetworkModule("network", s_NetworkModuleFlag, gNetworkModuleLevel);

} // namespace Hippo

