#ifndef _BluetoothAssertions_H_
#define _BluetoothAssertions_H_

#include "Assertions.h"

extern int g_bluetoothModuleLevel;

#ifdef __cplusplus
namespace Hippo {
#define BLUETOOTH_LOG(args...) LOG(HLG_OPERATION, g_bluetoothModuleLevel, args)
} // namespace Hippo

#endif // __plusplus

#endif // ifndef _BluetoothAssertions_H_

