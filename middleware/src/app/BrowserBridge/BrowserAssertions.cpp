
#include "BrowserAssertions.h"

#include "LogModule.h"


static int s_browserModuleFlag = 0;
int g_browserModuleLevel = LOG_LEVEL_ERROR;

namespace Hippo {

static LogModule BrowserModule("browser", s_browserModuleFlag, g_browserModuleLevel);

} // namespace Hippo
