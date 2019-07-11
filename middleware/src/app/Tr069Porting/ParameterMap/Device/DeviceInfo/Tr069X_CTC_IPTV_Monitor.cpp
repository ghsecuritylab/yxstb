#include "Tr069X_CTC_IPTV_Monitor.h"

#include "Tr069FunctionCall.h"

#include "Tr069.h"
#include "tr069_port_alarm.h"
#include "TR069Assertions.h"
extern "C"
{
#include "mid/mid_mutex.h"
#include "mid/mid_timer.h"
#include "mid/mid_ftp.h"
#include "mid/mid_time.h"
#include "mid/mid_tools.h"
}

#include "sys_basic_macro.h"
#include "NetworkFunctions.h"
#include "AppSetting.h"
#include "SysSetting.h"
#ifdef SQM_INCLUDED
#include "sqm_port.h"
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>


#ifdef TR069_MONITOR

static mid_mutex_t g_mutex = NULL;
static char *g_buf = NULL;
static int isPlay = 0;
struct IPTVMonitor g_realMonitor = {0, "", "300", 0, ""};
struct IPTVMonitor g_postMonitor = {0};
struct MonitorList g_monitorListData = {0};
struct MonitorPost g_monitorPostData = {0};
struct PlayMonitor g_oldProgram = {0};
struct PlayMonitor g_postProgram = {0};

#if defined(SQM_VERSION_C28) || defined (SQM_VERSION_ANDROID)
SQM_MSG_GET getStbData = {0};
#endif

/*------------------------------------------------------------------------------
	获取当前时间
 ------------------------------------------------------------------------------*/
//	返回值：sec
static unsigned int getCurrentTime(void) //tr069_port_get_time
{
    int TimeZone = 0;
    sysSettingGetInt("timezone", &TimeZone, 0);
    return (mid_time() + TimeZone * 3600);
}

#if defined(SQM_VERSION_C28) || defined(SQM_VERSION_ANDROID)
static int getSQMData(struct  SQM_GET_MSG *GetStbData,  int size ,int data_type ) //tr069_port_get_sqm_data
{

	if(data_type < 0 || data_type > 1)
		return -1;

	sqm_port_getmsg(GetStbData,  size, data_type );

	return 0;
}
#endif


#if defined(INCLUDE_SQM)
void monitor_sqm_data(int arg)
{
    static int alarmFlag = ALARM_RELIEVE;
    int timeLapseLimit = 0;
    int dfData = 0;
    int ret = 0;
    char alarmBuf[6];
#if defined(SQM_VERSION_C28) || defined(SQM_VERSION_ANDROID)
    ret = getSQMData(&getStbData, sizeof(getStbData), 0);
    if (ret == -1 ||getStbData.result == -1 || getStbData.result == -2) {
        memset(&getStbData, 0 ,sizeof(getStbData));
        ERR_OUT("Get sqm data failed!\n");
    }
    dfData = getStbData.StbData[0].df / 1000;
#endif

    tr069_api_getValue("Device.DeviceInfo.X_CTC_IPTV_Alarm.AlarmConfig.TimeLapseAlarmValue", alarmBuf, 6);
    timeLapseLimit = atoi(alarmBuf);
    if (timeLapseLimit <= 0)
        return;

    if (dfData <= timeLapseLimit && alarmFlag == ALARM_RINGING) {
        tr069_port_alarm_clear(ALARM_TYPE_Timelapse);
        alarmFlag = ALARM_RELIEVE;
    }

    if (dfData > timeLapseLimit && alarmFlag == ALARM_RELIEVE) {
        tr069_port_alarm_post(ALARM_TYPE_Timelapse,ALARM_TIME_LAPSE_CODE,ALARM_LEVEL_CRITICAL,"Time Lapse Exceeds The Threshold Alarm");
        alarmFlag = ALARM_RINGING;
    }

Err:
	return;
}
#endif

static int monitor_post_play_data(int type, int delay, const char *channelName, const char *channelAddress)
{
    if (type == TYPE_VOD)
        g_oldProgram.responseDelay = delay;
    else
        g_oldProgram.channelSwitchDelay = delay;
    if (NULL != channelName)
        strcpy(g_oldProgram.channelName, channelName);
    strcpy(g_oldProgram.channelAddress, channelAddress);

    return 0;
}

