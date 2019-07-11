#include "CpvrAssertions.h"
#include "LogModule.h"

static int s_CpvrModuleFlag = 0;
int gCpvrModuleLevel = LOG_LEVEL_WARNING;

namespace Hippo {

static LogModule CpvrQuickInstallModule("cpvr", s_CpvrModuleFlag, gCpvrModuleLevel);

} // namespace Hippo

