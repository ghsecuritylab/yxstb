#include <stdio.h>
#include <stdlib.h>

#include "cloud_api.h"

void Device_ConnectCallBack(C_U8 *device_info_buffer, C_U16 device_info_len)
{
        C_DeviceInfo device_info;
        memset(&device_info, 0, sizeof(C_DeviceInfo));
        device_info.hid_.uDataSize = device_info_len;
        device_info.hid_.pdata = device_info_buffer;
        //memcpy(device_info.hid_.pdata, device_info_buffer,device_info_len);
		Cloud_DeviceConnect( KeyType_HID, &device_info);
}

void Device_DisConnectCallBack(C_U8 *device_info_buffer, C_U16 device_info_len)
{
        C_DeviceInfo device_info;
        memset(&device_info, 0, sizeof(C_DeviceInfo));
        device_info.hid_.uDataSize = device_info_len;
        device_info.hid_.pdata = device_info_buffer;
       // memcpy(device_info.hid_.pdata, device_info_buffer,device_info_len);
		Cloud_DeviceDisconnect(KeyType_HID, &device_info);
}

void Device_InputReportCallbackFunc(C_U8 * report_buffer, IN C_U16 report_len)
{
	C_U8 len=sizeof(C_HID);
	C_HID keyData;
	C_HID *p;
	#if 0
	C_U32 ix = 0;
	printf("USB Acq Date: ");
	for(; ix < report_len ; ix++)
	{
		printf(" %02d", *(report_buffer+ix));
	}
	printf("\n\n");
	#endif
	keyData.pdata = (C_U8 *)report_buffer;
	keyData.uDataSize = report_len;
	p = &keyData;
	Cloud_OnKey(KeyType_HID, len, (C_U8 *)p);
}

void Device_FlingpcInputCallbackFunc(C_U16 packet_size, C_U16 packet_count, C_U8* packet_buffer)
{

}

void Device_Stop(void)
{

}
void Device_Callback_Superhid(void)
{
	USBHIDACQ_SetConnectCallback(Device_ConnectCallBack);
	USBHIDACQ_SetDisconnectCallback(Device_DisConnectCallBack);
	USBHIDACQ_SetInputReportCallback(Device_InputReportCallbackFunc);
	USBHIDACQ_SetFlingpcInputCallback(Device_FlingpcInputCallbackFunc);
	USBHIDACQ_SetStopMappingCallBackFunc(Device_Stop);
}