static void tr069_parse_postParamName(void) // 参数列表是否有用 ?
{
    int num = 0;
    int len = 0;
    int flag = 1;
    char *pos = NULL;
    char *buf = NULL;
    int *list = NULL;
    char monitorPostParam[64] = {0};

    list = g_monitorListData.list;

    if (g_realMonitor.parameterList[0] == '\0') {
        list[num++] = DEVICE_X_CTC_IPTV_Monitor_MdiMLR_INDEX;
        list[num++] = DEVICE_X_CTC_IPTV_Monitor_MdiDF_INDEX;
        list[num++] = DEVICE_X_CTC_IPTV_Monitor_Jitter_INDEX;
        list[num++] = DEVICE_X_CTC_IPTV_Monitor_CPURate_INDEX;
        list[num++] = DEVICE_X_CTC_IPTV_Monitor_MemRate_INDEX;

        list[num++] = DEVICE_X_CTC_IPTV_Monitor_AuthNumbers_INDEX;
        list[num++] = DEVICE_X_CTC_IPTV_Monitor_AuthFailNumbers_INDEX;
        list[num++] = DEVICE_X_CTC_IPTV_Monitor_MultiReqNumbers_INDEX;
        list[num++] = DEVICE_X_CTC_IPTV_Monitor_VodReqNumbers_INDEX;
        list[num++] = DEVICE_X_CTC_IPTV_Monitor_MultiFailNumbers_INDEX;
        list[num++] = DEVICE_X_CTC_IPTV_Monitor_VodFailNumbers_INDEX;

        list[num++] = DEVICE_X_CTC_IPTV_Monitor_BufUnderFlowNumbers_INDEX;
        list[num++] = DEVICE_X_CTC_IPTV_Monitor_BufOverFlowNumbers_INDEX;

	    list[num++] = DEVICE_X_CTC_IPTV_Monitor_ResponseDelay_INDEX;
        list[num++] = DEVICE_X_CTC_IPTV_Monitor_ChannelSwitchDelay_INDEX;
        list[num++] = DEVICE_X_CTC_IPTV_Monitor_ChannelName_INDEX;
        list[num++] = DEVICE_X_CTC_IPTV_Monitor_ChannelAddress_INDEX;
        list[num++] = DEVICE_X_CTC_IPTV_Monitor_Transmission_INDEX;
        list[num++] = DEVICE_X_CTC_IPTV_Monitor_ProgramStartTime_INDEX;
        list[num++] = DEVICE_X_CTC_IPTV_Monitor_ProgramEndTime_INDEX;
        list[num++] = DEVICE_X_CTC_IPTV_Monitor_BitRate_INDEX;
	    list[num++] = DEVICE_X_CTC_IPTV_Monitor_VideoQuality_INDEX;
        list[num++] = DEVICE_X_CTC_IPTV_Monitor_ChannelRequestFrequency_INDEX;
        list[num++] = DEVICE_X_CTC_IPTV_Monitor_AccessSuccessNumber_INDEX;
        list[num++] = DEVICE_X_CTC_IPTV_Monitor_AverageAccessTime_INDEX;
        list[num++] = DEVICE_X_CTC_IPTV_Monitor_WatchLong_INDEX;
        list[num++] = DEVICE_X_CTC_IPTV_Monitor_MediaStreamBandwidth_INDEX;

    } else {
        buf = g_realMonitor.parameterList;
        while (flag) {
            pos = strstr(buf, ",");
            if (pos == NULL) {
                strcpy(monitorPostParam, buf);
                flag = 0;
            } else {
                len = pos - buf;
                strncpy(monitorPostParam, buf, len);
                monitorPostParam[len] = '\0';
                buf = buf + len + 1;
            }
           if (strcasecmp(monitorPostParam, "MdiMLR") == 0)
               list[num++] = DEVICE_X_CTC_IPTV_Monitor_MdiMLR_INDEX;
           else if (strcasecmp(monitorPostParam, "MdiDF") == 0)
               list[num++] = DEVICE_X_CTC_IPTV_Monitor_MdiDF_INDEX;
           else if (strcasecmp(monitorPostParam, "Jitter") == 0)
               list[num++] = DEVICE_X_CTC_IPTV_Monitor_Jitter_INDEX;
           else if (strcasecmp(monitorPostParam, "CPURate") == 0)
               list[num++] = DEVICE_X_CTC_IPTV_Monitor_CPURate_INDEX;
           else if (strcasecmp(monitorPostParam, "MemRate") == 0)
               list[num++] = DEVICE_X_CTC_IPTV_Monitor_MemRate_INDEX;
           else if (strcasecmp(monitorPostParam, "AuthNumbers") == 0)
               list[num++] = DEVICE_X_CTC_IPTV_Monitor_AuthNumbers_INDEX;
           else if (strcasecmp(monitorPostParam, "AuthFailNumbers") == 0)
               list[num++] = DEVICE_X_CTC_IPTV_Monitor_AuthFailNumbers_INDEX;
           else if (strcasecmp(monitorPostParam, "MultiReqNumbers") == 0)
               list[num++] = DEVICE_X_CTC_IPTV_Monitor_MultiReqNumbers_INDEX;
           else if (strcasecmp(monitorPostParam, "VodReqNumbers") == 0)
               list[num++] = DEVICE_X_CTC_IPTV_Monitor_VodReqNumbers_INDEX;
           else if (strcasecmp(monitorPostParam, "MultiFailNumbers") == 0)
               list[num++] = DEVICE_X_CTC_IPTV_Monitor_MultiFailNumbers_INDEX;
           else if (strcasecmp(monitorPostParam, "VodFailNumbers") == 0)
               list[num++] = DEVICE_X_CTC_IPTV_Monitor_VodFailNumbers_INDEX;
           else if (strcasecmp(monitorPostParam, "BufUnderFlowNumbers") == 0)
               list[num++] = DEVICE_X_CTC_IPTV_Monitor_BufUnderFlowNumbers_INDEX;
           else if (strcasecmp(monitorPostParam, "BufOverFlowNumbers") == 0)
               list[num++] = DEVICE_X_CTC_IPTV_Monitor_BufOverFlowNumbers_INDEX;

           else if (strcasecmp(monitorPostParam, "ResponseDelay") == 0)
               list[num++] = DEVICE_X_CTC_IPTV_Monitor_ResponseDelay_INDEX;
           else if (strcasecmp(monitorPostParam, "ChannelSwitchDelay") == 0)
               list[num++] = DEVICE_X_CTC_IPTV_Monitor_ChannelSwitchDelay_INDEX;
           else if (strcasecmp(monitorPostParam, "ChannelName") == 0)
               list[num++] = DEVICE_X_CTC_IPTV_Monitor_ChannelName_INDEX;
           else if (strcasecmp(monitorPostParam, "ChannelAddress") == 0)
               list[num++] = DEVICE_X_CTC_IPTV_Monitor_ChannelAddress_INDEX;
           else if (strcasecmp(monitorPostParam, "Transmission") == 0)
               list[num++] = DEVICE_X_CTC_IPTV_Monitor_Transmission_INDEX;
           else if (strcasecmp(monitorPostParam, "ProgramStartTime") == 0)
               list[num++] = DEVICE_X_CTC_IPTV_Monitor_ProgramStartTime_INDEX;
           else if (strcasecmp(monitorPostParam, "ProgramEndTime") == 0)
               list[num++] = DEVICE_X_CTC_IPTV_Monitor_ProgramEndTime_INDEX;
           else if (strcasecmp(monitorPostParam, "BitRate") == 0)
               list[num++] = DEVICE_X_CTC_IPTV_Monitor_BitRate_INDEX;
           else if (strcasecmp(monitorPostParam, "VideoQuality") == 0)
               list[num++] = DEVICE_X_CTC_IPTV_Monitor_VideoQuality_INDEX;
           else if (strcasecmp(monitorPostParam, "ChannelRequestFrequency") == 0)
               list[num++] = DEVICE_X_CTC_IPTV_Monitor_ChannelRequestFrequency_INDEX;
           else if (strcasecmp(monitorPostParam, "AccessSuccessNumber") == 0)
               list[num++] = DEVICE_X_CTC_IPTV_Monitor_AccessSuccessNumber_INDEX;
           else if (strcasecmp(monitorPostParam, "AverageAccessTime") == 0)
               list[num++] = DEVICE_X_CTC_IPTV_Monitor_AverageAccessTime_INDEX;
           else if (strcasecmp(monitorPostParam, "WatchLong") == 0)
               list[num++] = DEVICE_X_CTC_IPTV_Monitor_WatchLong_INDEX;
           else if (strcasecmp(monitorPostParam, "MediaStreamBandwidth") == 0)
               list[num++] = DEVICE_X_CTC_IPTV_Monitor_MediaStreamBandwidth_INDEX;
        }
    }

    g_monitorListData.num = num;
    strcpy(g_postMonitor.parameterList, g_realMonitor.parameterList);

    return;
}

