
#include "BluetoothAssertions.h"
#include "LogModule.h"

static int s_bluetoothModuleFlag = 0;
int g_bluetoothModuleLevel = LOG_LEVEL_VERBOSE;

namespace Hippo {

static LogModule BrowserModule("bluetooth", s_bluetoothModuleFlag, g_bluetoothModuleLevel);

} // namespace Hippo

