#include "StreamAssertions.h"
#include "LogModule.h"

static int s_StreamModuleFlag = 0;
int gStreamModuleLevel = LOG_LEVEL_WARNING; // LOG_LEVEL_NORMAL

namespace Hippo {

static LogModule StreamModule("rtsp", s_StreamModuleFlag, gStreamModuleLevel);

} // namespace Hippo