int monitor_post_play(int type, int delay, const char *channelName, const char *channelAddress)
{
	unsigned int newStartTime = 0;

	if (g_realMonitor.enable == 0)
		return 0;

	if ((type != TYPE_VOD &&  type != TYPE_CHANNEL) ||NULL == channelAddress)
		ERR_OUT("Parameter is not valid!\n");

	newStartTime = getCurrentTime();

       if (delay==0) {
           if (g_oldProgram.programStartTime == 0)
               return 0;
           if (newStartTime - g_oldProgram.programStartTime > MIN_PLAY_TIME) {
               g_oldProgram.programEndTime = newStartTime;
	        if (g_realMonitor.isFileorRealTime == 0) {
	            memcpy(&g_postProgram, &g_oldProgram, sizeof(struct PlayMonitor));
	            tr069_parse_postParamName();
				//TR069_EVENT_POST(TR069_EXTERN_EVENT_MONITOR);
	        }
           }
           memset(&g_oldProgram, 0, sizeof(g_oldProgram));
           isPlay = 0;
       } else {
           if (g_oldProgram.programStartTime && (newStartTime - g_oldProgram.programStartTime > MIN_PLAY_TIME)) {
               g_oldProgram.programEndTime = newStartTime;
	        if (g_realMonitor.isFileorRealTime == 0) {
	            memcpy(&g_postProgram, &g_oldProgram, sizeof(struct PlayMonitor));
	            tr069_parse_postParamName();
	            //TR069_EVENT_POST(TR069_EXTERN_EVENT_MONITOR);
	        }
           }
           g_oldProgram.programStartTime = newStartTime;
           monitor_post_play_data(type, delay, channelName, channelAddress);
           isPlay = 1;
       }

       return 0;
Err:
	return -1;
}

