#include "JseGlueAssertions.h"
#include "LogModule.h"

static int s_JseGlueModuleFlag = 0;
int gJseGlueModuleLevel = LOG_LEVEL_WARNING; //LOG_LEVEL_NORMAL

namespace Hippo {

static LogModule JseGlueModule("jseglue", s_JseGlueModuleFlag, gJseGlueModuleLevel);

} // namespace Hippo
