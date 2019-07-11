
#include "ViewAssertions.h"

#include "LogModule.h"


static int s_viewModuleFlag = 0;
int g_viewModuleLevel = LOG_LEVEL_WARNING;

namespace Hippo {

static LogModule UltraPlayerModule("view", s_viewModuleFlag, g_viewModuleLevel);

} // namespace Hippo