int monitor_play_bitrate(int bitrate)
{
	g_oldProgram.bitRate = bitrate;

	return 0;
}
/*------------------------------------------------------------------------------
	MAC地址
 ------------------------------------------------------------------------------*/
static void getMACAddress(char *value, int size) // tr069_port_get_MACAddress
{
    network_tokenmac_get(value, size, ':');
#if defined(Sichuan)
    int num;
    for (num =0; num<17; num++) {
        if ((*(value+num) >= 97) && (*(value+num) <= 122))
            *(value+num) = *(value + num)-32;
    }
#endif
}

int changeMacAddress(char *mac, char *newMac)
{
  	int i = 0, j = 0;
	while(mac[i]){
           if(mac[i] != ':')
               newMac[j++] = mac[i];
	    i++;
	}
	return 0;
}

static void monitor_statistic_file(char *fileName)
{
    int len = 0;
    char *p = NULL;
    char buf[32] = {0};
    unsigned int endPoint = 0;
    unsigned int startPoint = 0;
    u_int addr = 0;
    char ip[16] = {0};
    char macAddr[32] = {0};

    endPoint = mid_time();
    startPoint = endPoint - (unsigned int)g_realMonitor.logUploadInterval;
    mid_tool_time2string(startPoint, buf, 0);

    //mid_net_mac_addr(fileName, 0);
    getMACAddress(macAddr, 32);
    changeMacAddress(macAddr, fileName);

    len = strlen(fileName);
    fileName[len] = '_';
    appSettingGetString("ntvuser", fileName + len + 1, 32, 0);
    p = fileName + len + 1;

    while (*p++) {
        if (*p == '@')
            *p = '_';
    }

    //mid_net_addr_get(ip);
	char ifname[URL_LEN] = { 0 };
	network_default_ifname(ifname, URL_LEN);
	network_address_get(ifname, ip, URL_LEN);
    addr = inet_addr(ip);
    len = strlen(fileName);
    len += snprintf(fileName + len, LOG_SERVER_URL_SIZE - len, "_%03d-%03d-%03d-%03d",
                             addr & 0xff, (addr >> 8) & 0xff, (addr >> 16) & 0xff, (addr >> 24) & 0xff);
    len += snprintf(fileName + len, LOG_SERVER_URL_SIZE - len, "_%s", buf);
    snprintf(fileName + len, LOG_SERVER_URL_SIZE - len, "_%06d.csv", atoi(g_realMonitor.logUploadInterval));
    len = 0;
    len += snprintf(g_buf + len, MONITOR_STATISTIC_FILE_SIZE - len, "MdiMLR,%d\n", app_tr069_port_Monitor_get_MdiMLR());
    len += snprintf(g_buf + len, MONITOR_STATISTIC_FILE_SIZE - len, "MdiDF,%d\n", app_tr069_port_Monitor_get_MdiDF());
    len += snprintf(g_buf + len, MONITOR_STATISTIC_FILE_SIZE - len, "Jitter,%d\n", app_tr069_port_Monitor_get_Jitter());
    len += snprintf(g_buf + len, MONITOR_STATISTIC_FILE_SIZE - len, "CPURate,%d\n",  tr069_port_get_CPUrate());
    len += snprintf(g_buf + len, MONITOR_STATISTIC_FILE_SIZE - len, "MemRate,%d\n", tr069_port_get_memValue());

    len += snprintf(g_buf + len, MONITOR_STATISTIC_FILE_SIZE - len, "AuthNumbers,%d\n", tr069_statistic_get_AuthNumbers());
    len += snprintf(g_buf + len, MONITOR_STATISTIC_FILE_SIZE - len, "AuthFailNumbers,%d\n", tr069_statistic_get_AuthFailNumbers());
    len += snprintf(g_buf + len, MONITOR_STATISTIC_FILE_SIZE - len, "MultiReqNumbers,%d\n", tr069_statistic_get_MultiReqNumbers());
    len += snprintf(g_buf + len, MONITOR_STATISTIC_FILE_SIZE - len, "VodReqNumbers,%d\n", tr069_statistic_get_VodReqNumbers());
    len += snprintf(g_buf + len, MONITOR_STATISTIC_FILE_SIZE - len, "MultiFailNumbers,%d\n", g_monitorPostData.multiFailNumbers); // 总次数还是统计周期内的次数
    len += snprintf(g_buf + len, MONITOR_STATISTIC_FILE_SIZE - len, "VodFailNumbers,%d\n", g_monitorPostData.vodFailNumbers); // 
    len += snprintf(g_buf + len, MONITOR_STATISTIC_FILE_SIZE - len, "BufUnderFlowNumbers,%d\n", g_monitorPostData.bufUnderFlowNumbers);
    len += snprintf(g_buf + len, MONITOR_STATISTIC_FILE_SIZE - len, "BufOverFlowNumbers,%d\n", g_monitorPostData.bufOverFlowNumbers);

    len += snprintf(g_buf + len, MONITOR_STATISTIC_FILE_SIZE - len, "ResponseDelay,%d\n", g_oldProgram.responseDelay);
    len += snprintf(g_buf + len, MONITOR_STATISTIC_FILE_SIZE - len, "ChannelSwitchDelay,%d\n", g_oldProgram.channelSwitchDelay);
    len += snprintf(g_buf + len, MONITOR_STATISTIC_FILE_SIZE - len, "ChannelName,%s\n", g_oldProgram.channelName);
    len += snprintf(g_buf + len, MONITOR_STATISTIC_FILE_SIZE - len, "ChannelAddress,%s\n", g_oldProgram.channelAddress);
    app_tr069_port_Monitor_get_Transmission(buf, sizeof(buf));
    len += snprintf(g_buf + len, MONITOR_STATISTIC_FILE_SIZE - len, "Transmission,%s\n", buf);
    mid_tool_time2string(g_oldProgram.programStartTime, buf, ' ');
    len += snprintf(g_buf + len, MONITOR_STATISTIC_FILE_SIZE - len, "ProgramStartTime,%s\n", buf);
    mid_tool_time2string(g_oldProgram.programEndTime, buf, ' ');
    len += snprintf(g_buf + len, MONITOR_STATISTIC_FILE_SIZE - len, "ProgramEndTime,%s\n", buf);
    len += snprintf(g_buf + len, MONITOR_STATISTIC_FILE_SIZE - len, "BitRate,%d\n", g_oldProgram.bitRate);

    len += snprintf(g_buf + len, MONITOR_STATISTIC_FILE_SIZE - len, "VideoQuality,%s\n", g_oldProgram.VideoQuality);
    len += snprintf(g_buf + len, MONITOR_STATISTIC_FILE_SIZE - len, "ChannelRequestFrequency,%d\n", g_oldProgram.ChannelRequestFrequency);
    len += snprintf(g_buf + len, MONITOR_STATISTIC_FILE_SIZE - len, "AccessSuccessNumber,%d\n", g_oldProgram.AccessSuccessNumber);
    len += snprintf(g_buf + len, MONITOR_STATISTIC_FILE_SIZE - len, "AverageAccessTime,%s\n", g_oldProgram.AverageAccessTime);
    len += snprintf(g_buf + len, MONITOR_STATISTIC_FILE_SIZE - len, "WatchLong,%d\n", g_oldProgram.WatchLong);
    len += snprintf(g_buf + len, MONITOR_STATISTIC_FILE_SIZE - len, "MediaStreamBandwidth,%d\n", g_oldProgram.MediaStreamBandwidth);

    g_buf[MONITOR_STATISTIC_FILE_SIZE] = 0;
    memset(&g_monitorPostData,0,sizeof(g_monitorPostData));

    return;
}
static void monitor_post_timer(int arg)
{
    char *username = "ftp";
    char *password = "ftp";
    char fileName[128] = {0};
    char ftpUrl[LOG_SERVER_URL_SIZE] = "ftp://133.37.66.22/home/ftpitms";
    int len = 0;
    unsigned int postTime = 0;
    monitor_sqm_data(0);

    if (isPlay == 1) {
		postTime = getCurrentTime();
		if (postTime - g_oldProgram.programStartTime <= MIN_PLAY_TIME) {
			//isPlay = 0;
		} else if (postTime - g_oldProgram.programStartTime < atoi(g_realMonitor.logUploadInterval)) {
			g_oldProgram.programEndTime = postTime;
		} else {
			g_oldProgram.programStartTime = postTime - atoi(g_realMonitor.logUploadInterval);
			g_oldProgram.programEndTime = postTime;
		}
		memcpy(&g_postProgram, &g_oldProgram, sizeof(struct PlayMonitor));
    }

    if (g_monitorListData.num == 0 || strcmp(g_realMonitor.parameterList, g_postMonitor.parameterList))
        tr069_parse_postParamName();
    if (g_realMonitor.isFileorRealTime == 0) {  //using tr069 protocol to post
        //TR069_EVENT_POST(TR069_EXTERN_EVENT_MONITOR);
    } else {											               //using ftp protocol to post
        monitor_statistic_file(fileName);
        len = strlen(g_buf);
        strcat(ftpUrl,"/");
        strcat(ftpUrl,fileName);
        if (mid_ftp_post(ftpUrl, username, password, g_buf, len)) {
            tr069_port_alarm_post(ALARM_TYPE_File_Access,ALARM_FILE_ACCESS_CODE,ALARM_LEVEL_CRITICAL,"Access To File Server Fail");
            ERR_OUT("mid_ftp_post failed! %s,%s,%s\n", ftpUrl, username, password);
        }
    }

Err:
    return;
}

