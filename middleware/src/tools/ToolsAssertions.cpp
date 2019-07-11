#include "ToolsAssertions.h"
#include "LogModule.h"

static int s_ToolsModuleFlag = 0;
int gToolsModuleLevel = LOG_LEVEL_WARNING;

namespace Hippo {

static LogModule OSToolsModule("tools", s_ToolsModuleFlag, gToolsModuleLevel);

} // namespace Hippo

