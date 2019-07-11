#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mid_stream.h"
#include "mid_task.h"
#include "mid/mid_tools.h"
#include "mid/mid_time.h"
#include "mid/mid_mutex.h"

#include "AppSetting.h"
#include "sys_basic_macro.h"
#include "sys_msg.h"
#include "app_tool.h"
#include "stream_port.h"

#include "tvms.h"
#include "tvms_define.h"
#include "tvms_setting.h"
#include "TvmsAssertions.h"

#include "SystemManager.h"
#include "UltraPlayer.h"
#include "BrowserBridge/Huawei/BrowserEventQueue.h"
#include "mid/mid_http.h"
#include "Session.h"

#define A2_INFO_LEN_MAX 1024


/*************************************************************
*   TVMS部分两个主线程
*
**************************************************************/

extern void tvms_request_loop();
extern void tvms_time_scan_loop();
extern void printf_tvms_msg(char *str,TVMS *p);
extern int  get_areaID ( char *areaid);
extern char* global_cookies;
/*0 on success, -1 on failure*/
int
tvms_request_task_create(void)
{
	return mid_task_create("tvms_request_task",(mid_func_t)tvms_request_loop, 0);
}

int
tvms_time_scan_task_create(void)
{
	return mid_task_create("tvms_time_scan_task",(mid_func_t)tvms_time_scan_loop, 0);
}

void
tvms_loop_init()
{
	return;
}

/*************************************************************
*    时间相关函数
*
**************************************************************/

int
tvms_time2string(int sec, char* buf, char insert)
{
	return mid_tool_time2string(sec, buf, insert);
}


unsigned int
tvms_string2time(char *str)
{
	return mid_tool_string2time(str);
}


/*得到当前的时间*/
unsigned int
tvms_mid_time()
{
	return mid_tvms_time();
}

/***********************************************
*    VOD播放相关
*************************************************/
/**获取当前的VOD播放状态*/
int
tvms_get_vod_playstate()
{
	if(STRM_STATE_PLAY == stream_port_get_state())
		return 0;
	else
		return -1;
}

/**得到VOD播放的偏移时间，服务器时间*/
unsigned int
tvms_get_vod_time()
{
        Hippo::SystemManager &sysManager = Hippo::systemManager();
        Hippo::UltraPlayer *player = NULL;
        unsigned int vod_time = 0;

        player = sysManager.obtainMainPlayer();

        if(player){
            vod_time = player->getCurrentTime();
            sysManager.releaseMainPlayer(player);
        }

        return vod_time;

}

/******************************************************
*				升级文件配置
*******************************************************/
/**从FLASH中读入TVMS心跳请求时间*/

int
tvms_heart_time_read()
{
	int hearttime = 0;

	tvms_conf_tvmsheartbitinterval_get(&hearttime);
	if(hearttime <= 0)
		hearttime = MIN_HEART_TIME;
	return hearttime;
}


/**从FLASH中读入TVMS的delay时间*/

int
tvms_delay_time_read()
{
	int delaytime = 0;

	tvms_conf_tvmsdelaylength_get(&delaytime);
	if(delaytime <MIN_DELAY_TIME)
		delaytime = MIN_DELAY_TIME ;
	return delaytime;
}


/**从FLASH中读入TVMS心跳请求URL*/

int
tvms_heart_url_read(char *url,int len)
{
	int ret = 0;
	if(url == NULL)
		return -1;
	url[0] = '\0';

	ret =tvms_conf_tvmsheartbiturl_get(url);
	if(ret == -1)
		return -1;
	if(strlen(url) == 0){
		ret = -1;
	}
	return ret;
}
/**从FLASH中读入TVMS vod请求URL*/

int
tvms_heartvod_url_read(char *url,int len)
{
	int ret = 0;
	if(url == NULL)
		return -1;
	url[0] = '\0';

	ret = tvms_conf_tvmsvodheartbiturl_get(url);
	if(ret == -1)
		return -1;
	if(strlen(url) == 0){
		ret = -1;
	}
	return ret;
}

/*************************************************
Description:获取mediacode
Input:     无
output:   code:mediacode
Return:   0:成功;-1:失败
 *************************************************/
static char g_app_mediacode[128] = { 0 };