static void monitor_change_timer(int arg)
{
    struct IPTVMonitor monitor;
    int dealTimer = 0;
    if (g_realMonitor.logUploadInterval[0] == 0)
        return;
    dealTimer = atoi(g_realMonitor.logUploadInterval);
    memcpy(&monitor, &g_realMonitor, sizeof(struct IPTVMonitor));
    if (monitor.enable == g_postMonitor.enable && !strcmp(monitor.logUploadInterval, g_postMonitor.logUploadInterval))
        return;

    memset(&g_monitorPostData,0,sizeof(g_monitorPostData));
    if (monitor.enable == 1 && atoi(monitor.logUploadInterval) > 0 && atoi(monitor.logUploadInterval) < 30 * 24 * 60)
        mid_timer_create(dealTimer, 0, monitor_post_timer, 0);
    else
        mid_timer_delete(monitor_post_timer, 0);

    memcpy(&g_postMonitor, &monitor, sizeof(struct IPTVMonitor));

    return;
}

static void monitor_disable_post(int arg)
{
    mid_timer_delete(monitor_post_timer, 0);
    memset(&g_postMonitor, 0, sizeof(struct IPTVMonitor));
    memset(&g_realMonitor, 0, sizeof(struct IPTVMonitor));
    memset(&g_monitorPostData, 0, sizeof(g_monitorPostData));
    memset(&g_postProgram, 0, sizeof(g_postProgram));

    return;
}

