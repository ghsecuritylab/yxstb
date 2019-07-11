#ifndef __TVMS_H__
#define __TVMS_H__

typedef struct tvms_msg_t
{
	struct 	tvms_msg_t *next;
	struct 	tvms_msg_t *prev;
	int                  showstatus; //-1 Ĭ��0 success1 delay2 noshow3nofile
	int 			showflag; //0Ĭ�ϣ�1��ʾչʾ
	//VODģʽ�£����ֵ����ȥʱ�䣬���ǲ�������ʱ��
	unsigned int 	pass_time;//������¼��ǰ����ʱ�䵽ĳ���ض�ʱ�������
	char 			msgTaskCode[65]; //�·�����Ϣ����Code��ȫ��Ψһ 
	int  			taskType; //��������� 0��֪ͨ����;1��LiveTV����;2��VOD����;3��������?
	int				priority; //��Ϣ��������ȼ���0-10�� 10���
	int  			confirmFlag; //ȷ���û��Ƿ��յ��� 1��ȷ���յ���Ĭ�ϣ�;0����ȷ��
	char   		origsendTime[15]; //ƴ��URLʱ��ʹ�ã���ԭʼ��ԭʼʱ��
	char 			sendTime[15]; //��λ��S,ȷ���û��Ƿ��յ�����Ϣ�ط�ʱ���
	int   		resendTime; //����ʱ��,��Ϣ��Ч�ڡ�����send_time+resend_time������Ĺ��ṩ����ϢXML�ļ���resnd-timeû�У���valid_timeĬ��Ϊ2100��1��1����ʱ��
	char 			validTime[15];
	char	 		mediaCode[129]; //ָ��MediaData�����ݵĹ�����Ϣ����������Ϊ��VOD��ָ��VOD��MediaCode;LiveTV��ָ��LiveTV��mediaCode	 
	int  			mode; //��Ϣģʽ0��������Ļģʽ��1����ҳ��ģʽ��2������ҳ��ģʽ
	char 		title[51]; //��Ϣ����
	char 			*displayURL ; //��Ϣҳ��URL��ַ
	int  			showDirect; //�Ƿ�ֱ��ǿ����ʾ��ҳ�棬���Ϊ��STB������ʾ��ʾ��Ϣ���û�����ĳ�������󣬲ŵ�������Ϣҳ��
	//char 		*reserve ; //�����ֶ�
}TVMS;

typedef struct _TVMSLIST_
{
	TVMS 	*vodlist;
	TVMS 	*heartlist;
	TVMS 	*showlist;//�����洢���͹�����Ϣ
}TVMSLIST;

typedef struct  control_tvms_s{
	char 		msgTaskCode[65];
	char 		show_status[10]; // 0 û�д���1 success 2 noshow 3 nofile 4 delay ��Ĭ����Ϣչʾ�ɹ�
	int 		control_flag; //  1 ��ʱ�����ڣ�2 ��Ϣչʾ״̬
}control_tvms_t;


#ifdef __cplusplus
extern "C" {
#endif

int  	init_tvms_msg(void);
void 	vod_tvmsmsg_request(void);
void 	tvms_request_loop(void);


void 	vod_tvmsmsg_clear_request(void);

void 	tvms_msg_status_deal(char * code, char * status);

void 	clear_tvms_msg(void);

void 	printf_tvms_msg(char *str,TVMS *p);
void 	tvms_time_scan_loop(void);
int search_showlist_by_msgcode( char *msgcode,char *msgtitle);

void livetv_tvmsmsg_request();
void 	livetv_tvmsmsg_clear_request(void);
void modify_delay_after_show(void);
void tvms_show_status(const char *buf);


#ifdef __cplusplus
}
#endif

#endif

