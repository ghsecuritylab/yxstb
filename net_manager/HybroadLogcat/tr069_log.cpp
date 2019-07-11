#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "mid_timer.h"
#include "tr069_log.h"
#include "LogInit.h"
#include "nm_dbg.h"
#include "v2_param/extendConfig.h"
#include "tr069_interface.h"

#define START_TIME_LEN 32

struct log_param {
    int log_switch;
	int log_type;
	int log_level;
	int log_output_type;
	char server_ip[16 + 1];
	int port;
	char start_time_str[START_TIME_LEN + 1];
	struct timeval start_time;
	int continue_time; //minutes;
	int log_type_modified;
	int log_level_modified;
	int log_output_type_modified;
	int server_ip_modified;
	int port_modified;
	int start_time_modified;
	int continue_time_modified;
};

static struct log_param param = {};
static int init = 0;
static void* udp_handle = NULL;
static int transaction = 0;


//CCYY-MM-DDThh:mm:ss
static int date_str2time(const char* str, unsigned int *pVal)
{
    struct tm pt;

    if (!str)
        return 0;
    if (sscanf(str, "%04d-%02d-%02dT%02d:%02d:%02d", 
            &pt.tm_year, &pt.tm_mon, &pt.tm_mday,
            &pt.tm_hour, &pt.tm_min, &pt.tm_sec) != 6)
        return -1;
    pt.tm_year -= 1900;
    pt.tm_mon -= 1;
    *pVal = (unsigned int)mktime(&pt);

    return 0;
}


int date_time2str(int sec, char* buf)
{
	int len = 0;

	if (buf == NULL){
		nm_msg("buf is null\n");
		goto Err;
	}

	if (sec == 0) {
		buf[0] = 0;
	} else  {
		struct tm *pt;
		pt = gmtime((time_t *)&sec);
		len = sprintf(buf, "%04d-%02d-%02dT%02d:%02d:%02d",
						pt->tm_year + 1900,
						pt->tm_mon + 1,
						pt->tm_mday,
						pt->tm_hour,
						pt->tm_min,
						pt->tm_sec);
	}
	return len;
Err:
	return 0;
}


static void log_upload_begin(int arg)
{
	nm_track();
	udp_handle = attachUdpLogFilter(param.server_ip, param.port, param.log_type, param.log_level);
	nm_track();
}

static void log_upload_finish(int arg)
{
	nm_track();
	if(udp_handle){
		nm_track();
		detachUdpLogFilter(udp_handle);
		memset(&param, 0, sizeof(struct log_param));
		udp_handle = NULL;
	}
    tr069_log_param_save();
}

void log_upload_clear()
{
	mid_timer_delete(log_upload_begin, 0);
	mid_timer_delete(log_upload_finish, 0);
	if(udp_handle){
		nm_track();
		detachUdpLogFilter(udp_handle);
		udp_handle = NULL;
	}
}

static int log_param_check()
{
	struct timeval tm_now;
	int time_shift = 0;
	
	nm_msg_level(LOG_DEBUG, "%d, %d, %d, %d, %d, %d, %d", param.log_type_modified, param.log_level_modified, param.log_output_type_modified, param.server_ip_modified, param.port_modified, param.start_time_modified, param.continue_time_modified);
	if(param.log_type_modified && param.log_level_modified && param.log_output_type_modified &&
		param.server_ip_modified && param.port_modified && param.start_time_modified && param.continue_time_modified){
		param.log_type_modified = 0, param.log_level_modified = 0, param.log_output_type_modified = 0;
		param.server_ip_modified =0, param.port_modified =0, param.start_time_modified = 0, param.continue_time_modified = 0;
		nm_msg_level(LOG_DEBUG, "all log param is changed, process to upload the log\n");
		gettimeofday(&tm_now, NULL);
		time_shift = param.start_time.tv_sec - tm_now.tv_sec;
		nm_msg_level(LOG_DEBUG, "now is %d, start time is %d, continue time is %d\n", tm_now.tv_sec, param.start_time.tv_sec, param.continue_time);
		log_upload_clear();
        param.log_switch = 1;
        tr069_log_param_save();
		if(time_shift <= 0){
			nm_msg("start immediately\n");
			log_upload_begin(0);
		} else
			mid_timer_create(time_shift, 1, log_upload_begin, 0);
		mid_timer_create(time_shift + param.continue_time, 1, log_upload_finish, 0);
		return 0;
	}
	return -1;
}