int tr069_port_extern_monitor_params(int *list, int size)
{
    int num = 0;
    int tIndex = 0;

    for (tIndex = 0; tIndex < g_monitorListData.num; tIndex++)
        list[num++] = g_monitorListData.list[tIndex];

    return num;
}

void tr069_port_extern_monitor_succeed(void)
{
    memset(&g_monitorPostData, 0, sizeof(g_monitorPostData));
    memset(&g_postProgram, 0, sizeof(g_postProgram));

    return;
}

#ifdef SQM_VERSION_C28
int app_tr069_port_Monitor_get_MdiMLR(void)
{
    g_monitorPostData.mdiMLR = getStbData.StbData[0].mlr/100000;
    return g_monitorPostData.mdiMLR;
}

int app_tr069_port_Monitor_get_MdiDF(void)
{
    g_monitorPostData.mdiDF = getStbData.StbData[0].df/1000;
    return g_monitorPostData.mdiDF;
}

int app_tr069_port_Monitor_get_Jitter(void)
{
    g_monitorPostData.jitter = getStbData.StbData[0].jitter/1000;
    return g_monitorPostData.jitter;
}
#else
int app_tr069_port_Monitor_get_MdiMLR(void)
{
    return 0;
}

int app_tr069_port_Monitor_get_MdiDF(void)
{
    return 0;
}
int app_tr069_port_Monitor_get_Jitter(void)
{
    return 0;
}
#endif

