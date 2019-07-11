#ifndef _TVMS_DEFINE_H_
#define _TVMS_DEFINE_H_

/********************************************************************************
*      打印控制
*
********************************************************************************/
#include "Assertions.h"
#include "tvms.h"
#ifndef PRINTF
#define PRINTF
#endif


/********************************************************************************
*      消息队列
*
*********************************************************************************/
#include "mid/mid_msgq.h"
#define tvms_yxmsgq_t  mid_msgq_t
#define tvms_yx_msgq_create mid_msgq_create
#define	tvms_yx_msgq_delete mid_msgq_delete
#define	tvms_yx_msgq_readfd_get mid_msgq_fd



#define MIN_HEART_TIME 60  //最小的心跳时间
#define MIN_DELAY_TIME 10  //最小延时时间

int tvms_yx_msgq_put( tvms_yxmsgq_t msgq, char* pmsg );
int tvms_yx_msgq_get( tvms_yxmsgq_t msgq, char * pmsg );


int 					tvms_request_task_create(void);

int 					tvms_time_scan_task_create(void);

int 					tvms_time2string(int sec, char* buf, char insert);
unsigned int 			tvms_string2time(char *str);
int 					tvms_get_vod_playstate(void);
int 					tvms_heart_time_read(void);
int 					tvms_delay_time_read(void);
int 					tvms_heart_url_read(char *url,int len);
int 					tvms_heartvod_url_read(char *url,int len);
int 					tvms_mediacode_read(char *mediacode);
int 					tvms_userid_read(char *userid);
void					tvms_send_event(TVMS *p);
unsigned int 					tvms_get_vod_time(void);
void 					tvms_loop_init(void);

int 					tvms_get_inifile(char *url,void *func);
unsigned int 			tvms_mid_time(void);
char* 				tvms_file_get_line( char* s_str, int s_len, char* buf );
int 					tvms_line_get_bracket( char* s_str, int s_len, char* title );
int 					tvms_line_get_tag( char* str, int size, char* tag, char* value );
int  					get_areaID ( char *areaid);
char 					*tvms_get_browser_type(void);
char 					*tvms_get_browser_version(void);
char 					*tvms_get_stb_type(void);

int tvms_mid_mutex_unlock(void* mutex);
int tvms_mid_mutex_lock(void* mutex);
void* tvms_mid_mutex_create(void);
void tvms_mid_task_delay(unsigned int ms);
int tvms_tvms_areaid_get( char *url);
char * tvms_app_UserGroupNMB_get(void);

/*************************************************
       当前播放的内容的Code，用于获取内容相关的消息
       写入STB的内存，由EPG发起播放请求时写入STB中，
       机顶盒退出播放时，自动清除该内存
*************************************************/
int tvms_mediacode_write(const char *code);
int tvms_mediacode_read(char *code);

#endif
