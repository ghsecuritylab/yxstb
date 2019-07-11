#ifndef _Bt_Parse_H_
#define _Bt_Parse_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __inout
#define __inout
#endif

typedef enum {
    BT_Processed = 0,
    BT_Continue = 1,
}BT_ParsingResult;

// returns -1 if BLUETOOTH cannot be supported by zebra or the type of the message is not bluetooth.
BT_ParsingResult ParseAsBluetooth(
    __inout unsigned int* msgno,
    __inout int* type,
    __inout int* stat);

int BluetoothParameterGet(char *BluetoothName, char *BluetoothPIN);
int BluetoothParameterSet(int para, char *para_data);
int BluetoothStateflagGet();
void BluetoothStateflagSet(int state);
void BluetoothDeviceOpen();
void BluetoothDeviceClose();
int BluetoothDeviceNameSet(char *str);
int BluetoothPINSet(char *str);
int BluetoothParamCheck();

#ifdef __cplusplus
}
#endif



#endif // ifndef _Bt_Parse_H_