int tvms_mediacode_read(char *code)
{
    if (!code)
        return -1;
    code[0] = '\0';
    strcpy(code, g_app_mediacode);
    return 0;
}

/*************************************************
Description:设置mediacode
Input:     code:mediacode
output:   无
Return:   0:成功;-1:失败
 *************************************************/

int tvms_mediacode_write(const char *code)
{
    int len = 0;
    if (!code)
        ERR_OUT("mediacode is null.\n");
    len = strlen(code);
    len = ((len > 127) ? 127 : len);
    strncpy(g_app_mediacode, code, len);
    g_app_mediacode[len] = '\0';
    return 0;
Err:
    return -1;
}

/**从FLASH中读入STB账号*/
int
tvms_userid_read(char *userid)
{
	char buf[USER_LEN];
	char *p = NULL;

	if(userid == NULL)
		return -1;
	appSettingGetString("ntvuser", buf, USER_LEN, 0);
	p = buf;
	if(p == NULL)
		return  -1;
	strcpy(userid,p);
	return 0;
}


/****************************************
*  考虑到文广需求是4个字节；?
*华为方面也没有最终确定?
*  此地方暂时没有实际作用，
*  使用时候使用自己随机定义的。
****************************************/
char *
tvms_get_browser_type()
{
	return "Ipanel";
}

char *
tvms_get_browser_version()
{
	return "0108";
}

char *
tvms_get_stb_type()
{
	return "EC2108";
}

/*************************************************
*        EVENT_TVMS
************************************************/
/**发送EVENT_TVMS事件给浏览器*/
void
tvms_send_event(TVMS *p)
{
	if(p == NULL)
		return;
	printf_tvms_msg("send_show_msg_browser",p);

	char displayURL[512] = {0};
	char areaid[128+1] = {0};
	char userid[32] = {0};

	get_areaID (areaid);
	tvms_userid_read(userid);
	if(strlen(areaid) == 0 || strlen(userid) == 0)
 		return ;
	//snprintf(displayURL,512,"%s?UserId=%s&UserGroup=%s&Vendor=HUAWEI&msgTaskCode=%s&task_type=%d&media_Code=%s&send_time=%s&StbType=%s&BrowserType=%s&BrowserVer=%s",
	//	p->displayURL,userid,areaid,p->msgTaskCode,p->taskType,p->mediaCode,p->origsendTime,tvms_get_stb_type(),tvms_get_browser_type(),tvms_get_browser_version());

#if 0
	snprintf(displayURL,512,"%s",p->displayURL);
#endif

	TVMS_LOG("tvms_send_event ,displayurl =%s\n",displayURL);
	if((p->taskType == 0) && (p->mode == 3)){
		char event[A2_INFO_LEN_MAX] = {0};
		snprintf(event, A2_INFO_LEN_MAX, "{\"type\":\"EVENT_TVMS_MESSAGE\",\"message_key_name\":\"%s\"}", displayURL);
               TVMS_LOG("event:%s\n", event);
		browserEventSend(event, NULL);
	}
	else {
		char event[A2_INFO_LEN_MAX] = {0};
		snprintf(event,A2_INFO_LEN_MAX,"{\"type\":\"EVENT_TVMS\",\"msgTaskCode\":\"%s\",\"mode\":%d,\"tvmsURL\":\"%s\",\"priority\":%d,\"confirmFlag\":%d,\"taskType\":%d,\"sendtime\":\"%s\",\"title\":\"%s\"}" ,\
										p->msgTaskCode, \
										p->mode,\
										p->displayURL,\
										p->priority,\
										p->confirmFlag,\
										p->taskType,\
										p->origsendTime,\
										p->title);
		//snprintf(event, A2_INFO_LEN_MAX, "{\"type\":\"EVENT_TVMS\",\"tvmsURL\":\"%s\",\"msgTaskCode\":\"%s\",	\
		//	\"priority\":%d,\"confirmFlag\":%d}", displayURL,p->msgTaskCode,p->priority,p->confirmFlag);
               TVMS_LOG("event:%s\n", event);
		browserEventSend(event, NULL);
	}
	return;
}
#if 1
/**发送EVENT_TVMS_ERROR事件给浏览器*/
extern "C" void
tvms_send_error_event(char *errorurl1)
{
	if(errorurl1 == NULL)
		return;
	int ret= -1 ;
	char *errorurl=NULL;
	char *temp = NULL;
	char *temp2 = NULL;
	int templen = 0;
	char displayURL[1024] = {0};
	char title[51] = {0};
	char msgtaskcode[129]	= {0};
	char event[A2_INFO_LEN_MAX] = {0};

	errorurl = strstr(errorurl1+strlen("http://"),"http://");
	if(errorurl == NULL)
		errorurl = errorurl1;

	temp = strstr(errorurl,"?UserId=");
	if(temp == NULL)
		return ;
	templen = temp - errorurl;
	if(templen > 1024)
		return ;
	//原始的URL
	memcpy(displayURL,errorurl,templen);
	displayURL[templen] = '\0';

	temp2 = strstr(temp,"msgTaskCode=");
	if(temp2 == NULL)
		return;
	temp2 += strlen("msgTaskCode=");

	temp= NULL;
	temp = strstr(temp2,"&task_type=");
	if(temp == NULL)
		return;
	TVMS_LOG("---------------------\n");
	templen = temp -temp2;
	if(templen > 128)
		return;
	TVMS_LOG("---------------------\n");
	memcpy(msgtaskcode,temp2,templen);
	msgtaskcode[templen] = '\0';
	TVMS_LOG("------msgtaskcode =%s---------------\n",msgtaskcode);
	//获取TVMS的标题
	ret = search_showlist_by_msgcode(msgtaskcode,title);
	if(ret == -1){
		TVMS_LOG("---now can 't find the msgcode-----\n");
		return ;
	}
	TVMS_LOG("---------------------\n");
	//发送event 事件
	snprintf(event, A2_INFO_LEN_MAX, "{\"type\":\"EVENT_TVMS_ERROR\",\"tvms_title\":\"%s\",	\"tvms_url\":\"%s\"}", title,displayURL);
	browserEventSend(event, NULL);

	return;
}
#endif





