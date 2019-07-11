
#ifndef __TR069_ALARM_V2_H__
#define __TR069_ALARM_V2_H__

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

#ifdef __cplusplus
extern "C" {
#endif

int tr069_port_alarm_post(int type, char *code, int level, char *location);
void tr069_port_alarm_clear(int alarm);

#ifdef __cplusplus
}
#endif

#endif //__TR069_ALARM_H__
