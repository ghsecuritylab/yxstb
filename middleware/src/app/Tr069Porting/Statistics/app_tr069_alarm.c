
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "app_tr069_alarm.h"
#include "libzebra.h"
#include "Assertions.h"
#include "tr069_api.h"
#include "tr069_port_alarm.h"
#include "mid_stream.h"
#include "tr069_port.h"
#include "../ParameterMap/Device/X_CU_STB/Alarm/Tr069CUAlarm.h"
#include "TR069Assertions.h"
#include "config/pathConfig.h"

#include <sys/vfs.h>
#include <dirent.h>
#if defined(INCLUDE_TR069)


typedef struct jiffy_counts_cpu {
	/* Linux 2.4.x has only first four */
	unsigned long long usr, nic, sys, idle;
	unsigned long long iowait, irq, softirq, steal;
	unsigned long long total;
	unsigned long long busy;
} jiffy_counts_cpu;

struct globals {
	jiffy_counts_cpu cur_jif;
	jiffy_counts_cpu prev_jif;
	char line_buf[80];
}G_cpu;

#define prev_jif         (G_cpu.prev_jif          )
#define cur_jif          (G_cpu.cur_jif           )
#define line_buf         (G_cpu.line_buf          )

static FILE* xfopen(const char *path, const char *mode)
{
	FILE *fp = fopen(path, mode);
	if (fp == NULL)
		ERR_OUT("can't open '%s'", path);
	return fp;
Err:
	return NULL;
}

static FILE*  xfopen_for_read(const char *path)
{
	return xfopen(path, "r");
}


 static int read_cpu_jiffy(FILE *fp, jiffy_counts_cpu *p_jif)
{
	static const char fmt[] = "cpu %llu %llu %llu %llu %llu %llu %llu %llu";
	int ret;

	if (!fgets(line_buf, 80, fp) || line_buf[0] != 'c' /* not "cpu" */)
		return 0;
	ret = sscanf(line_buf, fmt,
			&p_jif->usr, &p_jif->nic, &p_jif->sys, &p_jif->idle,
			&p_jif->iowait, &p_jif->irq, &p_jif->softirq,
			&p_jif->steal);
	if (ret >= 4) {
		p_jif->total = p_jif->usr + p_jif->nic + p_jif->sys + p_jif->idle
			+ p_jif->iowait + p_jif->irq + p_jif->softirq + p_jif->steal;
		/* procps 2.x does not count iowait as busy time */
		p_jif->busy = p_jif->total - p_jif->idle - p_jif->iowait;
	}

	return ret;
}


static void get_jiffy_counts(void)
{
	FILE* fp = xfopen_for_read("/proc/stat");

	/* We need to parse cumulative counts even if SMP CPU display is on,
	 * they are used to calculate per process CPU% */
	prev_jif = cur_jif;
	if (read_cpu_jiffy(fp, &cur_jif) < 4)
		ERR_OUT("can't read /proc/stat");

	fclose(fp);
	return;
Err:
	fclose(fp);
	return;
}

static char *fmt_100percent_8(char pbuf[8], unsigned long long value, unsigned long long total)
{
	unsigned long long t;
	if (value >= total) { /* 100% ? */
		strcpy(pbuf, "  100% ");
		return pbuf;
	}
	/* else generate " [N/space]N.N% " string */
	value = 1000 * value / total;
	t = value / 100;
	value = value % 100;
	pbuf[0] = ' ';
	pbuf[1] = t ? t + '0' : ' ';
	pbuf[2] = '0' + (value / 10);
	pbuf[3] = '.';
	pbuf[4] = '0' + (value % 10);
	pbuf[5] = '%';
	pbuf[6] = ' ';
	pbuf[7] = '\0';
	return pbuf;
}