int tr069_log_param_get(char *name, char *buf, int buf_len)
{
	if (!strcmp(name, "LogType") ){
		nm_track();
		sprintf(buf, "%d", param.log_type);
	} else if (!strcmp(name, "LogLevel") ){
		nm_track();
		sprintf(buf, "%d", param.log_level);
	} else if (!strcmp(name, "LogOutPutType")) {
		nm_track();
		sprintf(buf, "%d", param.log_output_type);
	} else if (!strcmp(name, "SyslogServer")){
		nm_track();
		snprintf(buf, buf_len, "%s:%d", param.server_ip, param.port);
	} else if (!strcmp(name, "SyslogStartTime")){
		//date_time2str(param.start_time.tv_sec, buf);
		strncpy(buf, param.start_time_str, buf_len);
       } else if (!strcmp(name, "SyslogContinueTime")){
       	sprintf(buf, "%d", param.continue_time / 60);
       }

	return 0;
}

void tr069_log_param_set(char *name, char *str)
{
    if(!init){
        memset(&param, 0, sizeof(struct log_param));
        init = 1;
    }
    //if (strncmp(name, "Device.X_00E0FC.LogParaConfiguration.", 37))
    //return;
    //name += 37;
    nm_msg_level(LOG_DEBUG, "%s:%d value is %s\n", name, strlen(name), str);
    if (!strcmp(name, "LogType") && str){
        nm_track();
        param.log_type = atoi(str);
        param.log_type_modified = 1;
    } else if (!strcmp(name, "LogLevel") && str){
        nm_track();
        param.log_level = atoi(str);
        param.log_level_modified = 1;
    } else if (!strcmp(name, "LogOutPutType") && str) {
        nm_track();
        param.log_output_type = atoi(str);
        param.log_output_type_modified = 1;
    } else if (!strcmp(name, "SyslogServer") && str){
        nm_track();
        char serverIp[128] = {0};
        char *port = strchr(str, ':');
        if(port && port[1] != 0){
            strncpy(serverIp, str, port - str);
            port ++;
            int port_int = atoi(port);
            nm_msg_level(LOG_DEBUG, "server is %s, serverip is %s, port is %d\n", str, serverIp, port_int);
            param.port = port_int;
            strncpy(param.server_ip, serverIp, 16);
            param.port_modified = 1;
            param.server_ip_modified = 1;
        }	
    } else if (!strcmp(name, "SyslogStartTime") && str){ //fromat like :"2013-12-24T15:17:38"
        unsigned int tm = 0;
        nm_track();
        if(date_str2time(str, &tm) == 0){
            param.start_time.tv_sec = tm;
            strncpy(param.start_time_str, str, START_TIME_LEN);
            param.start_time_modified = 1;
        }		
    } else if (!strcmp(name, "SyslogContinueTime") && str){
        nm_track();
        param.continue_time = 60 * atoi(str);
        param.continue_time_modified = 1;
    }
    log_param_check();
}

void tr069_log_param_save()
{
    extendConfigWrite("log");
}

void tr069_log_param_init()
{
    extendConfigInit( );
    extendConfigInsetObject("log");

    extendConfigInsetInt("log.LogSwitch", &param.log_switch);
    extendConfigInsetInt("log.LogType", &param.log_type);
    extendConfigInsetInt("log.LogLevel", &param.log_level);
    extendConfigInsetInt("log.LogOutPutType", &param.log_output_type);
    extendConfigInsetInt("log.SyslogServerPort", &param.port);
    extendConfigInsetString("log.SyslogServerIP", param.server_ip, 17);
    extendConfigInsetString("log.SyslogStartTime", param.start_time_str, 33);
    extendConfigInsetInt("log.SyslogContinueTime", &param.continue_time);

    extendConfigRead("log");

    if (param.log_switch) {
        param.log_type_modified = 1, param.log_level_modified = 1, param.log_output_type_modified = 1;
        param.server_ip_modified =1, param.port_modified =1, param.start_time_modified = 1, param.continue_time_modified = 1;
        log_param_check();
    }
}
