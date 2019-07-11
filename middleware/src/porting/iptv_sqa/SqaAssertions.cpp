#include "SqaAssertions.h"
#include "LogModule.h"

static int s_SqaModuleFlag = 0;
int gSqaModuleLevel = LOG_LEVEL_WARNING; // LOG_LEVEL_NORMAL

namespace Hippo {

static LogModule StreamModule("sqa", s_SqaModuleFlag, gSqaModuleLevel);

} // namespace Hippo