static void Search_mem_Status(char *memusedbuf)
{
	FILE *fp;
	char buf[80];
	char scrbuf[80];
	char pbuf[8];
	unsigned long total,used,mfree, shared, buffers, cached;

	/* read memory info */
	fp = xfopen_for_read("/proc/meminfo");

	/*
	 * Old kernels (such as 2.4.x) had a nice summary of memory info that
	 * we could parse, however this is gone entirely in 2.6. Try parsing
	 * the old way first, and if that fails, parse each field manually.
	 *
	 * First, we read in the first line. Old kernels will have bogus
	 * strings we don't care about, whereas new kernels will start right
	 * out with MemTotal:
	 *                              -- PFM.
	 */
	if (fscanf(fp, "MemTotal: %lu %s\n", &total, buf) != 2) {
		fgets(buf, sizeof(buf), fp);    /* skip first line */

		if (fscanf(fp, "Mem: %lu %lu %lu %lu %lu %lu", &total, &used, &mfree, &shared, &buffers, &cached) != 6)
			LogTr069Debug("fscanf err\n");
		/* convert to kilobytes */

		used /= 1024;
		mfree /= 1024;
		shared /= 1024;
		buffers /= 1024;
		cached /= 1024;
		total /= 1024;
	} else {
		/*
		 * Revert to manual parsing, which incidentally already has the
		 * sizes in kilobytes. This should be safe for both 2.4 and
		 * 2.6.
		 */
		if (fscanf(fp, "MemFree: %lu %s\n", &mfree, buf) != 2)
                    LogTr069Debug("fscanf err\n");

		/*
		 * MemShared: is no longer present in 2.6. Report this as 0,
		 * to maintain consistent behavior with normal procps.
		 */
		if (fscanf(fp, "MemShared: %lu %s\n", &shared, buf) != 2)
			shared = 0;

		if (fscanf(fp, "Buffers: %lu %s\n", &buffers, buf) != 2)
                LogTr069Debug("fscanf err\n");

		if (fscanf(fp, "Cached: %lu %s\n", &cached, buf) != 2)
                LogTr069Debug("fscanf err\n");

		used = total - mfree;
	}
	fclose(fp);
	fmt_100percent_8(pbuf,(unsigned long long)used,(unsigned long long)total);
	snprintf(memusedbuf,6,"%s",pbuf);
	/* output memory info */
	snprintf(scrbuf, 80,
		"Mem: %luK total, %luK used, %luK free, %luK shrd, %luK buff, %luK cached",
		total, used, mfree, shared, buffers, cached);

	LogTr069Debug("%s\n",scrbuf);

	return;
}

static void Search_cpu_Status(char *freebuf)
{
	unsigned total_diff;
	jiffy_counts_cpu *p_jif, *p_prev_jif;
	char scrbuf[80];

	get_jiffy_counts();

#define CALC_STAT(xxx) char xxx[8]
#define SHOW_STAT(xxx) fmt_100percent_8(xxx, (unsigned long long)(p_jif->xxx - p_prev_jif->xxx), (unsigned long long)total_diff)
#define FMT "%s"

	p_jif = &cur_jif;
	p_prev_jif = &prev_jif;

	total_diff = (unsigned)(p_jif->total - p_prev_jif->total);
	if (total_diff == 0) total_diff = 1;

	/* Need a block: CALC_STAT are declarations */
	CALC_STAT(usr);
	CALC_STAT(sys);
	CALC_STAT(nic);
	CALC_STAT(idle);
	CALC_STAT(iowait);
	CALC_STAT(irq);
	CALC_STAT(softirq);
	/*CALC_STAT(steal);*/
	snprintf(freebuf,6,FMT,SHOW_STAT(idle));
	snprintf(scrbuf, 80,
			"CPU:"FMT"usr"FMT"sys"FMT"nic"FMT"idle"FMT"io"FMT"irq"FMT"sirq",
			SHOW_STAT(usr), SHOW_STAT(sys), SHOW_STAT(nic), SHOW_STAT(idle),
			SHOW_STAT(iowait), SHOW_STAT(irq), SHOW_STAT(softirq));
	LogTr069Debug("%s\n",scrbuf);
#undef SHOW_STAT
#undef CALC_STAT
#undef FMT
}

