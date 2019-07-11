#include "Tr069LogMsg.h"

#include "TR069Assertions.h"
#include "Tr069FunctionCall.h"

#include "Tr069.h"

#include "sys_basic_macro.h"
#include "NetworkFunctions.h"
#include "AppSetting.h"
extern "C"
{
#include "mid_sys.h"
#include "mid/mid_tools.h"
#include "mid/mid_ftp.h"
#include "mid/mid_timer.h"
#include "mid/mid_mutex.h"
#include "mid/mid_time.h"
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define LOG_FILE_SIZE         (128 * 1024)

#define LOG_FTP_SERVER_LEN        1024
#define LOG_FTP_USER_LEN           32
#define LOG_FTP_PASSWORD_LEN       32
#define LOG_INFO_NUM           	   16
#define LOG_INFO_SIZE             256
#define LOG_TR069_BUF_SIZE        2048


struct RealMsg
{
    int Enable;
    int MsgOrFile;
};

struct LogMsg
{
    unsigned  int Starttime;
    unsigned  int  Endtime;
    char LogFtpServer[LOG_FTP_SERVER_LEN];
    char LogFtpUser[LOG_FTP_USER_LEN];
    char LogFtpPassword[LOG_FTP_PASSWORD_LEN];
    int Duration;
    int  RTSPInfoNum;
    char RTSPInfo[LOG_INFO_NUM][LOG_INFO_SIZE];
    int  HTTPInfoNum;
    char HTTPInfo[LOG_INFO_NUM][LOG_INFO_SIZE];
    int  IGMPInfoNum;
    char IGMPInfo[LOG_INFO_NUM][LOG_INFO_SIZE];
    int PkgTotalOneSec;
    int ByteTotalOneSec;
    unsigned PkgLostRate;
    unsigned AvarageRate;
    unsigned BUFFER;
    int  ERRNum;
    char ERR[LOG_INFO_NUM][LOG_INFO_SIZE];
    char VendorExt[LOG_INFO_SIZE];
};

static const char paramname[LOG_MSG_MAX][16]=
{
    "RTSPInfo",
    "HTTPInfo",
    "IGMPInfo",
    "PkgTotalOneSec",
    "ByteTotalOneSec",
    "PkgLostRate",
    "AvarageRate",
    "BUFFER",
    "ERROR",
    "VendorExt"
};
static struct RealMsg g_realmsg;
static struct LogMsg g_logmsg;
static mid_mutex_t g_logmutex = NULL;
static char *g_logbuf = NULL;
static int g_loglen = 0;


static void time_transfor_string(char *timestring)
{
    time_t sec;
    struct tm t;
    sec = mid_time();
    gmtime_r(&sec, &t);
    sprintf(timestring, "%02d-%02d-%02d:%02d-%02d-%02d",
                        (t.tm_year + 1900) % 100,
                         t.tm_mon + 1,
                         t.tm_mday,
                         t.tm_hour,
                         t.tm_min,
                         t.tm_sec);

    return;
}

void tr069LogMsgStatisticInfo(int flag, char *info)
{
    char timestring[64] = {0};

    if (!g_realmsg.Enable)
        return;
    mid_mutex_lock(g_logmutex);
    if (!g_realmsg.MsgOrFile)
        time_transfor_string(timestring);

    tr069LogMsgPost(timestring, flag, info);
    mid_mutex_unlock(g_logmutex);

    return;
}

int tr069LogMsgInit(void)
{
    if(g_logbuf != NULL) {
        LogTr069Error("already init!\n");
        return -1;
    }

    g_logbuf = (char *)malloc(LOG_FILE_SIZE + 4);
    if(g_logbuf == NULL) {
        LogTr069Error("os_malloc LOG_FILE_SIZE = %d failed\n", LOG_FILE_SIZE);
        return -1;
    }

    g_logmutex = mid_mutex_create();
    if(g_logmutex == NULL) {
        LogTr069Error("logmutex create failed\n");
        return -1;
    }

    memset(&g_logmsg, 0, sizeof(g_logmsg));
    memset(&g_realmsg, 0, sizeof(g_realmsg));

    return 0;
}

static void logMsgReset(void)
{
    g_logmsg.RTSPInfoNum = 0;
    g_logmsg.HTTPInfoNum = 0;
    g_logmsg.IGMPInfoNum = 0;
    g_logmsg.PkgTotalOneSec = 0;
    g_logmsg.ByteTotalOneSec = 0;
    g_logmsg.PkgLostRate = 0;
    g_logmsg.AvarageRate = 0;
    g_logmsg.BUFFER = 0;
    g_logmsg.ERRNum = 0;
}

static void logMsgPostFile(void)
{
    int i, len;
    char *p = NULL;
    char buf[64] = {0};
    char ifname[URL_LEN] = { 0 };
    char logFtpServer[LOG_FTP_SERVER_LEN];
    network_default_ifname(ifname, URL_LEN);

    g_logmsg.Endtime = mid_time();

    strcpy(logFtpServer, g_logmsg.LogFtpServer);

    p = strstr(logFtpServer, "://");
    if (p) {
        if((p = strrchr(p + 3, '/')) != NULL)
            p[0] = 0;
    
        len = strlen(logFtpServer);
        strcpy(logFtpServer + len, "/");
        appSettingGetString("ntvuser", logFtpServer + len + 1, USER_LEN, 0);
        len = strlen(logFtpServer);
        mid_sys_serial(buf);
        len += snprintf(logFtpServer + len, LOG_FTP_SERVER_LEN - len, "_%s", buf);
        network_tokenmac_get(buf, 64, '-');
        len += snprintf(logFtpServer + len, LOG_FTP_SERVER_LEN - len, "_%s", buf);
        network_address_get(ifname, buf, 64);
        for(i = 0; buf[i] != 0; i++) {
            if(buf[i] == '.')
                buf[i] = '-';
        }

        len += snprintf(logFtpServer + len, LOG_FTP_SERVER_LEN - len, "_%s", buf);
        mid_tool_time2string(g_logmsg.Starttime, buf, 0);
        len += snprintf(logFtpServer + len, LOG_FTP_SERVER_LEN - len, "_%s", buf);
        mid_tool_time2string(g_logmsg.Endtime, buf, 0);
        len += snprintf(logFtpServer + len, LOG_FTP_SERVER_LEN - len, "_%s.log", buf);

        if(mid_ftp_post(logFtpServer, g_logmsg.LogFtpUser, g_logmsg.LogFtpPassword, g_logbuf, (int)strlen(g_logbuf))) {
#ifdef Sichuan
            app_report_file_access_alarm( );
#endif
            LogTr069Error("mid_ftp_post failed! %s,%s,%s\n", logFtpServer, g_logmsg.LogFtpUser, g_logmsg.LogFtpPassword);
        }
    } else {
        LogTr069Error("LogFtpServer url format is invalid!\n");
    }

    memset(g_logbuf, 0, LOG_FILE_SIZE + 4);
    g_loglen = 0;
    g_logmsg.Starttime = mid_time();
}

int tr069LogMsgPost(char *tm, int type, char *info)
{
    if(g_realmsg.MsgOrFile) {
        switch(type) {
        case LOG_MSG_PKGTOTALONESEC:
            g_logmsg.PkgTotalOneSec = atoi(info);
            break;
        case LOG_MSG_PKGLOSTRATE:
            g_logmsg.PkgLostRate = atoi(info);

            break;
        case LOG_MSG_BYTETOTALONESEC:
            g_logmsg.ByteTotalOneSec = atoi(info);

            break;
        case LOG_MSG_BUFFER:
            g_logmsg.BUFFER = atoi(info);
            break;
        case LOG_MSG_AVARAGERATE:
            g_logmsg.AvarageRate = atoi(info);
            break;
        case LOG_MSG_HTTPINFO:
            snprintf(g_logmsg.HTTPInfo[g_logmsg.HTTPInfoNum % LOG_INFO_NUM], LOG_INFO_SIZE, info);
            g_logmsg.HTTPInfoNum++;
            break;
        case LOG_MSG_RTSPINFO:
            snprintf(g_logmsg.RTSPInfo[g_logmsg.RTSPInfoNum % LOG_INFO_NUM], LOG_INFO_SIZE, info);
            g_logmsg.RTSPInfoNum++;
            break;
        case LOG_MSG_IGMPINFO:
            snprintf(g_logmsg.IGMPInfo[g_logmsg.IGMPInfoNum % LOG_INFO_NUM], LOG_INFO_SIZE, info);
            g_logmsg.IGMPInfoNum++;
            break;
        case LOG_MSG_ERROR:
            snprintf(g_logmsg.ERR[g_logmsg.ERRNum % LOG_INFO_NUM], LOG_INFO_SIZE, info);
            g_logmsg.ERRNum++;
            break;
        default:
            break;
        }
    } else {
        if(*g_logmsg.LogFtpServer) {
            if((g_loglen + strlen(info) + 1) > LOG_FILE_SIZE)
                logMsgPostFile( );

            g_loglen += snprintf(g_logbuf + g_loglen, LOG_FILE_SIZE - g_loglen, "%s|%s|%s\n", tm, paramname[type], info);
            LogTr069Debug("logMsgLength = %d / %d\n", g_loglen, LOG_FILE_SIZE);
        } else {
        }
    }

    return 0;
}

void tr069LogMsgPostHTTPInfo(char *info_buf, int info_len)
{
	tr069LogMsgStatisticInfo(LOG_MSG_HTTPINFO, info_buf);
}

void tr069LogMsgPostRTSPInfo(int multflg, char *info_buf, int info_len)
{
    if (multflg)
        tr069LogMsgStatisticInfo(LOG_MSG_IGMPINFO, info_buf);
    else
        tr069LogMsgStatisticInfo(LOG_MSG_RTSPINFO, info_buf);
}

static void app_tr069_logmsg_disable(int arg)
{
    mid_mutex_lock(g_logmutex);
    g_realmsg.Enable = 0;
    if (!g_realmsg.MsgOrFile && g_loglen > 0)
        logMsgPostFile( );
    mid_mutex_unlock(g_logmutex);
}

static void app_tr069_logmsg_periodic(int arg)
{
    mid_mutex_lock(g_logmutex);
    if (g_realmsg.Enable && g_realmsg.MsgOrFile) {
        int eventID = TR069_API_SETVALUE("Event.Regist", "M CTC LOG_PERIODIC", 0);
        TR069_API_SETVALUE((char*)"Event.Parameter", (char*)"Device.X_CTC_IPTV.LogMsg.RTSPInfo",        eventID);
        TR069_API_SETVALUE((char*)"Event.Parameter", (char*)"Device.X_CTC_IPTV.LogMsg.HTTPInfo",        eventID);
        TR069_API_SETVALUE((char*)"Event.Parameter", (char*)"Device.X_CTC_IPTV.LogMsg.IGMPInfo",        eventID);
        TR069_API_SETVALUE((char*)"Event.Parameter", (char*)"Device.X_CTC_IPTV.LogMsg.PkgTotalOneSec",  eventID);
        TR069_API_SETVALUE((char*)"Event.Parameter", (char*)"Device.X_CTC_IPTV.LogMsg.ByteTotalOneSec", eventID);
        TR069_API_SETVALUE((char*)"Event.Parameter", (char*)"Device.X_CTC_IPTV.LogMsg.PkgLostRate",     eventID);
        TR069_API_SETVALUE((char*)"Event.Parameter", (char*)"Device.X_CTC_IPTV.LogMsg.AvarageRate",     eventID);
        TR069_API_SETVALUE((char*)"Event.Parameter", (char*)"Device.X_CTC_IPTV.LogMsg.BUFFER",          eventID);
        TR069_API_SETVALUE((char*)"Event.Parameter", (char*)"Device.X_CTC_IPTV.LogMsg.ERROR",           eventID);
        TR069_API_SETVALUE((char*)"Event.Post", (char*)"", eventID);
    } else {
        mid_timer_delete(app_tr069_logmsg_periodic, 0);
    }
    mid_mutex_unlock(g_logmutex);

    return ;
}
u_int tr069LogMsgGetEnable(void)
{
    unsigned enable = 0;
    mid_mutex_lock(g_logmutex);
    enable = g_realmsg.Enable;
    mid_mutex_unlock(g_logmutex);
    return enable;
}


///////////////////////////////////////////////////////////////////////
/*------------------------------------------------------------------------------
 ------------------------------------------------------------------------------*/
static int getTr069PortEnable(char* value, unsigned int size)
{
    if (snprintf(value, size,"%u", tr069LogMsgGetEnable()) >= (int)size) {
        printf("Error.size is too short.");
        return -1;
    }	 

    return 0;
}
 
static int setTr069PortEnable(char* value, unsigned int size) //tr069_log_set_Enable
{
    mid_mutex_lock(g_logmutex);
    g_realmsg.Enable = atoi(value);

    if (value) {
        logMsgReset( );
        g_loglen = 0;
        g_logmsg.Starttime = mid_time( );
        mid_timer_create(10, 0, app_tr069_logmsg_periodic, 0);
    }

    mid_mutex_unlock(g_logmutex);
  
    return 0;
}

/*------------------------------------------------------------------------------
 ------------------------------------------------------------------------------*/
static int getTr069PortMsgOrFile(char* value, unsigned int size)
{
    if (snprintf(value, size,"%u", g_realmsg.MsgOrFile) >= (int)size) {
        printf("Error.size is too short.");
        return -1;
    }	 

    return 0;
}

static int setTr069PortMsgOrFile(char* value, unsigned int size)
{
    g_realmsg.MsgOrFile = atoi(value);
  
    return 0;
}

/*------------------------------------------------------------------------------
 ------------------------------------------------------------------------------*/
static int getTr069PortDuration(char* value, unsigned int size)
{
    if (snprintf(value, size,"%d", g_logmsg.Duration) >= (int)size) {
        printf("Error.size is too short.");
        return -1;
    }	 

    return 0;
}

static int setTr069PortDuration(char* value, unsigned int size) //tr069_log_set_Duration
{
    g_logmsg.Duration = atoi(value);
    mid_timer_delete(app_tr069_logmsg_disable, 0);
    mid_timer_create(atoi(value), 1, app_tr069_logmsg_disable, 0);  
    return 0;
}

/*------------------------------------------------------------------------------
 ------------------------------------------------------------------------------*/
static int getTr069PortLogFtpServer(char* value, unsigned int size) //tr069_log_get_LogFtpServer
{
    char buf[LOG_FTP_SERVER_LEN] = {0};
    char *p = NULL;

    if(value) {
        strcpy(buf, g_logmsg.LogFtpServer);
        if((p = strrchr(buf + 6, '/')) != NULL)
            p[0] = 0;
        strcpy(value, buf);
    }
    return 0;
}

static int setTr069PortLogFtpServer(char* value, unsigned int size)
{
    strcpy(g_logmsg.LogFtpServer, value);
    return 0;
}

/*------------------------------------------------------------------------------
 ------------------------------------------------------------------------------*/
static int getTr069PortLogFtpUser(char* value, unsigned int size) //tr069_log_get_LogFtpUser
{
    if(value)
        strcpy(value, g_logmsg.LogFtpUser);
    return 0;
}

static int setTr069PortLogFtpUser(char* value, unsigned int size) //tr069_log_set_LogFtpUser
{
    strcpy(g_logmsg.LogFtpUser, value);
    return 0;
}

/*------------------------------------------------------------------------------
 ------------------------------------------------------------------------------*/
static int getTr069PortLogFtpPassword(char* value, unsigned int size) //tr069_log_get_LogFtpPassword
{
    if(value)
        strcpy(value, g_logmsg.LogFtpPassword);
    return 0;
}

static int setTr069PortLogFtpPassword(char* value, unsigned int size) //tr069_log_set_LogFtpPassword
{
    strcpy(g_logmsg.LogFtpPassword, value); 
  
    return 0;
}

/*------------------------------------------------------------------------------
 ------------------------------------------------------------------------------*/
static int getTr069PortRTSPInfo(char* value, unsigned int size)
{
    int i = 0, num = 0, len = 0;

    mid_mutex_lock(g_logmutex);
    if(g_logmsg.RTSPInfoNum <= LOG_INFO_NUM)
        num = g_logmsg.RTSPInfoNum;
    else
        num = LOG_INFO_NUM;

    for(i = 0; i < num; i++)
        len += snprintf(value + len, LOG_TR069_BUF_SIZE - len, "%s", g_logmsg.RTSPInfo[i]);
    g_logmsg.RTSPInfoNum = 0;
    mid_mutex_unlock(g_logmutex);


    return 0;
}

/*------------------------------------------------------------------------------
 ------------------------------------------------------------------------------*/
static int getTr069PortHTTPInfo(char* value, unsigned int size) //tr069_log_get_HTTPInfo
{
    int i = 0, num = 0, len = 0;

    mid_mutex_lock(g_logmutex);
    if(g_logmsg.HTTPInfoNum <= LOG_INFO_NUM)
        num = g_logmsg.HTTPInfoNum;
    else
        num = LOG_INFO_NUM;

    for(i = 0; i < num; i++)
        len += snprintf(value + len, LOG_TR069_BUF_SIZE - len, "%s", g_logmsg.HTTPInfo[i]);
    g_logmsg.HTTPInfoNum = 0;
    mid_mutex_unlock(g_logmutex);

    return 0;
}

/*------------------------------------------------------------------------------
 ------------------------------------------------------------------------------*/
static int getTr069PortIGMPInfo(char* value, unsigned int size)
{
    int i = 0, num = 0, len = 0;

    mid_mutex_lock(g_logmutex);
    if(g_logmsg.IGMPInfoNum <= LOG_INFO_NUM)
        num = g_logmsg.IGMPInfoNum;
    else
        num = LOG_INFO_NUM;

    for(i = 0; i < num; i++)
        len += snprintf(value + len, LOG_TR069_BUF_SIZE - len, "%s", g_logmsg.IGMPInfo[i]);
    g_logmsg.IGMPInfoNum = 0;
    mid_mutex_unlock(g_logmutex);

    return 0;
}

/*------------------------------------------------------------------------------
 ------------------------------------------------------------------------------*/
static int getTr069PortPkgTotalOneSec(char* value, unsigned int size) //tr069_log_get_totalPkgPerSec
{
    mid_mutex_lock(g_logmutex);
    snprintf(value, size,"%d", g_logmsg.PkgTotalOneSec);
    g_logmsg.PkgTotalOneSec = 0;
    mid_mutex_unlock(g_logmutex);

    return 0;
}

/*------------------------------------------------------------------------------
 ------------------------------------------------------------------------------*/
static int getTr069PortByteTotalOneSec(char* value, unsigned int size) //tr069_log_get_totalBytePerSec
{
    mid_mutex_lock(g_logmutex);
    snprintf(value, size,"%d", g_logmsg.ByteTotalOneSec);
    g_logmsg.ByteTotalOneSec = 0;
    mid_mutex_unlock(g_logmutex);

    return 0;
}

/*------------------------------------------------------------------------------
 ------------------------------------------------------------------------------*/
static int getTr069PortPkgLostRate(char* value, unsigned int size) //tr069_log_get_PkgLostRate
{
    mid_mutex_lock(g_logmutex);
    snprintf(value, size, "%u", g_logmsg.PkgLostRate);
    g_logmsg.PkgLostRate = 0;
    mid_mutex_unlock(g_logmutex);

    return 0;
}

/*------------------------------------------------------------------------------
 ------------------------------------------------------------------------------*/
static int getTr069PortAvarageRate(char* value, unsigned int size) //tr069_log_get_AvarageRate
{
    mid_mutex_lock(g_logmutex);
    snprintf(value, size,"%u", g_logmsg.AvarageRate);
    g_logmsg.AvarageRate = 0;
    mid_mutex_unlock(g_logmutex);

    return 0;
}

/*------------------------------------------------------------------------------
 ------------------------------------------------------------------------------*/
static int getTr069PortBUFFER(char* value, unsigned int size) //tr069_log_get_buffer
{
    mid_mutex_lock(g_logmutex);
    snprintf(value, size, "%u", g_logmsg.BUFFER);
    g_logmsg.BUFFER = 0;
    mid_mutex_unlock(g_logmutex);

    return 0;
}

/*------------------------------------------------------------------------------
 ------------------------------------------------------------------------------*/
static int getTr069PortERROR(char* value, unsigned int size) //tr069_log_get_error
{
    int i = 0, num = 0, len = 0;

    mid_mutex_lock(g_logmutex);
    if(g_logmsg.ERRNum <= LOG_INFO_NUM)
        num = g_logmsg.ERRNum;
    else
        num = LOG_INFO_NUM;

    for(i = 0; i < num; i++)
        len += snprintf(value + len, LOG_TR069_BUF_SIZE - len, "%s", g_logmsg.ERR[i]);
    g_logmsg.ERRNum = 0;
    mid_mutex_unlock(g_logmutex);

    return 0;
}

/*------------------------------------------------------------------------------
 ------------------------------------------------------------------------------*/
static int getTr069PortVendorExt(char* value, unsigned int size) //tr069_log_get_VendorExt
{
    return 0;
}

Tr069LogMsg::Tr069LogMsg()
	: Tr069GroupCall("LogMsg")
{
	
	/* 以下对象的注册到表root.Device.X_CTC_IPTV.LogMsg  */

    Tr069Call* fun1  = new Tr069FunctionCall("Enable", getTr069PortEnable, setTr069PortEnable);
    regist(fun1->name(), fun1);    

  
    Tr069Call* fun2  = new Tr069FunctionCall("MsgOrFile", getTr069PortMsgOrFile, setTr069PortMsgOrFile);
    regist(fun2->name(), fun2);    
    
    Tr069Call* fun3  = new Tr069FunctionCall("Duration", getTr069PortDuration, setTr069PortDuration);
    regist(fun3->name(), fun3);    
    
    Tr069Call* fun4  = new Tr069FunctionCall("LogFtpServer", getTr069PortLogFtpServer, setTr069PortLogFtpServer);
    regist(fun4->name(), fun4);    
    

    Tr069Call* fun5  = new Tr069FunctionCall("LogFtpUser", getTr069PortLogFtpUser, setTr069PortLogFtpUser);
    regist(fun5->name(), fun5);    
    
    Tr069Call* fun6  = new Tr069FunctionCall("LogFtpPassword", getTr069PortLogFtpPassword, setTr069PortLogFtpPassword);
    regist(fun6->name(), fun6);    
    
    Tr069Call* fun7  = new Tr069FunctionCall("RTSPInfo", getTr069PortRTSPInfo, NULL);
    regist(fun7->name(), fun7);    
    
    Tr069Call* fun8  = new Tr069FunctionCall("HTTPInfo", getTr069PortHTTPInfo, NULL);
    regist(fun8->name(), fun8);       
    
    Tr069Call* fun9  = new Tr069FunctionCall("IGMPInfo", getTr069PortIGMPInfo, NULL);
    regist(fun9->name(), fun9);    
    
    Tr069Call* fun10  = new Tr069FunctionCall("PkgTotalOneSec", getTr069PortPkgTotalOneSec, NULL);
    regist(fun10->name(), fun10);    
    
    Tr069Call* fun11  = new Tr069FunctionCall("ByteTotalOneSec", getTr069PortByteTotalOneSec, NULL);
    regist(fun11->name(), fun11);    
    
    Tr069Call* fun12  = new Tr069FunctionCall("PkgLostRate", getTr069PortPkgLostRate, NULL);
    regist(fun12->name(), fun12);    
    
    Tr069Call* fun13  = new Tr069FunctionCall("AvarageRate", getTr069PortAvarageRate, NULL);
    regist(fun13->name(), fun13);    
    
    Tr069Call* fun14  = new Tr069FunctionCall("BUFFER", getTr069PortBUFFER, NULL);
    regist(fun14->name(), fun14);    
    
    Tr069Call* fun15  = new Tr069FunctionCall("ERROR", getTr069PortERROR, NULL);
    regist(fun15->name(), fun15);    
    
    Tr069Call* fun16  = new Tr069FunctionCall("VendorExt", getTr069PortVendorExt, NULL);
    regist(fun16->name(), fun16);         


}

Tr069LogMsg::~Tr069LogMsg()
{
}
