
#ifndef __TR069_PORT_ALARM_H__
#define __TR069_PORT_ALARM_H__

#include "tmerTask.h"

enum {
    ALARM_TYPE_STB_Upgrade = 0,//1. ����������ʧ��
    ALARM_TYPE_Disk_Write,//2. ���̶�дʧ��
    ALARM_TYPE_CPU_Used,//3. CPUʹ�ðٷֱȳ�����ֵ
    ALARM_TYPE_Disk_Used,//4. ����ʹ�ÿռ�ٷֱȳ�����ֵ
    ALARM_TYPE_Memory_Used,//5. �ڴ�ʹ�ðٷֱȳ�����ֵ
    ALARM_TYPE_Dropped,//6. �����ʳ�����ֵ
    ALARM_TYPE_Authorize,//7. IPTVҵ����֤ʧ��
    ALARM_TYPE_Channel,//8. ����Ƶ��ʧ�ܸ澯
    ALARM_TYPE_Decode,//9. �����н������

    ALARM_TYPE_Decrypt,//10.�����н���ʧ��
    ALARM_TYPE_Dropframe,//11.��֡�ʳ�����ֵ
    ALARM_TYPE_Timelapse,//12.ʱ�ӳ�����ֵ
    ALARM_TYPE_Cushion,      //13.�������(����/����)
    ALARM_TYPE_EPG_Access, //14.����EPG ������ʧ��
    ALARM_TYPE_Media_Access, //15.����ý�������ʧ��
    ALARM_TYPE_File_Access,  //16.�ļ�����������ʧ��
    ALARM_TYPE_Media_Format, //17.ý���ʽ��֧��

    ALARM_TYPE_MAX
};

enum {
    ALARM_LEVEL_NONE = 0,
    ALARM_LEVEL_CRITICAL,//����
    ALARM_LEVEL_MAJOR,//��Ҫ
    ALARM_LEVEL_MINOR,//��Ҫ
    ALARM_LEVEL_WARNING,//��ʾ
    ALARM_LEVEL_INDETERMINATE,//��ȷ��
    ALARM_LEVEL_CLEARED,
    ALARM_LEVEL_MAX
};

enum {
	ALARM_TYPE_DEVICE = 0,
	ALARM_TYPE_THRESHOULD,
	ALARM_TYPE_CONDITION,
	ALARM_TYPE_NETWORK,
	ALARM_TYPE_SYSERROR
};

#define SERIAL_SAMPLING_CYC         3

#ifdef Sichuan
#define ALARM_CPU_CODE              2000
#define ALARM_MEM_CODE              2001
#define ALARM_DISK_FULL_CODE        2002
#define ALARM_DECODE_CODE           2003
#define ALARM_DECRYPT_CODE          2004
#define ALARM_CUSHION_CODE          2005
#define ALARM_DROP_PACKAGE_CODE     2050
#define ALARM_DROP_FRAME_CODE       2060
#define ALARM_TIME_LAPSE_CODE       2070
#define ALARM_EPG_ACCESS_CODE       3000
#define ALARM_MEDIA_ACCESS_CODE     3001
#define ALARM_FILE_ACCESS_CODE      3002
#define ALARM_MEDIA_FORMAT_CODE     3003
#else
#define ALARM_CPU_CODE              100103
#define ALARM_MEM_CODE              100105
#define ALARM_DISK_FULL_CODE        100104
#define ALARM_DISK_WR_CODE          100102
#define ALARM_DROP_PACKAGE_CODE     100106
#define ALARM_DECODE_CODE           100108
#define ALARM_CHANNEL_CODE          300104
#define ALARM_UPGRAGE_CODE          300101
#define ALARM_AUTHORIZE_CODE        300103
#endif

#ifdef __cplusplus
extern "C" {
#endif

void tr069_port_alarmInit(void);


void alarmStrdup(char **, char *);
int alarmPermille(unsigned long long, unsigned long long);

void alarmTimerRegist(unsigned int sec, OnTimer onTimer, int arg);
void alarmTimerUnRegist(OnTimer onTimer, int arg);

void tr069_port_alarmCPU(int, int, int);
void tr069_port_alarmDisk(int, int, int);
void tr069_port_alarmMemory(int, int, int);

void tr069_port_memoryStatusInit(void);
void tr069_port_processStatusInit(void);

#ifdef __cplusplus
}
#endif

#endif //__TR069_PORT_ALARM_H__