static int ConversionAlarmValue(int * lower_limit,int * upper_limit, char * alarm_buf)
{
	char temp_buf[32] = {0};
	char *p = NULL;

	if(alarm_buf == NULL)
		ERR_OUT("alarm value is not exist!");
	p = strchr(alarm_buf,',');
	if (p == NULL) {
		*upper_limit = 100;
		ERR_OUT("alarm value is not valid!");
	}
	memcpy(temp_buf,alarm_buf,p-alarm_buf);
	temp_buf[p-alarm_buf] = 0;
	*lower_limit  = atoi(temp_buf);
	*upper_limit = atoi(p+1);

	if ( *lower_limit <= 0 || *lower_limit > *upper_limit) {
		*upper_limit = 100;
		ERR_OUT("alarm value is not valid!");
	}
Err:
	return -1;
}

static int gAlarmSwitch = 0;
static int gPacketsLostLowerLimit = 80;
static int gPacketsLostUpperLimit = 90;

static int gFramesLostLowerLimit = 80;
static int gFramesLostUpperLimit = 90;

void app_setAlarmSwitch(char *str)
{
    gAlarmSwitch = atoi(str);
}

void app_setPacketsLostAlarmValue(char *str)
{
    sscanf(str, "%d,%d", &gPacketsLostLowerLimit, &gPacketsLostUpperLimit);
    LogTr069Debug("lower = %d, upper = %d\n", gPacketsLostLowerLimit, gPacketsLostUpperLimit);
}

void app_setFramesLostAlarmValue(char *str)
{
    sscanf(str, "%d,%d", &gFramesLostLowerLimit, &gFramesLostUpperLimit);
    LogTr069Debug("lower = %d, upper = %d\n", gFramesLostLowerLimit, gFramesLostUpperLimit);
}

void app_reportPacketsLost(int permille)
{
    int tr069Type;

    tr069Type = 0;
    sysSettingGetInt("tr069_type", &tr069Type, 0);

    if (2 == tr069Type) {
        tr069_cu_setPacketsLost(permille);
    } else {
        static int alarmFlag = 0;
        int lower, upper, alarmSwitch;

        lower = gPacketsLostLowerLimit * 10;
        upper = gPacketsLostUpperLimit * 10;
        alarmSwitch = gAlarmSwitch;

        LogTr069Debug("lower = %d, upper = %d, permille = %d, alarmFlag = %d, alarmSwitch = %d\n", lower, upper, permille, alarmFlag, alarmSwitch);
        if (permille < lower && alarmFlag) {
            if (gAlarmSwitch)
                tr069_port_alarm_clear(ALARM_TYPE_Dropped);
            alarmFlag = 0;
        } else if (permille > upper && !alarmFlag) {
            if (gAlarmSwitch)
            #ifdef Sichuan
            	tr069_port_alarm_post(ALARM_TYPE_Dropped,ALARM_DROP_PACKAGE_CODE,ALARM_LEVEL_CRITICAL,"Package Loss Rate Exceeds The Threshold Alarm");
            #else
            	tr069_port_alarm_post(ALARM_TYPE_Dropped,ALARM_DROP_PACKAGE_CODE,ALARM_LEVEL_WARNING,"Contact Operator Service Interface");
            #endif
            alarmFlag = 1;
        }
    }
}

int app_get_framelos_vaue(int * lower_limit, int * upper_limit)
{
    *lower_limit = gFramesLostLowerLimit;
    *upper_limit = gFramesLostUpperLimit;
    return 0;
}

