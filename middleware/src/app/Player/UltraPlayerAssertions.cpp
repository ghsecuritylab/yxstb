
#include "UltraPlayerAssertions.h"

#include "LogModule.h"


static int s_ultraPlayerModuleFlag = 0;
int g_ultraPlayerModuleLevel = LOG_LEVEL_WARNING;

namespace Hippo {

static LogModule UltraPlayerModule("player", s_ultraPlayerModuleFlag, g_ultraPlayerModuleLevel);

} // namespace Hippo
