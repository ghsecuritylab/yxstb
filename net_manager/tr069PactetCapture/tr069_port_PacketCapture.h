#ifndef __tr069_port_PacketCapture_h__
#define __tr069_port_PacketCapture_h__
#ifdef __cplusplus
extern "C" {
#endif
void tr069SetPacketCaptureParamValue(char * name , char *str, int len);
void tr069GetPacketCaptureParamValue(char *name , char *str, int len);
#ifdef __cplusplus
}
#endif
#endif//__tr069_port_PacketCapture_h__

