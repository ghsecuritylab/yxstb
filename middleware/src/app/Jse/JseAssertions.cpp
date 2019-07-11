#include "JseAssertions.h"
#include "LogModule.h"

static int s_JseModuleFlag = 0;
int gJseModuleLevel = LOG_LEVEL_WARNING;

static LogModule JseModule("JseCall", s_JseModuleFlag, gJseModuleLevel);


