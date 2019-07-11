#include "SqmAssertions.h"
#include "LogModule.h"

static int s_SqmModuleFlag = 0;
int gSqmModuleLevel = LOG_LEVEL_WARNING; // LOG_LEVEL_NORMAL

namespace Hippo {

static LogModule StreamModule("sqm", s_SqmModuleFlag, gSqmModuleLevel);

} // namespace Hippo

