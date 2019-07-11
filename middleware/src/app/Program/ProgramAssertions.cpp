
#include "ProgramAssertions.h"

#include "LogModule.h"


static int s_programModuleFlag = 0;
int g_programModuleLevel = LOG_LEVEL_WARNING;

namespace Hippo {

static LogModule ProgramModule("program", s_programModuleFlag, g_programModuleLevel);

} // namespace Hippo