/*************************************************
*				INI	文件相关
*************************************************/
char*
tvms_file_get_line( char* s_str, int s_len, char* buf )
{
	return file_get_line(s_str,s_len, buf );
}

int
tvms_line_get_bracket( char* s_str, int s_len, char* title )
{
	return  line_get_bracket( s_str,s_len, title );
}

int
tvms_line_get_tag( char* str, int size, char* tag, char* value )
{
	return line_get_tag(str,size, tag, value );
}

/****通过HTTP读取INI文件*********/
int
tvms_get_inifile(char *url,void *func)
{
    return mid_http_call(url, (mid_http_f)func, 0, NULL, 0, global_cookies);
}


/**********************************************
*				互斥锁
***********************************************/
int
tvms_mid_mutex_lock(void* mutex)
{
	//testu++;
	//TVMS_LOG("0tvms_mid_mutex_lock=%d\n",testu);
	int ret =mid_mutex_lock( (mid_mutex_t)mutex);
	//TVMS_LOG("1tvms_mid_mutex_lock=%d\n",testu);
	return ret ;
}

int
tvms_mid_mutex_unlock(void* mutex)
{
	//TVMS_LOG("0tvms_mid_mutex_unlock=%d\n",testu);
	int ret = mid_mutex_unlock((mid_mutex_t) mutex);
	//TVMS_LOG("1tvms_mid_mutex_unlock=%d\n",testu);
	return ret ;
}

void*
tvms_mid_mutex_create(void)
{
	return (void *)mid_mutex_create();
}

/*******************毫秒级别的延时*******************************/
void
tvms_mid_task_delay(unsigned int ms)
{
	mid_task_delay(ms);  //WZW modified to fix pc-lint Error 144
}

int
tvms_tvms_areaid_get( char *url)
{
	return appSettingGetString("areaid", url, 128, 0);
}

/*********************消息队列*****************************/

int
tvms_yx_msgq_put( tvms_yxmsgq_t msgq, char* pmsg )
{
	return mid_msgq_putmsg(msgq,pmsg);
}
int
tvms_yx_msgq_get( tvms_yxmsgq_t msgq, char * pmsg )
{
	return mid_msgq_getmsg(msgq,pmsg);
}

char *
tvms_app_UserGroupNMB_get()
{
	return session().getUserGroupNMB();
}