void app_tr069_port_Monitor_get_Transmission(char *value, int size)
{
	int flag = 0;
	sysSettingGetInt("TransportProtocol", &flag, 0);

	switch (flag) {
	case 0:
       case 4:
	   	strcpy(value, "TCP");
		break;
	case 1:
	case 6:
		strcpy(value, "UDP");
		break;
	case 2:
	case 5:
		strcpy(value, "RTP/TCP");
		break;
	case 3:
	case 7:
		strcpy(value, "RTP/UDP");
		break;
	default:
		break;
	}

	return;
}

int app_tr069_port_Monitor_get_BufUnderFlowNumbers(void)
{
    return g_monitorPostData.bufUnderFlowNumbers;
}

int app_tr069_port_Monitor_get_BufOverFlowNumbers(void)
{
    return g_monitorPostData.bufOverFlowNumbers;
}

int app_tr069_port_Monitor_get_ResponseDelay(void)
{
	return g_postProgram.responseDelay;
}

int app_tr069_port_Monitor_get_ChannelSwitchDelay(void)
{
	return g_postProgram.channelSwitchDelay;
}

void app_tr069_port_Monitor_get_ChannelName(char *value, int size)
{
	strcpy(value, g_postProgram.channelName);
	return;
}

void app_tr069_port_Monitor_get_ChannelAddress(char *value, int size)
{
	strcpy(value, g_postProgram.channelAddress);
	return;
}


void app_tr069_port_Monitor_get_ProgramStartTime(char *value, int size)
{
	mid_tool_time2string(g_postProgram.programStartTime, value, 0);
	return;
}

void app_tr069_port_Monitor_get_ProgramEndTime(char *value, int size)
{
	mid_tool_time2string(g_postProgram.programEndTime, value, 0);
	return;
}

int app_tr069_port_Monitor_get_BitRate(void)
{
	return g_postProgram.bitRate;
}

/*四川电信规范3.0+监控参数，可选实现的参数，暂预留接口*/
void app_tr069_port_Monitor_get_VideoQuality(char *value, int size)
{
    strcpy(value, g_postProgram.VideoQuality);
}
int app_tr069_port_Monitor_get_ChannelRequestFrequency(void)
{
    return g_postProgram.ChannelRequestFrequency;
}

int app_tr069_port_Monitor_get_AccessSuccessNumber(void)
{
    return g_postProgram.AccessSuccessNumber;
}

void app_tr069_port_Monitor_get_AverageAccessTime(char *value, int size)
{
    strcpy(value, g_postProgram.AverageAccessTime);
}

int app_tr069_port_Monitor_get_WatchLong(void)
{
    return g_postProgram.WatchLong;
}

int app_tr069_port_Monitor_get_MediaStreamBandwidth(void)
{
    return g_postProgram.MediaStreamBandwidth;
}


void monitor_cycle_statistics(int flag)
{
    switch (flag) {
    case FLAG_MULTIFAILNUM:
        g_monitorPostData.multiFailNumbers++;
        break;
    case FLAG_VODFAILNUM:
        g_monitorPostData.vodFailNumbers++;
        break;
    case FLAG_BUFINCNUM:
        g_monitorPostData.bufOverFlowNumbers++;
        break;
    case FLAG_BUFDECNUM:
        g_monitorPostData.bufUnderFlowNumbers++;
        break;
    default:
        break;
    }

    return;
}


int app_monitor_statistic_config_init(void)
{
    if (g_buf != NULL)
        ERR_OUT("g_buf ALREADY INIT!");
    g_buf = (char *)malloc(MONITOR_STATISTIC_FILE_SIZE + 4);
    if(g_buf == NULL)
        ERR_OUT("os_malloc MONITOR_STATISTIC_FILE_SIZE = %d failed\n", MONITOR_STATISTIC_FILE_SIZE);
    g_mutex = mid_mutex_create( );
    if (g_mutex == NULL)
        ERR_OUT("logmutex create failed\n");

    return 0;
Err:
    return -1;
}

