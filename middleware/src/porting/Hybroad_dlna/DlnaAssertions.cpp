#include "DlnaAssertions.h"

#include "LogModule.h"


static int s_DlnaModuleFlag = 0;
int g_DlnaModuleLevel = LOG_LEVEL_ERROR;

//namespace Hippo {

static LogModule DlnaModule("dlna", s_DlnaModuleFlag, g_DlnaModuleLevel);

//} // namespace Hippo
