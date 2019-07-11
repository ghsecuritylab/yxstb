#include "TR069Assertions.h"
#include "LogModule.h"


static int s_tr069ModuleFlag = 0;
int gTr069ModuleLevel = LOG_LEVEL_WARNING;

namespace Hippo {

static LogModule Tr069Module("tr069", s_tr069ModuleFlag, gTr069ModuleLevel);

} // namespace Hippo

