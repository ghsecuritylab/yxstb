
#include "LogModuleCore.h"

#include "LogModule.h"


static int s_systemModuleFlag = 0;
int g_systemModuleLevel = LOG_LEVEL_WARNING;

static LogModule systemModule("system", s_systemModuleFlag, g_systemModuleLevel);


static int s_runningModuleFlag = 0;
int g_runningModuleLevel= LOG_LEVEL_WARNING;

static LogModule runningModule("running", s_runningModuleFlag, g_runningModuleLevel);
