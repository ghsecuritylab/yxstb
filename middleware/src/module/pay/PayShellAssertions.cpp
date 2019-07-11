
#include "PayShellAssertions.h"
#include "LogModule.h"

static int s_PayShellModuleFlag = 0;
int gPayShellModuleLevel = LOG_LEVEL_WARNING;

namespace Hippo {

static LogModule PayShellModule("payshell", s_PayShellModuleFlag, gPayShellModuleLevel);

} // namespace Hippo

