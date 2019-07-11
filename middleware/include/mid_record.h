
#ifndef __MID_RECORD_H__
#define __MID_RECORD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "mid_stream.h"
#include "ind_pvr.h"

void mid_record_init(void);

void mid_record_mount(void);
void mid_record_unmount(void);

/*
	¼�Ƹ��õ�ǰ������
	Ĭ���ǿ����ģ�����Ƶ����Ϊ���������Թرոù��ܣ����������
 */
void mid_record_mix(int mix);

/*
	����Ԥ���ռ�
	������� bitrate
	����ʱ��ʱ�� second
 */
void mid_record_set_reserve(unsigned int bitrate, unsigned int second);
/*
	��ȡԤ���ռ䣬��MΪ��λ
 */
unsigned int mid_record_get_reserve(unsigned int total);


/*
	���� mid_record_suffix(".in", ".ts"); ֮��
	¼��Ŀ¼�е� info media0 �ļ��ͻ��� info.in media0.ts
 */
void mid_record_suffix(char* sinfo, char* smedia);

/*
¼�ƹ����г����쳣����������record_port_message��֪ͨ�ϲ㣬������һ��������Ϊindex��
int record_port_message(int pIndex, unsigned int id, RECORD_MSG msg, int arg);
 */

/*
	��̨¼���޶�Ϊ��·����ÿһ·�ֱ��֧��RTSP,HTTP
	��̨¼�ƺͲ����Ƿ���ģ�����Ŀǰ�ܹ���ƣ�STBͬʱ��·RTSP���ź���·RTSP¼�ƵĲ��У�4·RTSP���Բ��У�
 */
/*
	��̨��ʼ¼��
	index��
		¼��·���� 0 �� 1��ͬ����·���ŵ�index�������
	url��¼��·��
		�Ϸ��ĸ�ʽ��rtsp://xxx,igmp://xxx,http://xxx
	info_buf��
		¼�ƽ�Ŀ��Ϣ�������¼��ʱ�ı���URL��
	info_len��
		¼�ƽ�Ŀ��Ϣ���ȣ�����Ϊ8����Ϊ����¼�ơ�
	id:
		������¼��ID

		begin, end ¼����ʼʱ���¼�ƽ���ʱ�䣬����û����ȷ��Χ�ļ�ʱ¼�ƣ�Ӧ�ô�0
	����ֵ��
		¼��ID
 */
unsigned int mid_record_open(int index, char* url, APP_TYPE apptype, char *info_buf, int info_len, unsigned int id, unsigned int begin, unsigned int end);

/*
	����mid_record_open0����¼��ID��Ȼ������mid_record_open1����¼��������������mid_record_open���ù����в���¼����Ϣ����¼��ID��Ӧ��
 */
unsigned int mid_record_open0(int index, char* url, APP_TYPE apptype, char* info_buf, int info_len, unsigned int id, unsigned int begin, unsigned int end);
void mid_record_open1(int index);

/*
 �Զ���¼�� ��
 ���� info_buf��info_len, id ���� mid_record_open���������ģ����ID��Ϊ0���Զ���¼�ƿɲ���ԭ��¼��ID�Ƿ���ڣ�������ھ�׷��¼�ơ�
 ����ֵ��¼�ƶ����handle��-1��ʾ��Ч
 */
typedef int (*RecordCall_open)(char *info_buf, int info_len, unsigned int id);
/*
 �Զ���¼�� ����push
 handle �� RecordCall_open ���صġ�
 ����ֵΪʵ��д�����ͣ��ɹ��ĳ��ȡ����ɻָ��Ĵ��󷵻� -1
 */
typedef int (*RecordCall_push)(int handle, char *buf, int len);
/*
 �Զ���¼�� �ر�
 */
typedef void (*RecordCall_close)(int handle);
/*
	�Զ���¼��ע��
 */
int mid_record_call(int index, RecordCall_open rcall_open, RecordCall_push rcall_push, RecordCall_close rcall_close);

/*
	�ر�¼��
	id: Ϊ¼�ƽ������ӵĲ�������Ӧ��¼��ID��Ϊ0��ʾ�ر�¼������
 */
#define mid_record_close mid_record_close_v1
void mid_record_close(int index, unsigned int id);

/*
	̽�������ٶ�
 */
void mid_record_check_bandwidth(int index);

/*
	����¼�ƽ���ʱ��
	v1 ����һ��id����
 */
#define mid_record_set_endtime mid_record_set_endtime_v1
void mid_record_set_endtime(int index, unsigned int id, unsigned int endtime);

/*
	ɾ����¼�ƵĽ�Ŀ
 */
#define mid_record_delete mid_record_delete_v1
void mid_record_delete(unsigned int id, PvrMsgCall call);

unsigned int mid_record_get_num(void);

/*
	sizeӦ����sizeof(struct PVRInfo)
	info_buf��info_len Ϊ��ʼ¼��ʱ��mid_record_open ����ġ���Ҫ���浽¼�ƽڵ�����Զ�����Ϣ��
		info_len �ĳ�ֵӦΪinfo_buf �������ռ�
		info_len �ķ���ֵΪʵ����䵽info_buf�����ݵĳ���
		ע�⣺info_len �ĳ�ֵ��Ҳ��info_buf�������ռ� С��ʵ�ʴ����Զ�����Ϣ�ĳ���ʱ��info_len���ᱻ��Ϊ 0
	v2 ȥ������ size_t size
 */
#define mid_record_get_info	mid_record_get_info_v2
int mid_record_get_info(int index, struct PVRInfo *info, char *info_buf, int* info_len);

/*
	����¼���û���Ϣ
	ע�⣺��ID����Ϊ¼���б����Ѵ��ڵ�¼�Ƽ�¼������ᱨ��
 */
int mid_record_user_write(unsigned int id, char *buf, int len);
/*
	��ȡ¼�Ƽ�¼�����Ϣ
	����û���Ϣȷ��δ���������ò�Ҫ���øýӿڣ���������һ�������ӡ
	size ָʾbuf���峤��
	����ֵ
		С��0��ʾ��ȡ����
		���ڻ����0Ϊʵ�ʶ�ȡ����
 */
int mid_record_user_read(unsigned int id, char *buf, int size);

//���������
void mid_record_limit_bandwidth(int index, int bitrate);
/*
 percent ��ʱ����ٷֱȣ�percentΪ0��ʾǧ��֮һ
 second ��ʱ��������ʱ��
 */
void mid_record_adjust_bandwidth(int index, int percent, int second);

/*
	���ش���mid_record_open֮ǰ����
 */
void mid_record_bitrate(int index, int bitrate);

void mid_record_set_tuner(int index, int tuner);
void mid_record_set_fcc(int index, int fcc_type);
void mid_record_set_igmp(int index, MultiPlayCall play_call, MultiStopCall stop_call, void* igmp, int igmp_size);

void mid_record_virtual(int flag);

#ifdef __cplusplus
}
#endif

#endif//__MID_RECORD_H__

