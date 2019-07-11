#include <stdio.h>
#include <stdarg.h>
#include <sys/time.h>
#include <time.h>

#include "nm_dbg.h"

#define TR069_DBG_INI "/data/tr069_debug.ini"

const char *ini_default = "debug_level:0\nhelp:debug_level value { 0, debug,output all info; 1, warning, output warnings && errors; 2,error, only output errors}\n";

#ifdef ANDROID_LOGCAT_OUTPUT
#include <android/log.h>
/*
typedef enum android_LogPriority {
	ANDROID_LOG_UNKNOWN = 0,
	ANDROID_LOG_DEFAULT,
	ANDROID_LOG_VERBOSE, 
	ANDROID_LOG_DEBUG,
	ANDROID_LOG_INFO, 
	ANDROID_LOG_WARN, 
	ANDROID_LOG_ERROR, 
	ANDROID_LOG_FATAL, 
	ANDROID_LOG_SILENT, } 
*/
#else
#define output_fun printf
#endif

#define LOG_CACHE_SIZE 2048
#define HW_VERSION_LEN 32
#define HW_ADDITIONAL_VERSION_LEN 64
#define HW_MAC_LEN 24

static char hw_version[HW_VERSION_LEN + 1] = {0};
static char hw_additional_version[HW_ADDITIONAL_VERSION_LEN + 1] = {0};
static char hw_mac[HW_MAC_LEN + 1] = {0};
static int output_level = LOG_DEBUG;
static int debug_init = 0;

static void  date_get(struct tm* dt)
{
    if (dt) {
        struct timeval current;
        struct tm temp_time;
        
        if (!gettimeofday(&current, NULL)){
            localtime_r(&current.tv_sec, &temp_time);
            dt->tm_year       = temp_time.tm_year + 1900;
            dt->tm_mon      = temp_time.tm_mon + 1;
            dt->tm_wday  = temp_time.tm_wday;
            dt->tm_mday        = temp_time.tm_mday;
            dt->tm_hour       = temp_time.tm_hour;
            dt->tm_min     = temp_time.tm_min;
            dt->tm_sec     = temp_time.tm_sec;
	    }
    }
}

int nm_dbg_init()
{
	char data_buf[4 * 1024 + 1] = {0};
	int ret;
	char *temp_ptr = NULL;
	int data_len = 0;
	FILE *fp = fopen(TR069_DBG_INI, "r");

	if(!fp){
		fp = fopen(TR069_DBG_INI, "w");
		ret = fwrite(ini_default, strlen(ini_default), 1, fp);
		if(ret >= 0){
			fclose(fp);
			debug_init = 1;
			return 0;
		}		
	} else {
		ret = fgets(data_buf, 4 * 1024, fp);
		if( ret && strlen(data_buf) > 0){
			data_len = strlen(data_buf);
			temp_ptr = strstr(data_buf, "debug_level");
			if(temp_ptr){
				temp_ptr += strlen("debug_level");
				while((*temp_ptr == ' ' || *temp_ptr ==':') &&  temp_ptr <= (data_buf + data_len))
					temp_ptr ++;
				if(temp_ptr <= (data_buf + data_len)){
					output_level = atoi(temp_ptr);
					printf("output level is %d\n", output_level);
				}
			}
			debug_init = 1;
			fclose(fp);
			return 0;
		}
		fclose(fp);
	}
	return -1;
}

/*
3	Error:只输出有报错的日志 
6	Informational: 输出事务日志（本文中所有定义的类别消息）
7	Debug: 输出所有日志，包括机顶盒的内部调试信息、浏览器的调试信息。

*/
static const char* text_level[] = {"Assert : ", "Error! : ", "Warning: ", "Normal : ", "Verbose: "};
static const char* month_str[] = {"Undefined", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

void nm_dbg(int log_level, const char *file, const char *function, int line, const char *fmt, ...)
{
	char m_message[LOG_CACHE_SIZE + 1] = {0};
	va_list ap;
	int no_msg = 0;

	if(!debug_init)
		nm_dbg_init();
	if (log_level < LOG_DEBUG || log_level > LOG_ERROR) {
		return;
	}
	if (output_level > log_level)
		return ;
	if(fmt){
		va_start(ap, fmt);
		vsnprintf(m_message, LOG_CACHE_SIZE, fmt, ap);
		va_end(ap);
	} else no_msg = 1;
	#ifdef ANDROID_LOGCAT_OUTPUT
	{
		android_LogPriority android_dbg_level;
		struct tm dt;
		int info = 17 * 8;
		int level_index = 4; //default Verbose

		switch(log_level){
			case LOG_DEBUG:{
				android_dbg_level = ANDROID_LOG_DEBUG;
				info += 7; 
				level_index = 3;
				break;
			}
			case LOG_WARNING:{
				android_dbg_level = ANDROID_LOG_WARN;
				info += 6; 
				level_index = 2;
				break;
			}
			case LOG_ERROR:{
				android_dbg_level = ANDROID_LOG_ERROR;
				info += 3; 
				level_index = 1;
				break;
			}	
			default:{
				info += 7; 
				android_dbg_level = ANDROID_LOG_VERBOSE;
			}
		}
		date_get(&dt);
		if(no_msg){
			__android_log_print(android_dbg_level   , "net_manager", "%02d %02d:%02d:%02d %s %s %s %s:%d code:11 %s", dt.tm_mday, 
			dt.tm_hour, dt.tm_min, dt.tm_sec, hw_mac, hw_version, hw_additional_version, file, line, text_level[level_index]);
		} else {
			__android_log_print(android_dbg_level   , "net_manager", "<%d>%s %02d %02d:%02d:%02d %s %s %s %s:%d code:11 %s %s", info, month_str[dt.tm_mon], dt.tm_mday, 
			dt.tm_hour, dt.tm_min, dt.tm_sec, hw_mac, hw_version, hw_additional_version, file, line, text_level[level_index], m_message);
		}            
	}	
	#else
	output_fun("%s", m_message);
	#endif
	return;
};

int nm_dbg_sysinfo_set(char *version, char *additional_version, char *mac)
{
	if(version)
	strncpy(hw_version, version, HW_VERSION_LEN);
	if(additional_version)
	strncpy(hw_additional_version, additional_version, HW_ADDITIONAL_VERSION_LEN);
	if(mac)
	strncpy(hw_mac, mac, HW_MAC_LEN);
	return 0;
}

int nm_dbg_output_level_set(int level)
{
	if(level < LOG_DEBUG || level > LOG_ERROR){
		return -1;
	}
	output_level = level;
	return 0;
}

int file_dump(char *file_name, char *buf, int buf_len)
{
	FILE *f_dump;

	f_dump = fopen(file_name, "w");
	fwrite(buf, 1, buf_len, f_dump);
	fclose(f_dump);
	return 0;
}