/*  机顶盒升级失败告警 */
int app_report_upgrade_alarm(int discrcode)
{
	char strbuf[32] = {0};
	LogTr069Debug("upgrade failed,discrcode [%d]\n",discrcode);
	switch(discrcode)
	{
		case UPGRADE_NET_DISCONNECT:
			strncpy(strbuf,"Upgrade Server Can't Access",sizeof("Upgrade Server Can't Access"));
			break;
		case UPGRADE_ILLEGAL_VERSION:
			strncpy(strbuf,"Firmware Signature Check Failed",sizeof("Firmware Signature Check Failed"));
			break;
		case UPGRADE_WRITE_FLASH_FAIL:
			strncpy(strbuf,"Firmware Write Failed",sizeof("Firmware Write Failed"));
			break;
		default:
			break;
	}
#ifndef Sichuan
	tr069_port_alarm_post(ALARM_TYPE_STB_Upgrade,ALARM_UPGRAGE_CODE,ALARM_LEVEL_MAJOR,strbuf);
#endif
	return 0;
}

/* 加入频道失败告警	*/
int app_report_channel_alarm(int discrcode)
{
#ifndef Sichuan
	char strbuf[32] = {0};
	LogTr069Debug("discrcode [%d]\n",discrcode);
	switch(discrcode)
	{
		case STRM_MSG_OPEN_ERROR:
			strncpy(strbuf,"Protocol Interoperate Error",sizeof("Protocol Interoperate Error"));
			tr069_port_alarm_post(ALARM_TYPE_Channel,ALARM_CHANNEL_CODE,ALARM_LEVEL_MAJOR,strbuf);
			break;
		case STRM_MSG_RECV_TIMEOUT:
			strncpy(strbuf,"Streaming Timeout",sizeof("Streaming Timeout"));
			tr069_port_alarm_post(ALARM_TYPE_Channel,ALARM_CHANNEL_CODE,ALARM_LEVEL_MAJOR,strbuf);
			break;
		default:
			break;
	}
#endif
	return 0;
}

/* 机顶盒解码错误告警*/
int app_report_decode_alarm(void)
{
#ifndef Sichuan
	tr069_port_alarm_post(ALARM_TYPE_Decode,ALARM_DECODE_CODE,ALARM_LEVEL_MAJOR,"CHANNEL ALARM");
#endif
	return 0;
}

/* IPTV业务认证失败告警	 */
int app_report_authorize_alarm(int value)
{
#ifndef Sichuan
	if(value == 0)
		tr069_port_alarm_post(ALARM_TYPE_Authorize,ALARM_AUTHORIZE_CODE,ALARM_LEVEL_CRITICAL,"Contact Operator Service Interface");
#endif
	return 0;
}

#ifdef Sichuan
/* 机顶盒解密错误告警*/
int app_report_decrypt_alarm(void)
{
//	tr069_port_alarm_post(ALARM_TYPE_Decrypt,ALARM_DECRYPT_CODE,ALARM_LEVEL_MAJOR,"Stb Decrypt Fail");
	return 0;
}

/* 缓冲溢出告警*/
int app_report_cushion_alarm(void)
{
	tr069_port_alarm_post(ALARM_TYPE_Cushion,ALARM_CUSHION_CODE,ALARM_LEVEL_MINOR,"Cushion Overflow");
	return 0;
}

/* 丢帧率超过阈值告警*/
int app_report_drop_frame_alarm(void)
{
	tr069_port_alarm_post(ALARM_TYPE_Dropframe,ALARM_DROP_FRAME_CODE,ALARM_LEVEL_CRITICAL,"Frame Loss Rate Exceeds The Threshold Alarm");
	return 0;
}

/* 时延超过阈值告警*/
int app_report_time_lapse_alarm(void)
{
	tr069_port_alarm_post(ALARM_TYPE_Timelapse,ALARM_TIME_LAPSE_CODE,ALARM_LEVEL_CRITICAL,"Time Lapse Exceeds The Threshold Alarm");
	return 0;
}

