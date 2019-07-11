/**
 * @brief    SuperUSBHID v1.0.1 �汾�������ݲɼ�ģ��ͷ�ļ�
 * @author   ������
 * @version  1.0.0
 * @date     2010.10.11
 */

#ifndef VIDEO_CLOUD_SUPERUSBHID_ACQ
#define VIDEO_CLOUD_SUPERUSBHID_ACQ

#ifdef WIN32

#ifdef SUPERUSBHID_SDK_EXPORTS
#define SUPERUSBHID_API __declspec(dllexport)
#else
#define SUPERUSBHID_API __declspec(dllimport)
#if defined USING_DXINPUT
#ifdef _DEBUG
#pragma comment(lib, "superUSBHIDD.lib")
#else
#pragma comment(lib, "superUSBHID.lib")
#endif
#else
#error "Please define macro USING_DXINPUT to determinate using which dll"
#endif
#endif

#ifdef __cplusplus
#define DEFINE_SUPERUSBHID_API extern "C" SUPERUSBHID_API
#else
#define DEFINE_SUPERUSBHID_API SUPERUSBHID_API
#endif

#else
#define DEFINE_SUPERUSBHID_API
#endif

#include "cloud_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief  �����豸����ʱ�Ļص�����
 * @param    device_info_buffer ������Ϣ����������ʽ��count(4),{dev_id(4),handle(4),rpt_desc_len(2),rpt_desc(rpt_desc_len)}��
 * @param    device_info_len    ������Ϣ����������
 */
typedef void (*ConnectCallbackFunc)( 
                                IN C_U8  * device_info_buffer,
                                IN C_U16   device_info_len );

/**
 * @brief  ���豸�γ�ʱ�Ļص�����
 * @param    device_info_buffer ������Ϣ����������ʽ��count(4),{dev_id(4),handle(4)}��
 * @param    device_info_len    ������Ϣ����������
 */
typedef void (*DisconnectCallbackFunc)( 
                                IN C_U8  * device_info_buffer,
                                IN C_U16   device_info_len );

/**
 * @brief  ���յ��豸���������뱨��ʱ�Ļص�����
 * @param    input_buffer ���뱨�滺��������ʽ��count(4),{handle(4),rpt_len(2),rpt_data(rpt_len)}��
 * @param    input_len    ���뱨�滺��������
 */
typedef void (*InputReportCallbackFunc)( 
                                IN C_U8  * input_buffer,
                                IN C_U16   report_len );

/**
 * @brief  ���յ�FlingPC�豸�������������ݰ�ʱ�Ļص�����
 * @param    packet_size    FlingPC���ݰ���С
 * @param    packet_count   �������ݰ�����
 * @param    packet_buffer  �������ݻ�����
 */
typedef void (*FlingpcInputCallbackFunc)( 
                                IN C_U16   packet_size,
                                IN C_U16   packet_count,
                                IN C_U8  * packet_buffer );

/**
 * @brief  ��ȡ��ǰUSB�ɼ���汾��
 * @return ���ص�ǰUSB�ɼ���汾��
 */
char* USBHIDACQ_Version();

/**
 * @brief  ���õ����豸����ʱ�Ļص�����
 * @param    callback �ص���������ϵͳ�������豸����ʱ����
 */
DEFINE_SUPERUSBHID_API
void USBHIDACQ_SetConnectCallback( IN ConnectCallbackFunc callback );

/**
 * @brief  ���õ��豸�γ�ʱ�Ļص�����
 * @param    callback �ص���������ϵͳ�����豸�γ�ʱ����
 */
DEFINE_SUPERUSBHID_API
void USBHIDACQ_SetDisconnectCallback( IN DisconnectCallbackFunc callback );

/**
 * @brief  ���õ��յ��豸���������뱨��ʱ�Ļص�����
 * @param    callback �ص���������ϵͳ�����յ��豸���������뱨��ʱ����
 */
DEFINE_SUPERUSBHID_API
void USBHIDACQ_SetInputReportCallback( IN InputReportCallbackFunc callback );

/**
 * @brief  ���õ��յ�FlingPC�豸�������������ݰ�ʱ�Ļص�����
 * @param    callback �ص���������ϵͳ�����յ�FlingPC�豸�������������ݰ�ʱ����
 */
DEFINE_SUPERUSBHID_API
void USBHIDACQ_SetFlingpcInputCallback( IN FlingpcInputCallbackFunc callback );

/**
 * @brief  ӳ����ڲ����ã�֪ͨOS�ײ�һ��USBHID������������ˣ��ɼ�����ʵ�֣�
 * @param    handle        �豸���
 * @param    output_buffer ������滺����
 * @param    output_len    ������泤��
 */
DEFINE_SUPERUSBHID_API
void USBHIDACQ_OutputReport( IN C_U32  handle,
                             IN C_U8     *output_buffer,
                             IN C_U16     output_len );

/**
 * @brief  ӳ����ڲ����ã�֪ͨOS�ײ�һ����һ��FlingPC������������ˣ��ɼ�����ʵ�֣�
 * @param    packet_size    FlingPC���ݰ���С
 * @param    packet_count   ������ݰ�����
 * @param    packet_buffer  ������ݻ�����
 */
DEFINE_SUPERUSBHID_API
void USBHIDACQ_FlingpcOutput( IN C_U16  packet_size,
                              IN C_U16  packet_count,
                              IN C_U8  *packet_buffer );

/**
 * @brief  ��ʼ�������ݲɼ�
 * @return ���ز����Ƿ�ɹ�
 */
DEFINE_SUPERUSBHID_API
C_BOOL USBHIDACQ_Start( void );

/**
 * @brief  ֹͣ�������ݲɼ�
 */
DEFINE_SUPERUSBHID_API
void USBHIDACQ_Stop( void );

/**
 * @brief  ������������
 */
DEFINE_SUPERUSBHID_API
void USBHIDACQ_SetAttachHWND(void* hwnd );

/**
 * @brief  ����ֹͣ�ɼ������ص�����
 */
DEFINE_SUPERUSBHID_API
void USBHIDACQ_SetStopMappingCallBackFunc(void (*callback)());

#ifdef __cplusplus
}
#endif

#endif // end of VIDEO_CLOUD_SUPERUSBHID_ACQ
