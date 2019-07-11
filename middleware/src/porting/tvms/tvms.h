#ifndef __TVMS_H__
#define __TVMS_H__

typedef struct tvms_msg_t
{
	struct 	tvms_msg_t *next;
	struct 	tvms_msg_t *prev;
	int                  showstatus; //-1 默认0 success1 delay2 noshow3nofile
	int 			showflag; //0默认，1表示展示
	//VOD模式下，这个值是逝去时间，但是不启动定时器
	unsigned int 	pass_time;//用来记录当前发送时间到某个特定时间的秒数
	char 			msgTaskCode[65]; //下发的消息任务Code，全局唯一 
	int  			taskType; //任务的类型 0：通知任务;1：LiveTV任务;2：VOD任务;3：开机任?
	int				priority; //消息任务的优先级别：0-10， 10最高
	int  			confirmFlag; //确保用户是否收到到 1：确保收到（默认）;0：不确保
	char   		origsendTime[15]; //拼接URL时候，使用，最原始的原始时间
	char 			sendTime[15]; //单位是S,确保用户是否收到的消息重发时间段
	int   		resendTime; //绝对时间,消息有效期。等于send_time+resend_time。如果文广提供的消息XML文件中resnd-time没有，则valid_time默认为2100年1月1日零时。
	char 			validTime[15];
	char	 		mediaCode[129]; //指定MediaData与内容的关联信息，任务类型为：VOD：指定VOD的MediaCode;LiveTV：指定LiveTV的mediaCode	 
	int  			mode; //消息模式0：滚动字幕模式；1：简单页面模式；2：交互页面模式
	char 		title[51]; //消息标题
	char 			*displayURL ; //消息页面URL地址
	int  			showDirect; //是否直接强制显示各页面，如果为否，STB可以显示提示信息后，用户按了某个按键后，才弹出该消息页面
	//char 		*reserve ; //保留字段
}TVMS;

typedef struct _TVMSLIST_
{
	TVMS 	*vodlist;
	TVMS 	*heartlist;
	TVMS 	*showlist;//用来存储发送过的消息
}TVMSLIST;

typedef struct  control_tvms_s{
	char 		msgTaskCode[65];
	char 		show_status[10]; // 0 没有处理1 success 2 noshow 3 nofile 4 delay ，默认消息展示成功
	int 		control_flag; //  1 定时器到期，2 消息展示状态
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