////////////////////////////
static int getTr069PortCTCIPTVMonitorEnable(char* str, unsigned int val)
{
    snprintf(str, val, "%d", g_realMonitor.enable);

    return 0;
}

static int setTr069PortCTCIPTVMonitorEnable(char* str, unsigned int val)
{
	if (mid_mutex_lock(g_mutex))
		 WARN_PRN("\n");
	g_realMonitor.enable = atoi(str);
	mid_mutex_unlock(g_mutex);

	mid_timer_create(1, 1, monitor_change_timer, 0);

    return 0;
}

static int getTr069PortCTCIPTVMonitorTimeList(char* str, unsigned int val)
{
    strcpy(str, g_realMonitor.timeList);

    return 0;
}

static int setTr069PortCTCIPTVMonitorTimeList(char* str, unsigned int val)
{
    if (mid_mutex_lock(g_mutex))
        WARN_PRN("\n");
    strcpy(g_realMonitor.timeList, str);
    mid_timer_create(atoi(g_realMonitor.timeList), 1, monitor_disable_post, 0);
    mid_mutex_unlock(g_mutex);

    return 0;
}

static int getTr069PortCTCIPTVMonitorLogUploadInterval(char* str, unsigned int val)
{
    strcpy(str, g_realMonitor.logUploadInterval);
	
    return 0;
}

static int setTr069PortCTCIPTVMonitorLogUploadInterval(char* str, unsigned int val)
{
    if (mid_mutex_lock(g_mutex))
        WARN_PRN("\n");
    strcpy(g_realMonitor.logUploadInterval, str);
    mid_timer_create(1, 1, monitor_change_timer, 0);
    mid_mutex_unlock(g_mutex);

    return 0;
}

static int getTr069PortCTCIPTVMonitorIsFileorRealTime(char* str, unsigned int val)
{
    snprintf(str, val, "%d", g_realMonitor.isFileorRealTime);

    return 0;
}

static int setTr069PortCTCIPTVMonitorIsFileorRealTime(char* str, unsigned int val)
{
    if (mid_mutex_lock(g_mutex))
        WARN_PRN("\n");
    g_realMonitor.isFileorRealTime = atoi(str); // ftp or tr069 event
    mid_mutex_unlock(g_mutex);

    mid_timer_create(1, 1, monitor_change_timer, 0);
    return 0;
}

static int getTr069PortCTCIPTVMonitorParameterList(char* str, unsigned int val)
{
	strcpy(str, g_realMonitor.parameterList);

    return 0;
}

static int setTr069PortCTCIPTVMonitorParameterList(char* str, unsigned int val)
{
	if (mid_mutex_lock(g_mutex))
		   WARN_PRN("\n");
	   strcpy(g_realMonitor.parameterList, str);	//	parameterList  useless
	   mid_mutex_unlock(g_mutex);
    return 0;
}

#endif //TR069_MONITOR

Tr069X_CTC_IPTV_Monitor::Tr069X_CTC_IPTV_Monitor()
	: Tr069GroupCall("X_CTC_IPTV_Monitor")
{
#ifdef TR069_MONITOR

    Tr069Call* Enable            = new Tr069FunctionCall("Enable", getTr069PortCTCIPTVMonitorEnable, setTr069PortCTCIPTVMonitorEnable);
    Tr069Call* TimeList          = new Tr069FunctionCall("TimeList", getTr069PortCTCIPTVMonitorTimeList, setTr069PortCTCIPTVMonitorTimeList);
    Tr069Call* LogUploadInterval = new Tr069FunctionCall("LogUploadInterval", getTr069PortCTCIPTVMonitorLogUploadInterval, setTr069PortCTCIPTVMonitorLogUploadInterval);
    Tr069Call* IsFileorRealTime  = new Tr069FunctionCall("IsFileorRealTime", getTr069PortCTCIPTVMonitorIsFileorRealTime, setTr069PortCTCIPTVMonitorIsFileorRealTime);
    Tr069Call* ParameterList     = new Tr069FunctionCall("ParameterList", getTr069PortCTCIPTVMonitorParameterList, setTr069PortCTCIPTVMonitorParameterList);

    regist(Enable->name(), Enable);
    regist(TimeList->name(), TimeList);
    regist(LogUploadInterval->name(), LogUploadInterval);
    regist(IsFileorRealTime->name(), IsFileorRealTime);
    regist(ParameterList->name(), ParameterList);

#endif

}

Tr069X_CTC_IPTV_Monitor::~Tr069X_CTC_IPTV_Monitor()
{
}
