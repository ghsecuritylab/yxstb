
#include "JseHWBluetooth.h"
#include "JseFunctionCall.h"

#include "bt_parse.h"
#include "SysSetting.h"

#include <stdio.h>

static int JseBluetoothDeviceNameRead( const char* param, char* value, int len )
{
    return sysSettingGetString("BluetoothDeviceName", value, len, 0);
}

static int JseBluetoothDeviceNameWrite( const char* param, char* value, int len )
{
    return BluetoothDeviceNameSet(value);
}

static int JseBluetoothPINRead( const char* param, char* value, int len )
{
    return sysSettingGetString("BluetoothPIN", value, len, 0);
}

static int JseBluetoothPINWrite( const char* param, char* value, int len )
{
    return BluetoothPINSet(value);
}

static int JseOpenDeviceWrite( const char* param, char* value, int len )
{
    BluetoothDeviceOpen();
    return 0;
}

static int JseCloseDeviceWrite( const char* param, char* value, int len )
{
    BluetoothDeviceClose();
    return 0;
}

static int JseBluetoothStateRead( const char* param, char* value, int len )
{
    snprintf(value, len, "%d", BluetoothStateflagGet());
    return 0;
}

JseHWBluetooth::JseHWBluetooth()
    : JseGroupCall("Bluetooth")
{
    JseCall* call;

    call = new JseFunctionCall("DeviceName", JseBluetoothDeviceNameRead, JseBluetoothDeviceNameWrite);
    regist(call->name(), call);

    call = new JseFunctionCall("PIN", JseBluetoothPINRead, JseBluetoothPINWrite);
    regist(call->name(), call);

    call = new JseFunctionCall("OpenDevice", 0, JseOpenDeviceWrite);
    regist(call->name(), call);

    call = new JseFunctionCall("CloseDevice", 0, JseCloseDeviceWrite);
    regist(call->name(), call);

    call = new JseFunctionCall("State", JseBluetoothStateRead, 0);
    regist(call->name(), call);
}

JseHWBluetooth::~JseHWBluetooth()
{
}