/* 访问EPG 服务器失败告警*/
int app_report_epg_access_alarm(void)
{
	tr069_port_alarm_post(ALARM_TYPE_EPG_Access,ALARM_EPG_ACCESS_CODE,ALARM_LEVEL_CRITICAL,"Access To EPG Server Fail");
	return 0;
}

/* 访问媒体服务器失败告警*/
int app_report_media_access_alarm(void)
{
	tr069_port_alarm_post(ALARM_TYPE_Media_Access,ALARM_MEDIA_ACCESS_CODE,ALARM_LEVEL_CRITICAL,"Access To Media Server Fail");
	return 0;
}


/* 文件服务器连接失败告警*/
int app_report_file_access_alarm(void)
{
	tr069_port_alarm_post(ALARM_TYPE_File_Access,ALARM_FILE_ACCESS_CODE,ALARM_LEVEL_CRITICAL,"Access To File Server Fail");
	return 0;
}


/* 媒体格式不支持告警*/
int app_report_media_format_alarm(void)
{
//	tr069_port_alarm_post(ALARM_TYPE_Media_Format,ALARM_MEDIA_FORMAT_CODE,ALARM_LEVEL_MAJOR,"Media Format Is Not Supported");
	return 0;
}

int app_clear_decrypt_alarm(void)
{
	tr069_port_alarm_clear(ALARM_TYPE_Decrypt);

	return 0;
}

int app_clear_cushion_alarm(void)
{
	tr069_port_alarm_clear(ALARM_TYPE_Cushion);

	return 0;
}

int app_clear_drop_frame_alarm(void)
{
	tr069_port_alarm_clear(ALARM_TYPE_Dropframe);

	return 0;
}

int app_clear_time_lapse_alarm(void)
{
	tr069_port_alarm_clear(ALARM_TYPE_Timelapse);

	return 0;
}

int app_clear_epg_access_alarm(void)
{
	tr069_port_alarm_clear(ALARM_TYPE_EPG_Access);

	return 0;
}

int app_clear_media_access_alarm(void)
{
	tr069_port_alarm_clear(ALARM_TYPE_Media_Access);

	return 0;
}

int app_clear_file_access_alarm(void)
{
	tr069_port_alarm_clear(ALARM_TYPE_File_Access);

	return 0;
}

int app_clear_media_format_alarm(void)
{
	tr069_port_alarm_clear(ALARM_TYPE_Media_Format);

	return 0;
}
#endif

int app_clear_decode_alarm(void)
{
	tr069_port_alarm_clear(ALARM_TYPE_Decode);

	return 0;
}

int app_clear_authorize_alarm(void)
{
#ifndef Sichuan
	tr069_port_alarm_clear(ALARM_TYPE_Authorize);
#endif
	return 0;
}

int tr069_port_get_CPUrate(void)
{
	char cpufreebuf[8] = {0};
	int cpu_percent = 0;
	Search_cpu_Status(cpufreebuf);
	cpu_percent = 100 - atoi(cpufreebuf);
	return cpu_percent;
}
int tr069_port_get_memValue(void)
{
	char memusedbuf[8] = {0};
	int mem_percent;

	Search_mem_Status(memusedbuf);
	mem_percent = atoi(memusedbuf);
	return mem_percent;
}
#else

int app_get_framelos_vaue(int * lower_limit, int * upper_limit)
{
	return 0;
}

int app_get_packet_alarm_enable(void)
{
	return 0;
}

/*  机顶盒升级失败告警 */
int app_report_upgrade_alarm(int discrcode)
{
	return 0;
}

/* 加入频道失败告警	*/
int app_report_channel_alarm(int discrcode)
{
	return 0;
}


/* 机顶盒解码错误告警*/
int app_report_decode_alarm(void)
{
	return 0;
}

/* IPTV业务认证失败告警	 */
int app_report_authorize_alarm(int value)
{
	return 0;
}

int app_clear_decode_alarm(void)
{
	return 0;
}

int app_clear_authorize_alarm(void)
{

	return 0;
}
#endif
