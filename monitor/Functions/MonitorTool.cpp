
#include <sys/wait.h>
#include <stdio.h>
#include <sys/sysinfo.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <mntent.h>
#include <fcntl.h>
#include <errno.h>
#include <curl/curl.h>
#include <sys/un.h>
#include <linux/types.h>
#include <linux/netlink.h>

#include "ParseCmd.h"
#include "MonitorManager.h"
#include "ParseXML.h"
#include "MonitorTool.h"
#include "MonitorTraceroute.h"
#include "MonitorAgent.h"
#include "MonitorTimer.h"
#include "preconfig.h"


typedef long long msec_t;

#ifdef Android
void* monitorCheckUSBDiskStatus(void *param);
#endif


extern char stbMonitorIP[32];
extern moni_buf g_monitorBuf;
extern msgq_t g_Msg;
extern char stbMonitorIP[32];

int g_StartupCaptured;
static int g_removeUSBDisk = 0;
int g_StartupFlag;
char g_StartupLogFile[100];

static int g_pcap_time = 10;
static int g_pcap_size = 8192;
static char g_pcap_command[4096] = {0};
static char g_DebugInfoFolder[100];
static char g_StartupLoadAddr[1024];
//static int g_stopStartupCapture;
static char g_StartupInfoFolder[1024];
static STBDebugInfoStatus g_debugInfoStatus = DEBUG_INFO_EXIT;
STBStartupInfoStatus g_startupInfoStatus = STARTUP_INFO_EXIT;
static char g_DebugInfoCaptureCmd[100];
static char g_DebugInfoRouteFileName[100];
static char g_DebugInfoArpFileName[100];
static char g_DebugInfoPsFileName[100];
static char g_DebugInfoTopFileName[100];
static char g_DebugInfoIfconfigFileName[100];
static char g_startupPacketFile[1024];
static int g_DebugInfoUSBStore = 0;
static int g_DebugInfoGetError = 0;
long long g_DebugInfoPacketSize = 0;
long long g_UDiskFreeSize = 0;
extern STBDebugInfoType g_debugInfoType;

//static TRACEROUTE_STATUS traceroute_flag = TRACEROUTE_STOP;

#define COLLECTFOLDERNAME   "/var/STBDebugInfo"
#define REMOTEPCAPFILE      "/var/"
#define REMOTEPCAPFILEEXT   ".cap"
#define RECV_BUFFER_LEN     (2048)

void monitorSetPingStop( void );
int monitorTraceroute( char *url, moni_buf_t buf, void* stb_func );

void monitorSendUsbMsgToBrower(int argc)
{
#ifdef ANDROID
    MonitorAgent::GetInstance()->NotifyAgent((HMW_MgmtMsgType)MGMT_MT_UDISK_OUT, 1, NULL);
#else
    //setUsbNoInsertMessage("\u672a\u68c0\u6d4b\u5230USB\u8bbe\u5907!");
#endif
}


int getDebugInfoStatus()
{
    return g_debugInfoStatus;
}

/************************************************************
Function: getStartupInfo
Description: 开机抓包入口
Input:
Output:
Return:
Others:
************************************************************/
extern "C"
int monitorGetStartupInfo()
{
    int startupInfoFlag         = 0;
    char macAddress[30]         = {0};
    char command[1024]          = {0};
    char uploadAddress[1024]    = {0};
    char USBDiskPath[100]       = {0};
    char strStartupInfoFlag[2]  = {0};
    pthread_t checkUSBDiskPid = 0;
    FILE *logfd = NULL;
    time_t t;
    time_t tempTime;

    if (g_startupInfoStatus != STARTUP_INFO_EXIT)
        return -1;

    g_startupInfoStatus = STARTUP_INFO_START;

    printf("get StartupInfo flag uploadAddr...\n");
    MonitorAgent::GetInstance()->ReadAgent(MGMT_CONFIG_MT, (const char *)"StartupCaptured", strStartupInfoFlag, 2);
    MonitorAgent::GetInstance()->ReadAgent(MGMT_CONFIG_MT, (const char *)"StartupUploadAddr", uploadAddress, 100);
    if (strlen(strStartupInfoFlag) == 0)
        return GET_FLAG_ERROR;
    startupInfoFlag = atoi(strStartupInfoFlag);

    if (((startupInfoFlag != 1) && (startupInfoFlag != 2))) {
        printf("no StartupInfo!\n");
 		MonitorAgent::GetInstance()->WriteAgent(MGMT_CONFIG_MT, (const char *)"StartupCaptured", (char *)"0", 2);
    	MonitorAgent::GetInstance()->WriteAgent(MGMT_CONFIG_MT, (const char *)"StartupUploadAddr", (char *)" ", 2);
        g_startupInfoStatus = STARTUP_INFO_EXIT;
        return -1;
    }

    g_StartupFlag = startupInfoFlag;

    printf("%d...\n", g_StartupFlag);
    strncpy(g_StartupLoadAddr, uploadAddress, strlen(uploadAddress));

    monitorGetMacAddress(macAddress);
    if (g_StartupFlag == 1) {
        sprintf(g_StartupInfoFolder, "/var/STBStartup%s/", macAddress);

    } else if (g_StartupFlag == 2) {
        monitorGetFirstUSBDisk(USBDiskPath);
        time(&t);
        struct tm *ctm = localtime(&t);

        if (strlen(USBDiskPath) == 0) {
            printf("no USBdisk\n");
            g_StartupFlag = 0;
            #ifdef ANDROID
            MonitorAgent::GetInstance()->WriteAgent(MGMT_CONFIG_MT, (const char *)"System.ShowToast", (char *)"{\"messageType\":2,\"message\":\"\u672a\u68c0\u6d4b\u5230USB\u8bbe\u5907!\"}", 16);
            #else
            MonitorAgent::GetInstance()->NotifyAgent(MGMT_MT_UDISK_OUT, 0, NULL);
            #endif
 			MonitorAgent::GetInstance()->WriteAgent(MGMT_CONFIG_MT, (const char *)"StartupCaptured", (char *)"0", 2);
    		MonitorAgent::GetInstance()->WriteAgent(MGMT_CONFIG_MT, (const char *)"StartupUploadAddr", (char *)" ", 2);
            g_startupInfoStatus = STARTUP_INFO_EXIT;

            return -1;
        }

        g_removeUSBDisk = 0;

        while (1) {
            time(&tempTime);
            struct tm *stTime = localtime(&tempTime);
            printf("%04d\n", stTime->tm_year);
            if (stTime->tm_year != 70)
                break;
            usleep(100000);
        }

        sprintf(g_StartupInfoFolder, "%sSTBStartup%s%04d%02d%02d%02d%02d%02d/", USBDiskPath, macAddress,
            ctm->tm_year + 1900, ctm->tm_mon + 1, ctm->tm_mday,
            ctm->tm_hour, ctm->tm_min, ctm->tm_sec);
        printf("....%s\n", g_StartupInfoFolder);

        //检测U盘拔出
        #ifdef ANDROID
        // Linux由NativeHandler监控， 不需要自己监控。
        pthread_create(&checkUSBDiskPid, NULL, monitorCheckUSBDiskStatus, NULL);
        #endif
    }

    sprintf(command, "busybox rm -rf %s", g_StartupInfoFolder);
    monitorRunCommand(command);
    memset(command, 0, 1024);
    sprintf(command, "mkdir %s", g_StartupInfoFolder);
    monitorRunCommand(command);

    //get log
    printf("capture log\n");
    sprintf(g_StartupLogFile, "%slog", g_StartupInfoFolder);
    #ifdef ANDROID
    logfd = fopen(g_StartupLogFile, "a+");

    if (!logfd)
        printf("Debuginfo logfile create erro\n");

    startupLog(logfd);
    #endif

    //get packet
    printf("capture packet\n");
    pthread_t capThread_id = 0;
    pthread_create(&capThread_id, NULL, monitorGetStartupPacket, NULL);

    sleep(1);

    //check file size and memsize
    printf("check \n");
    pthread_t checkThread_id = 0;
    pthread_create(&checkThread_id, NULL, monitorCheckStartupStatus, NULL);

    return 0;
}


/************************************************************
Function: monitorStopStartupInfo
Description: 关闭开机抓包
Input:
Output:
Return:
Others:
************************************************************/
int monitorStopStartupInfo()
{
    if (g_startupInfoStatus == STARTUP_INFO_START) {
        g_startupInfoStatus = STARTUP_INFO_STOP;
        return 0;
    }
    return -1;
}


/************************************************************
Function: monitorGetStartupPacket
Description: 抓包线程
Input:
Output:
Return:
Others:
************************************************************/
void * monitorGetStartupPacket(void *parm)
{
    char fileName[1024] = {0};
    char command[1024] = {0};

    time_t ti;
    time(&ti);
    struct tm *ctm = localtime(&ti);

    pthread_detach(pthread_self());

    while (1) {
        time(&ti);
        struct tm *ctm = localtime(&ti);
        printf("%04d\n", ctm->tm_year);
        if (ctm->tm_year != 70)
            break;
        usleep(100000);
    }

    sprintf(fileName, "%s%04d%02d%02d%02d%02d%02d",
            g_StartupInfoFolder, ctm->tm_year + 1900, ctm->tm_mon + 1, ctm->tm_mday,
            ctm->tm_hour, ctm->tm_min, ctm->tm_sec);

    strcat(fileName, REMOTEPCAPFILEEXT);
    strcpy(g_startupPacketFile, fileName);
    memset(command, 0, 1024);
    if (g_StartupFlag == 1)
        sprintf(command, "tcpdump -i any -N -n -ttt -A -v -X -l -s0 -w %s -W 1 -Q 20 &", fileName);
    else if (g_StartupFlag == 2)
        sprintf(command, "tcpdump -i any -N -n -ttt -A -v -X -l -s0 -w %s -W 1 &", fileName);
    monitorExecCmd(command, "r", NULL, NULL);
    return NULL;
}


/************************************************************
Function: monitorCheckStartupStatus
Description: 监测临时文件夹大小，并将文件上传到SFTP服务器
Input:
Output:
Return:
Others:
************************************************************/
void* monitorCheckStartupStatus(void *parm)
{
    int pid = 0;
    int ret = 0;
    char *sftp_ip = NULL;
    char *sftp_user = NULL;
    char *sftp_pswd = NULL;

    pthread_detach(pthread_self());

    //check
    while (monitorCheckStartupSize() && (g_startupInfoStatus == STARTUP_INFO_START) && g_removeUSBDisk == 0) {
        usleep(1000000);
    }

    g_startupInfoStatus = STARTUP_INFO_STOP;

    //stop capture packet
    pid = monitorGetPidByName((char *)("tcpdump"));
    if(pid != 0) {
        kill(pid, SIGKILL);
        waitpid(pid, NULL, 0);
    }

    //stop log
    stopStartupLog();

 	MonitorAgent::GetInstance()->WriteAgent(MGMT_CONFIG_MT, (const char *)"StartupCaptured", (char *)"0", 2);
    MonitorAgent::GetInstance()->WriteAgent(MGMT_CONFIG_MT, (const char *)"StartupUploadAddr", (char *)" ", 2);

    if (g_StartupFlag == 2) {
        g_StartupFlag = 0;
        g_startupInfoStatus = STARTUP_INFO_EXIT;
        return NULL;
    }

    //upload files
    sftp_ip = strtok(g_StartupLoadAddr, "^");
    sftp_user = strtok(NULL, "^");
    sftp_pswd = strtok(NULL, "^");

    printf("sftp_ip = %s sftp_user = %s sftp_pswd = %s \n",sftp_ip, sftp_user, sftp_pswd);

    if (sftp_user && sftp_user && sftp_ip) {
        ret = monitorUploadFiles(g_StartupInfoFolder, sftp_user, sftp_pswd, sftp_ip);
        if (ret != 0)
            //10分钟后自动删除文件夹
            monitorTimerCreate(600, 1, monitorTimerDelStartupInfo, 0);
    } else {
        printf("upload addr error!\n");
        g_startupInfoStatus = STARTUP_INFO_UPLOAD;
        //10分钟后自动删除文件夹
        monitorTimerCreate(600, 1, monitorTimerDelStartupInfo, 0);
    }
    return NULL;
}


/************************************************************
Function:monitorCheckStartupSize
Description: 查看内存大小，文件大小
Input:
Output:
Return:
Others:
************************************************************/
int monitorCheckStartupSize()
{
    struct sysinfo s_info;
    sysinfo(&s_info);

    //内存中不超过10M
    if (g_StartupFlag == 1) {
        //已用大小超过10M就返回，内存小于20M就返回
        if ((monitorGetFolderSize(g_StartupInfoFolder) >= 10 *1024 * 1024)
            || (s_info.freeram <= 20 * 1024 * 1024)) {
            printf("over size ....\n");
            return 0;
        }
    }

    //U盘中不超过20M
    if (g_StartupFlag == 2) {
        //已用大小超过10M就返回，内存小于20M就返回
        if ((monitorGetFolderSize(g_StartupInfoFolder) >= 20 *1024 * 1024)
            || (s_info.freeram <= 20 * 1024 * 1024)
            || g_removeUSBDisk) {
            printf("exit startup info ....\n");
            return 0;
        }
    }

    return 1;
}


/************************************************************
Function: monitorTimerDelStartupInfo
Description: 删除Startup目录所有文件
Input:
Output:
Return:
Others:
************************************************************/
void monitorTimerDelStartupInfo(int parm)
{
    char command[1024] = {0};

    sprintf(command, "busybox rm -rf %s", g_StartupInfoFolder);
    monitorRunCommand(command);
    g_startupInfoStatus = STARTUP_INFO_EXIT;
}


/************************************************************
Function: uploadStartupInfo
Description: 上传Startup目录所有文件
Input:
Output:
Return:
Others:
************************************************************/
int monitorUploadStartupInfo(char *uploadAddr)
{
    char *sftp_ip = NULL;
    char *sftp_user = NULL;
    char *sftp_pswd = NULL;

    if (!uploadAddr)
        return -1;

    memset(g_StartupLoadAddr, 0, sizeof(g_StartupLoadAddr));
    strcpy(g_StartupLoadAddr, uploadAddr);

    if (g_startupInfoStatus == STARTUP_INFO_START) {
        g_startupInfoStatus = STARTUP_INFO_STOP;

    } else if (g_startupInfoStatus == STARTUP_INFO_UPLOAD) {
        sftp_ip = strtok(g_StartupLoadAddr, "^");
        sftp_user = strtok(NULL, "^");
        sftp_pswd = strtok(NULL, "^");

        printf("sftp_ip = %s sftp_user = %s sftp_pswd = %s \n",sftp_ip, sftp_user, sftp_pswd);

        if (sftp_user && sftp_user && sftp_ip) {
            monitorUploadFiles(g_StartupInfoFolder, sftp_user, sftp_pswd, sftp_ip);

        }

    }

    return 0;
}


/**************************DebugInfo************************/

/************************************************************
Function: monitorStartDebugInfo
Description: 一键收集信息开始
Input: tcpdump 命令配置信息
Output:
Return:
Others:
************************************************************/
int monitorStartDebugInfo(char *parameter)
{
    time_t t;
    char macAddress[13]   = {0};
    char command[100]     = {0};
    char USBDiskPath[100] = {0};
    pthread_t  checkDiskThreadID = 0;

    if (g_debugInfoStatus != DEBUG_INFO_EXIT) {
        printf("g_debugInfoStatus = %d\n", g_debugInfoStatus);
        return -1;
    }

    g_debugInfoStatus = DEBUG_INFO_START;
	#ifdef ANDROID
		MonitorAgent::GetInstance()->NotifyAgent(MGMT_MT_ENTER_DEBUG, 0, NULL);
	#endif
    g_DebugInfoGetError = 0;
    bzero(g_DebugInfoFolder, sizeof(g_DebugInfoFolder));
    bzero(g_DebugInfoRouteFileName, sizeof(g_DebugInfoRouteFileName));
    bzero(g_DebugInfoArpFileName, sizeof(g_DebugInfoArpFileName));
    bzero(g_DebugInfoPsFileName, sizeof(g_DebugInfoPsFileName));
    bzero(g_DebugInfoTopFileName, sizeof(g_DebugInfoTopFileName));
    bzero(g_DebugInfoIfconfigFileName, sizeof(g_DebugInfoIfconfigFileName));
    bzero(g_DebugInfoCaptureCmd, sizeof(g_DebugInfoCaptureCmd));

    //usb store
    if (parameter == NULL) {
        strcpy(g_DebugInfoCaptureCmd, "tcpdump -i any -N -n -ttt -A -v -X -l -s0");
        monitorGetFirstUSBDisk(USBDiskPath);
        time(&t);
        struct tm *ctm = localtime(&t);

        if (strlen(USBDiskPath) == 0) {
            printf("no USBdisk\n");
            #ifdef ANDROID
            MonitorAgent::GetInstance()->NotifyAgent(MGMT_MT_UDISK_OUT, 1, NULL);
            #endif
            g_debugInfoStatus = DEBUG_INFO_EXIT;
            g_debugInfoType = DEBUG_INFO_NONE;
            return -1;
        }

        g_removeUSBDisk = 0;
        monitorGetMacAddress(macAddress);
        sprintf(g_DebugInfoFolder, "%sSTBDebugInfo%s%04d%02d%02d%02d%02d%02d", USBDiskPath, macAddress,
            ctm->tm_year + 1900, ctm->tm_mon + 1, ctm->tm_mday,
            ctm->tm_hour, ctm->tm_min, ctm->tm_sec);

        g_DebugInfoUSBStore = 1;
        printf("....%s\n", g_DebugInfoFolder);

        //检测U盘拔出
        #ifdef ANDROID
        // Linux由NativeHandler监控， 不需要自己监控。
        pthread_create(&checkDiskThreadID, NULL, monitorCheckUSBDiskStatus, NULL);
        #endif

    } else {
        if (strcmp(parameter, "null") != 0)
            strcpy(g_DebugInfoCaptureCmd, parameter);
        else
            strcpy(g_DebugInfoCaptureCmd, "");

        //创建收集信息目录

        time(&t);
        struct tm *ctm = localtime(&t);
        monitorGetMacAddress(macAddress);
        sprintf(g_DebugInfoFolder, COLLECTFOLDERNAME"%s%04d%02d%02d%02d%02d%02d",
                macAddress, ctm->tm_year + 1900, ctm->tm_mon + 1, ctm->tm_mday,
                ctm->tm_hour, ctm->tm_min, ctm->tm_sec);

    }


    sprintf(command, "mkdir %s", g_DebugInfoFolder);
    monitorExecCmd(command, "r", NULL, NULL);

    //收集信息
    pthread_t  collect_id = 0;
    pthread_create(&collect_id, NULL, monitorGetDebugInfoFiles, NULL);

    sleep(1);
    pthread_t check_id = 0;
    pthread_create(&check_id, NULL, monitorCheckDebugInfoStatus, NULL);
    return 0;
}



/************************************************************
Function: monitorStopDebugInfo
Description: 一键收集信息结束
Input:
Output:
Return:
Others:
************************************************************/
int monitorStopDebugInfo(char *parameter)
{
	if(DEBUG_INFO_EXIT != g_debugInfoStatus){
		#ifdef ANDROID
			MonitorAgent::GetInstance()->NotifyAgent(MGMT_MT_EXIT_DEBUG, 0, NULL);
		#endif
		g_debugInfoStatus = DEBUG_INFO_STOP;
	}
    return 0;
}



/************************************************************
Function: monitorUploadDebugInfo
Description: 上传debuginfo文件
Input: 上传路径及用户名密码
Output:
Return:
Others:
************************************************************/
int monitorUploadDebugInfo(char *upload_param)
{
    if (!upload_param) {
        g_debugInfoStatus = DEBUG_INFO_EXIT;
        return -1;
    }

    if (DEBUG_INFO_START == g_debugInfoStatus) {
			#ifdef ANDROID
				MonitorAgent::GetInstance()->NotifyAgent(MGMT_MT_EXIT_DEBUG, 0, NULL);
			#endif
        g_debugInfoStatus = DEBUG_INFO_STOP;
        usleep(500000);
    }
    
    char *upload_ip = strtok(upload_param + 5, "^");
    char *upload_user = strtok(NULL, "^");
    char *upload_pswd = strtok(NULL, "^");

    if (!upload_ip || !upload_user || !upload_pswd) {
        printf("DebugInfo upload addr error \n");
        g_debugInfoStatus = DEBUG_INFO_EXIT;
        return -1;
    }

    monitorUploadFiles(g_DebugInfoFolder, upload_user, upload_pswd, upload_ip);
    g_debugInfoStatus = DEBUG_INFO_EXIT;
    return 0;

}


/************************************************************
Function: monitorGetDebugInfoFiles
Description: 获取一键收集信息文件
Input:
Output:
Return:
Others:
************************************************************/
void* monitorGetDebugInfoFiles(void *parm)
{
    char logFileName[100] = {0};
    char configFileCmd[100] = {0};
    pthread_t  packet_id = 0;
    FILE *logfd = NULL;

    pthread_detach(pthread_self());

    sprintf(g_DebugInfoRouteFileName,    "%s/STBRoute.txt",    g_DebugInfoFolder);
    sprintf(g_DebugInfoArpFileName,      "%s/STBArp.txt",      g_DebugInfoFolder);
    sprintf(g_DebugInfoPsFileName,       "%s/STBPs.txt",       g_DebugInfoFolder);
    sprintf(g_DebugInfoTopFileName,      "%s/STBTop.txt",      g_DebugInfoFolder);
    sprintf(g_DebugInfoIfconfigFileName, "%s/STBIfconfig.txt", g_DebugInfoFolder);

#if 0
#ifdef ANDROID
#ifdef DEFAULT_STBTYPE
    if (strcmp("EC6106V8H", DEFAULT_STBTYPE) == 0) {
        #define FILES_INI_PATHS "/data/data/com.hybroad.iptv.app/files/*.ini"
        #define FILES_CFG_PATHS "/data/data/com.hybroad.iptv.app/files/*.cfg"
    } else if (strcmp("EC6106V6", DEFAULT_STBTYPE) == 0) {
        #define FILES_INI_PATHS "/data/iptv/*.ini"
        #define FILES_CFG_PATHS "/data/iptv/*.cfg"
    }
#else // #ifdef DEFAULT_STBTYPE
    #define DEFAULT_STBTYPE
#endif // #ifdef DEFAULT_STBTYPE
#else
    #define FILES_INI_PATHS "/root/*.ini"
    #define FILES_CFG_PATHS "/root/*.cfg"
#endif

    //collect config files
    sprintf(configFileCmd, "cp "FILES_INI_PATHS" %s", g_DebugInfoFolder);
    monitorRunCommand(configFileCmd);
    sprintf(configFileCmd, "cp "FILES_CFG_PATHS" %s", g_DebugInfoFolder);
    monitorRunCommand(configFileCmd);
#else // if 0

//collect config files
#ifdef ANDROID
#ifdef DEFAULT_STBTYPE
    if (strcmp("EC6106V8H", DEFAULT_STBTYPE) == 0) {
        sprintf(configFileCmd, "cp /data/data/com.hybroad.iptv.app/files/*.ini %s", g_DebugInfoFolder);
        monitorRunCommand(configFileCmd);
        sprintf(configFileCmd, "cp /data/data/com.hybroad.iptv.app/files/*.cfg %s", g_DebugInfoFolder);
        monitorRunCommand(configFileCmd);
    } else if (strcmp("EC6106V6", DEFAULT_STBTYPE) == 0) {
        sprintf(configFileCmd, "cp /data/iptv/*.ini %s", g_DebugInfoFolder);
        monitorRunCommand(configFileCmd);
        sprintf(configFileCmd, "cp /data/iptv/*.cfg %s", g_DebugInfoFolder);
        monitorRunCommand(configFileCmd);
    }
#endif
#else
    sprintf(configFileCmd, "cp /root/*.ini %s", g_DebugInfoFolder);
    monitorRunCommand(configFileCmd);
    sprintf(configFileCmd, "cp /root/*.cfg %s", g_DebugInfoFolder);
    monitorRunCommand(configFileCmd);
#endif  // ANDROID
#endif // if 0

    //collect packet
    if (strlen(g_DebugInfoCaptureCmd)) {
        pthread_create(&packet_id, NULL, monitorGetDebugInfoPacket, NULL);
    }

    //collect log
    sprintf(logFileName, "%s/log", g_DebugInfoFolder);
    logfd = fopen(logFileName, "a+");

    if (!logfd)
        printf("Debuginfo logfile create erro\n");

    writeLog(logfd);


    //collect other info
    monitorTimerCreate(5, 0, monitorTimerGetDebugInfo, 0);

    return NULL;
}


/************************************************************
Function: monitorCheckDebugInfoStatus
Description: 查看状态
Input:
Output:
Return:
Others:
************************************************************/
void* monitorCheckDebugInfoStatus(void* param)
{
    struct sysinfo s_info;
    char packetFileName[100] = {0};
    int pcapStartTime = 0;
    int lastReportTime = 0;
    long long int folderSize = 0;
    struct stat64 st;

    sprintf(packetFileName, "%s/STBCap.cap", g_DebugInfoFolder);
    pthread_detach(pthread_self());

    while (1) {
        if (g_DebugInfoGetError == 1) {
            printf("g_DebugInfoGetError = 1\n");
            break;
        }

        //内存小于10M就变为DEBUG_INFO_STOP
        if (DEBUG_INFO_START == g_debugInfoStatus && g_DebugInfoUSBStore == 0) {
            sysinfo(&s_info);

            //内存小于10M就返回
            if (s_info.freeram <= 10 * 1024 * 1024) {
                printf("ram < 10M\n");
                g_debugInfoStatus = DEBUG_INFO_STOP;
            	#ifdef ANDROID
					MonitorAgent::GetInstance()->NotifyAgent(MGMT_MT_EXIT_DEBUG, 0, NULL);
				#endif

            }

        } else if (DEBUG_INFO_STOP == g_debugInfoStatus) {
            break;
        }

        folderSize = monitorGetFolderSize(g_DebugInfoFolder);
        //printf("folderSize = [%lld]\n", folderSize);

        if (g_DebugInfoUSBStore == 1) {
            //U盘已存储2G或快到U盘最大容量时就结束
            if ((folderSize/1024 >= 2 * 1024 * 1024)
               //|| (folderSize/1024) >= (g_UDiskFreeSize - 20 * 1024)
               || (g_removeUSBDisk == 1)) {
                MonitorAgent::GetInstance()->NotifyAgent((HMW_MgmtMsgType)MGMT_HYBROAD_TCPDUMP_DOWN, 1, NULL);

                g_debugInfoStatus = DEBUG_INFO_STOP;
                printf("udisk over size or out %lld %lld %d\n", folderSize, g_UDiskFreeSize, g_removeUSBDisk);
                break;
            }

        } else if (g_DebugInfoUSBStore == 0) {
            if (stat64(packetFileName, &st) < 0)
                continue;
            //printf("packet size = %lld  %lld %lld\n", (long long)st.st_size, g_DebugInfoPacketSize, ((g_DebugInfoPacketSize * 1024  * 1024 * 9) / 10));
            //if ((long long)st.st_size >= (g_DebugInfoPacketSize * 1024  * 1024 * 9) / 10) {
            //    g_debugInfoStatus = DEBUG_INFO_STOP;
            //    break;
            //}
        }

        usleep(100000);
    }

    if (g_DebugInfoUSBStore == 0) {
        //10分钟后自动删除文件夹
        char *p_DebugInfoFolder = (char *)malloc(strlen(g_DebugInfoFolder)+1);
        if(p_DebugInfoFolder){
            memset(p_DebugInfoFolder, 0, strlen(g_DebugInfoFolder)+1);
            strcpy(p_DebugInfoFolder, g_DebugInfoFolder);
            monitorTimerCreate(600, 1, monitorTimerDelDebugInfo, (int)p_DebugInfoFolder);
        }
    }
    //printf("test:stop collect..\n");

    //stop collect
    int pid = monitorGetPidByName((char *)("tcpdump"));
    printf("pid = %d\n", pid);
    if(pid != 0) {
        kill(pid, SIGKILL);
        waitpid(pid, NULL, 0);
    }

    //stop collect other info
    monitorTimerDelete(monitorTimerGetDebugInfo, 0);

    //stop log
    stopLog();
    if (g_DebugInfoUSBStore == 1) {
        MonitorAgent::GetInstance()->NotifyAgent((HMW_MgmtMsgType)MGMT_HYBROAD_TCPDUMP_DOWN, 1, NULL);

    }

    g_debugInfoStatus = DEBUG_INFO_EXIT;
    g_DebugInfoUSBStore = 0;
    g_DebugInfoGetError = 0;

    return NULL;
}


/************************************************************
Function: monitorTimerDelDebugInfo
Description: 删除DebugInfo目录所有文件
Input:
Output:
Return:
Others:
************************************************************/
void monitorTimerDelDebugInfo(int parm)
{
    char command[1024] = {0};
    char *p_DebugInfoFolder = (char *)parm;

    if(!p_DebugInfoFolder){
        printf("para == 0\n");
        return;
    }

    sprintf(command, "busybox rm -rf %s", g_DebugInfoFolder);
    monitorRunCommand(command);
    free(p_DebugInfoFolder);
    p_DebugInfoFolder = NULL;
    g_debugInfoStatus = DEBUG_INFO_EXIT;

}


/************************************************************
Function: monitorGetDebugInfoPacket
Description: 删除DebugInfo目录所有文件
Input:
Output:
Return:
Others:
************************************************************/
void* monitorGetDebugInfoPacket(void *parm)
{
        char packetFileName[100] = {0};
        char tcpdumpCommand[1024] = {0};
        //int free_size = 0;

        pthread_detach(pthread_self());

        //struct sysinfo s_info;
        //sysinfo(&s_info);
        //free_size = (s_info.freeram - 2*1024*1024)/(1024*1024);


        //if(free_size > 0){
        //    if(free_size > 100){
        //        free_size = 100;
        //}

        //g_DebugInfoPacketSize = free_size;

        sprintf(packetFileName, "%s/STBCap.cap", g_DebugInfoFolder);
        printf("[%s %s %d]g_DebugInfoCaptureCmd:%s\n", __FILE__, __FUNCTION__, __LINE__, g_DebugInfoCaptureCmd);
        printf("[%s %s %d]packetFileName:%s\n", __FILE__, __FUNCTION__, __LINE__, packetFileName);

        if (g_DebugInfoUSBStore == 1)
            sprintf(tcpdumpCommand, "%s -w %s &", g_DebugInfoCaptureCmd, packetFileName);
        else {
            if (strncmp(g_DebugInfoCaptureCmd + 5, "tcpdump", 7) == 0) {
                if (g_DebugInfoCaptureCmd[12] == 0)
                    sprintf(tcpdumpCommand, "tcpdump -s0 -w %s -Q 20 &", packetFileName);
                else
                    sprintf(tcpdumpCommand, "tcpdump -s0 -w %s -Q 20 %s &", packetFileName, &g_DebugInfoCaptureCmd[12]);
            }
        }
        printf("[%s %s %d]tcpdumpCommand:%s\n", __FILE__, __FUNCTION__, __LINE__, tcpdumpCommand);

        monitorExecCmd(tcpdumpCommand, "r", NULL, NULL);

        //}else{
        //    g_debugInfoStatus = DEBUG_INFO_STOP;
        //}
        return NULL;

}


/************************************************************
Function: monitorTimerGetDebugInfo
Description: 定时器函数，得到debuginfo文件信息
Input:
Output:
Return:
Others:
************************************************************/
void monitorTimerGetDebugInfo(int parameter)
{
    char strCommand[1024] = {0};

    if (DEBUG_INFO_START == g_debugInfoStatus) {
        //route
        printf("get route info\n");
        memset(strCommand, 0, sizeof(strCommand));
        printf("g_DebugInfoRouteFileName = [%s]\n", g_DebugInfoRouteFileName);
        sprintf(strCommand, "busybox route -n >> %s", g_DebugInfoRouteFileName);
        if (monitorRunCommand(strCommand) == 256)
            g_DebugInfoGetError = 1;

        //arp
        printf("get arp info\n");
        memset(strCommand, 0, sizeof(strCommand));
        sprintf(strCommand, "busybox arp -an >> %s", g_DebugInfoArpFileName);
        if (monitorRunCommand(strCommand) == 256)
            g_DebugInfoGetError = 1;

        //ps
        printf("get ps info\n");
        memset(strCommand, 0, sizeof(strCommand));
        sprintf(strCommand, "busybox ps >> %s", g_DebugInfoPsFileName);
        if (monitorRunCommand(strCommand) == 256)
            g_DebugInfoGetError = 1;

        //top
        printf("get top info\n");
        memset(strCommand, 0, sizeof(strCommand));
        sprintf(strCommand, "busybox top -n 1 >> %s", g_DebugInfoTopFileName);
        if (monitorRunCommand(strCommand) == 256)
            g_DebugInfoGetError = 1;

        //ifconfig
        printf("get ifconfig info\n");
        memset(strCommand, 0, sizeof(strCommand));
        sprintf(strCommand, "busybox ifconfig >> %s", g_DebugInfoIfconfigFileName);
        if (monitorRunCommand(strCommand) == 256)
            g_DebugInfoGetError = 1;
    }

}



/************************************************************
Function: monitorGetMacAddress
Description: 得到mac地址
Input:
Output:mac地址字串
Return:
Others:
************************************************************/
int monitorGetMacAddress(char* mac)
{
    struct ifreq tmp;
    int sockMac = 0;
    char macAddress[30] = {0};

    sockMac = socket(AF_INET, SOCK_STREAM, 0);
    if( sockMac == -1){
        printf("create socket fail\n");
        return -1;
    }
    memset(&tmp,0,sizeof(tmp));
    strncpy(tmp.ifr_name,"eth0",sizeof(tmp.ifr_name)-1 );
    if( (ioctl( sockMac, SIOCGIFHWADDR, &tmp)) < 0 ){
        printf("mac ioctl error\n");
        return -1;
    }
    sprintf(macAddress, "%02x%02x%02x%02x%02x%02x",
            (unsigned char)tmp.ifr_hwaddr.sa_data[0],
            (unsigned char)tmp.ifr_hwaddr.sa_data[1],
            (unsigned char)tmp.ifr_hwaddr.sa_data[2],
            (unsigned char)tmp.ifr_hwaddr.sa_data[3],
            (unsigned char)tmp.ifr_hwaddr.sa_data[4],
            (unsigned char)tmp.ifr_hwaddr.sa_data[5]
            );

    close(sockMac);
    memcpy(mac,macAddress,strlen(macAddress));
    return 1;
}


/************************************************************
Function: monitorExecCmd
Description: 执行命令并存储结果
Input:
Output:
Return:
Others:
************************************************************/
void monitorExecCmd(const char *command, const char *type, const char *pFileName, const char *mode)
{
    char   buf[1024]    = {0};
    FILE * pipeStream   = NULL;
    FILE * fileStream   = NULL;

    if (command != NULL && type != NULL) {
        if ((pipeStream = popen((char *)command, type)) == NULL) {
            printf("ERROR:: system");
        }
    }
    fflush(stdout);

    if (pFileName != NULL) {
        if ((fileStream = fopen(pFileName, mode)) != NULL) {
            while (fgets(buf, 1024, pipeStream) != NULL) {
                fwrite(buf, strlen(buf), 1, fileStream);
            }

        }
    }

    if (pipeStream != NULL)
        pclose(pipeStream);
    if (fileStream != NULL)
        fclose(fileStream);
}



/************************************************************
Function: monitorUploadFiles
Description: 上传文件夹
Input:
Output:
Return:
Others:
************************************************************/
int monitorUploadFiles(char *folderName, char *user, char *pswd, char *uploadAddress)
{
    int i   = 0;
    int ret = 0;
    char command[1024]  = {0};
    char uploadURL[200] = {0};
    char fileName[100]  = {0};
    DIR *pFolder = NULL;
    struct dirent *pDirent = NULL;
    struct stat buf;

    if (!folderName || !user || !pswd || !uploadAddress) {
        printf("upload parameter error\n");
        return -1;
    }

    memset(&buf, 0, sizeof(buf));



    for (i = 0; i < 3; i++) {
        pFolder = opendir(folderName);
        if (pFolder == NULL) {
            printf("uploadFolder name error\n");
            return -1;
        }

        if (folderName[strlen(folderName) - 1] != '/')
                folderName[strlen(folderName)] = '/';


        for (pDirent = readdir(pFolder); pDirent != NULL; pDirent = readdir(pFolder)) {
            if ((strcmp(pDirent->d_name, ".") != 0) && (strcmp(pDirent->d_name,  "..") != 0)) {
                memset(fileName, 0, 100);
                memset(uploadURL, 0, 100);
                sprintf(fileName, "%s%s", folderName, pDirent->d_name);
                sprintf(uploadURL, "sftp://%s:%s@%s/%s", user, pswd, uploadAddress, pDirent->d_name);

                if ((ret = monitorSftpUpload(fileName, uploadURL)) != 0) {
                    printf("upload error\n");
                    break;
                }
                printf("upload ok \n");
            }
        }
        closedir(pFolder);

        if (ret == 0)
            break;

        usleep(15 * 1000 * 1000);
    }


    //删掉文件夹
    if (ret == 0) {
        memset(command, 0, 1024);
        sprintf(command, "busybox rm -rf %s", folderName);
        monitorRunCommand(command);
    }

    return ret;

}


/************************************************************
Function: monitorGetIfaceIndex
Description: 得到网卡index值
Input:
Output:
Return:
Others:
************************************************************/
int monitorGetIfaceIndex(int fd, const char* interfaceName)
{
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));

    if (interfaceName == NULL)
    {
        return -1;
    }

    strcpy(ifr.ifr_name, interfaceName);

    if (ioctl(fd, SIOCGIFINDEX, &ifr) == -1)
    {
        printf("RED ioctl error\n");
        return -1;
    }
    return ifr.ifr_ifindex;
}


/************************************************************
Function: monitorGetFolderSize
Description: 得到文件夹大小
Input:
Output:
Return:
Others:
************************************************************/
long long int monitorGetFolderSize(char *folderName)
{
    DIR *pFolder = NULL;
    long long int  totalSize = 0;
    char filePath[200] = {0};
    struct dirent *pDirent = NULL;
    struct stat64 st;

    if (folderName[strlen(folderName) - 1] != '/')
        folderName[strlen(folderName)] = '/';

    memset(&st, 0, sizeof(st));
    pFolder = opendir(folderName);

    if (pFolder == NULL) {
        printf("monitorGetFolderSize error\n");
        return -1;
    }

    for (pDirent = readdir(pFolder); pDirent != NULL; pDirent = readdir(pFolder)) {
        if ((strcmp(pDirent->d_name, ".") == 0) || (strcmp(pDirent->d_name, "..") == 0))
            continue;

        sprintf(filePath, "%s%s", folderName, pDirent->d_name);

        if (stat64(filePath, &st) < 0)
            return 0;

        totalSize += st.st_size;
    }

    closedir(pFolder);
    return totalSize;

}


/************************************************************
Function: monitorGetFirstUSBDisk
Description: 得到第一个U盘挂载路径
Input:
Output:U盘挂载路径
Return:
Others:
************************************************************/
void monitorGetFirstUSBDisk(char *USBDiskPath)
{
    int   i     = 0;
    int   flag  = 0;
    FILE* pMnt  = NULL;
    int   devNum = 0;
	struct mntent *pMntEntNext = NULL;
    int ret = 0;
    FILE *fstream = NULL;
    char file_buf[100 * 1024] = {0};
    char lineBuf[1024] = {0};
    char *tok = NULL;

#ifdef ANDROID
    if ((fstream = fopen("/proc/mounts", "r")) == NULL) {
        printf("open /proc/mounts\n");
        strcpy(USBDiskPath, "");
        return;
    }

    while(!feof(fstream)) {
        if (!fgets(lineBuf, 1024, fstream) && !feof(fstream)) {
            fclose(fstream);
            printf("no udisk\n");
            strcpy(USBDiskPath, "");
            return;
        }

        if (((tok = strstr(lineBuf, "/mnt/sda/sda")) != NULL)
          ||((tok = strstr(lineBuf, "/mnt/sdb/sdb")) != NULL)) {
            strncpy(USBDiskPath, tok, 13);
            if (USBDiskPath[strlen(USBDiskPath) - 1] != '/')
                    USBDiskPath[strlen(USBDiskPath)] = '/';

            printf("udisk path : %s\n", USBDiskPath);
            //
            char tempCommand[100] = {0};
            sprintf(tempCommand, "mount -o remount %s %s", USBDiskPath, USBDiskPath);
            monitorRunCommand(tempCommand);
            g_UDiskFreeSize = monitorGetUDiskFreeCapacity(USBDiskPath);
            break;
        }
    }

    fclose(fstream);
    return;
#else

	for (i = 0; i < 50; i++) {
        flag = 0;
		if (!(pMnt = setmntent("/proc/mounts", "r"))) {
			printf("setmntent return NULL\n");
			continue;
		}

		while ((pMntEntNext = getmntent(pMnt))) {
			if (!strncmp( pMntEntNext->mnt_dir, "/mnt/usb", 8)) {
                strcpy(USBDiskPath, pMntEntNext->mnt_dir);

                if (USBDiskPath[strlen(USBDiskPath) - 1] != '/')
                    USBDiskPath[strlen(USBDiskPath)] = '/';

                devNum++;
                g_UDiskFreeSize = monitorGetUDiskFreeCapacity(USBDiskPath);

             }
        }
        endmntent(pMnt);

        if (devNum > 1)
            strcpy(USBDiskPath, "");
		if (devNum == 1)
            break;
		usleep(20 *1000);
       }
#endif
}

#ifdef ANDROID
// Linux由NativeHandler监控， 不需要自己监控。

/************************************************************
Function: monitorCheckUSBDiskStatus
Description: 检查U盘是否拔出
Input:
Output:
Return:
Others:
************************************************************/
void* monitorCheckUSBDiskStatus(void *param)
{
    const int buffersize = 1024;
    int ret;

    struct sockaddr_nl snl;
    bzero(&snl, sizeof(struct sockaddr_nl));
    snl.nl_family = AF_NETLINK;
    snl.nl_groups = 1;
    snl.nl_pid = 0;
    int on = 1;

    pthread_detach(pthread_self());

    int s = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
    if (s == -1)
    {
        perror("socket");
        return NULL;
    }
    ret = setsockopt(s, SOL_SOCKET, SO_RCVBUF, &buffersize, sizeof(buffersize));

    ret = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );

    ret = bind(s, (struct sockaddr *)&snl, sizeof(struct sockaddr_nl));
    if (ret < 0)
    {
        perror("bind");
        close(s);
        return NULL;
    }

    while(1)
    {
        /* Netlink message buffer */
        char buf[2048 * 2] = {0};
        recv(s, &buf, sizeof(buf), 0);
		printf("xuke   %s\n", buf);
        if (STARTUP_INFO_START == g_startupInfoStatus) {
             g_startupInfoStatus = STARTUP_INFO_STOP;
             MonitorAgent::GetInstance()->WriteAgent(MGMT_CONFIG_MT, (const char *)"System.ShowToast", (char *)"{\"messageType\":2,\"message\":\"U\u76d8\u5df2\u62d4\u51fa!\"}", 16);
        }
        if (DEBUG_INFO_START == g_debugInfoStatus) {
            g_debugInfoStatus = DEBUG_INFO_STOP;
            MonitorAgent::GetInstance()->WriteAgent(MGMT_CONFIG_MT, (const char *)"System.ShowToast", (char *)"{\"messageType\":2,\"message\":\"U\u76d8\u5df2\u62d4\u51fa!\"}", 16);
            #ifdef ANDROID
				MonitorAgent::GetInstance()->NotifyAgent(MGMT_MT_EXIT_DEBUG, 0, NULL);
			#endif
        }

        g_debugInfoType = DEBUG_INFO_NONE;
        g_removeUSBDisk = 1;


        break;
    }

    close(s);
    return NULL;

}
#else
extern "C" int monitorUnInsertUDisk()
{
    g_removeUSBDisk = 1;

    if (STARTUP_INFO_START == g_startupInfoStatus)
        g_startupInfoStatus = STARTUP_INFO_STOP;
    if (DEBUG_INFO_START == g_debugInfoStatus)
        g_debugInfoStatus = DEBUG_INFO_STOP;

    g_debugInfoType = DEBUG_INFO_NONE;
    return 0;
}

#endif


/************************************************************
Function: monitorSftpUpload
Description: sftp上传功能函数
Input: uploadURL:  sftp://user:pwd@ip/filename
Output:
Return:
Others:
************************************************************/
int monitorSftpUpload(char *uploadFileName, char *uploadURL)
{
    int   res       = 0;
    void* lCurl     = NULL;
    long  lSize     = 0;

    printf("sftp upload start\n");
    FILE* lReadFp = fopen(uploadFileName, "r");

    if (!lReadFp) {
        printf("fopen error\n");
        return -1;
    }

	fseek(lReadFp, 0L, SEEK_END);
	lSize = ftell(lReadFp);
	fseek(lReadFp, 0L, SEEK_SET);

    lCurl = curl_easy_init(); /* start a libcurl easy session */
    if (!lCurl) {
        printf("curl init error\n");
        fclose(lReadFp);
        return -1;
    }

    curl_easy_setopt(lCurl, CURLOPT_SSH_AUTH_TYPES, CURLSSH_AUTH_PASSWORD);
    curl_easy_setopt(lCurl, CURLOPT_URL, uploadURL);
    curl_easy_setopt(lCurl, CURLOPT_CONNECTTIMEOUT, 10); /* connnect time */
    curl_easy_setopt(lCurl, CURLOPT_UPLOAD, 1L); /* enable upload */
    curl_easy_setopt(lCurl, CURLOPT_VERBOSE, 1L); /* debug use */
    curl_easy_setopt(lCurl, CURLOPT_READDATA, lReadFp);
    curl_easy_setopt(lCurl, CURLOPT_INFILESIZE, lSize);
    curl_easy_setopt(lCurl, CURLOPT_FTP_CREATE_MISSING_DIRS, 1); /* auto make directory */
    res = curl_easy_perform(lCurl); /* will block */
    if (res != 0)
        printf("curl failed\n");
    curl_easy_cleanup(lCurl); /* end a libcurl easy session */

    fclose(lReadFp);

    printf("sftp Upload End!\n");
    return res;
}


/************************************************************
Function: monitorGetPidByName
Description: 根据PID得到程序名称
Input:
Output:
Return:
Others:
************************************************************/
int monitorGetPidByName(char* processName)
{
    struct dirent *dirp;
    DIR *dp;
    int fd;
    char path[32] = {0};
    char buff[128] = {0};

    if((dp = opendir("/proc")) == NULL) {
        printf("read directory failed!\n");
        return -1;
    }

    while((dirp = readdir(dp)) != NULL) {
        if(strcmp(dirp->d_name, ".") == 0 ||
           strcmp(dirp->d_name, "..") == 0 ||
           dirp->d_type != DT_DIR)
            continue;
        if(atoi(dirp->d_name) != 0) {
            sprintf(path, "/proc/%s/cmdline", dirp->d_name);
            if((fd = open(path, O_RDONLY)) != -1) {
                memset(buff, 0, 128);
                read(fd, buff, 128);
                close(fd);
                if(strstr(buff, processName)) {
                    closedir(dp);
                    return atoi(dirp->d_name);
                }
            }
        }
    }
    closedir(dp);

    return 0;
}



//  远程抓包
int monitorRemoteCapture(moni_buf_t buf, int len)
{
    int port = 0;
    char ip[20] = {0};
    char express[1024] = {0};
    char *p1 = NULL;
    char *p2 = NULL;
    char *p = buf->buf + len;

    printf("buf->buf = %s, len = %d\n", buf->buf, len);
    printf("buf = %s\n", p);
    p1 = strcasestr(p, "ip:");

    if(p1) {
        p1 += 3;
        p2 = strchr(p, ' ');
        if(!p2)
            return -1;
        memcpy(ip, p1, p2 - p1);
    }
    p1 = strcasestr(p, "port:");
    if(p1) {
        p1 += 5;
        port = atoi(p1);
    }
    if(strlen(ip) > 0)
        sprintf(express, "host %s", ip);
    if(port > 0) {
        if(strlen(express) > 0)
            strcat(express, " and port ");
        else
            strcpy(express, "port ");
        sprintf(express + strlen(express), "%d", port);
    }

    p1 = strstr(p, "^time:");
    if(p1)
        g_pcap_time = atoi(p1 + 6);
    printf("pcap time is %d\n", g_pcap_time);

    p1 = strstr(p, "^filesize:");
    if(p1)
        g_pcap_size = atoi(p1 + 10);
    if((g_pcap_size <= 0) || (g_pcap_size > REMOTEPCAPFILESIZE)){
        g_pcap_size = REMOTEPCAPFILESIZE; // 设置为150M，防止size过大导致内存溢出
        // 在tcpdump应用内加了内存判断， 不用担心size过大导致内存溢出来， 设最大
    }
    printf("pcap filesize is %d\n", g_pcap_size);


    snprintf(g_pcap_command, 4096, "-i eth0 -N -n -ttt -A -v -X  -Q 20 -s0 -l %s", express);

    pthread_t pcap_pid = 0;
    printf("buf->buf = %s, len = %d, buf->len = %d\n", buf->buf, len, buf->len);
    p1 = strstr(buf->buf, "ioctl");
    if(!p1)
        return 0;
    printf("p1 = %s\n", p1);

    pthread_create(&pcap_pid, NULL, monitorRemoteCapureFunc, NULL);

    return 0;
}



void* monitorRemoteCapureFunc(void* arg)
{
    char fileName[64];
    char tcpdumpCommond[4096];
    int lastReportTime = 0;
    int lastReportLength = 0;
    int totalLength = 0;
    int pid = 0;
    //int pcapStartTime = 0;
    struct stat st;
    int len = strlen(g_monitorBuf.buf);

    time_t t;
	time(&t);
    struct tm *ctm = localtime(&t);

    sprintf(fileName, REMOTEPCAPFILE"%04d%02d%02d%02d%02d%02d",
            ctm->tm_year + 1900, ctm->tm_mon + 1, ctm->tm_mday,
            ctm->tm_hour, ctm->tm_min, ctm->tm_sec);

    //if(mid_ntp_status() <= 0)
    //    strcat(fileName, "-no-ntp-sync");
    strcat(fileName, REMOTEPCAPFILEEXT);
    sprintf(tcpdumpCommond, "tcpdump -w %s %s &", fileName, g_pcap_command);
    printf("tcpdumpCommond = %s\n", tcpdumpCommond);
    pthread_t pthread_pid = 0;
    pthread_create(&pthread_pid, NULL, monitorStartRemotePcap, tcpdumpCommond);
    usleep(100000);//防止stb_remotepcap_ioctl的数据发送和get_pcapfilesize数据发送时间重叠导致异常。


    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
	msec_t msec;
    msec = tp.tv_sec;
    msec = msec * 1000 + tp.tv_nsec / 1000000;


	//pcapStartTime = clock() / 1000;
    time_t startTime = 0;
    time_t currentTime = 0;
    time(&startTime);
    time(&currentTime);

    while(1) {
        time(&currentTime);
        //int currentTime = clock() / 1000;
        //printf("%d %d \n", currentTime, pcapStartTime);
        //if((currentTime - pcapStartTime) >= g_pcap_time && 0 != g_pcap_time)
        if((currentTime - startTime) >= g_pcap_time && 0 != g_pcap_time)
            break;
        sync();
        if(stat(fileName, &st) < 0) {
            if(errno == ENOENT) {
                continue;
            }
            perror("stat");
            break;
        }
        totalLength = st.st_size;
        if(st.st_size >= g_pcap_size && 0 != g_pcap_size)
            break;

        pid = monitorGetPidByName((char *)("tcpdump"));
        if (pid == 0)
            break;

        if((currentTime - startTime) > 0 && lastReportTime != currentTime) {
            sprintf(g_monitorBuf.buf + len, "inform^get_pcapfilesize:%ld^NULL", (long)st.st_size);
            g_monitorBuf.len = strlen(g_monitorBuf.buf);
            printf("g_monitorBuf.buf:%s\n", g_monitorBuf.buf + len);
            MonitorManager::GetInstance()->monitor_cmd_response(&g_monitorBuf);
            lastReportTime = currentTime;
            lastReportLength = st.st_size;
        }
        usleep(100000);
    }
    pid = monitorGetPidByName((char *)("tcpdump"));
    printf("pid = %d\n", pid);
    if(pid != 0) {
        kill(pid, SIGKILL);
        waitpid(pid, NULL, 0);
    }

    if(stat(fileName, &st) < 0)
        totalLength = g_pcap_size;
    else
        totalLength = st.st_size;
    if(totalLength != lastReportLength) {
        sprintf(g_monitorBuf.buf + len, "inform^get_pcapfilesize:%ld^NULL", (long)totalLength);
        g_monitorBuf.len = strlen(g_monitorBuf.buf);
        MonitorManager::GetInstance()->monitor_cmd_response(&g_monitorBuf);
    }
    // send file;
    monitorSendRemoteCaptureFile(fileName);

    sprintf(g_monitorBuf.buf + len, "inform^get_capfileuploadsize:%d^null", totalLength);
    g_monitorBuf.len = strlen(g_monitorBuf.buf);
    MonitorManager::GetInstance()->monitor_cmd_response(&g_monitorBuf);
    memset(g_monitorBuf.buf, 0, sizeof(g_monitorBuf.buf));
    return 0;

}


void* monitorStartRemotePcap(void* arg)
{
    int ret = 0;

    monitorExecCmd((char*)arg, "r", NULL, NULL);
    if(ret < 0) {
        printf("ERROR:: system");
    }
    return NULL;
}


void monitorSendRemoteCaptureFile(const char* filename)
{
    char uploadURL[200] = {0};
    char* env = NULL;
    char ip[32] = {0};
    char user[32] = "hwtcpdump";
    char pswd[32] = "HwdumpIsGood";
    int ret = 0;
    char *token = NULL;

    env = getenv("REMOTEPCAP_USERNAME");
    if(env && strlen(env) > 0)
        strncpy(user, env, sizeof(user));
    env = getenv("REMOTEPCAP_PASSWORD");
    if(env && strlen(env) > 0)
        strncpy(pswd, env, sizeof(pswd));

    monitorGetSTBPositionIP(ip);
    if(strlen(ip) > 0) {
        //sprintf(command, "sftp put %s %s:%s@%s", filename, user, pswd, ip);
        //MONITOR_LOG("command: %s\n", command);
        if ((token = strrchr((char *)filename, '/'))) {
            sprintf(uploadURL, "sftp://%s:%s@%s/%s", user, pswd, ip, (token + 1));
            ret = monitorSftpUpload((char *)filename, uploadURL);
            if(ret == 0)
                printf("upload ok\n");
        }
        else
            printf("filename error\n");
    }
    //MONITOR_LOG("Send ok\n");
    unlink(filename);
}


void monitorGetSTBPositionIP(char *ip)
{
    if(!ip)
        return;
    strcpy(ip, stbMonitorIP);
}


int monitorRunCommand(char *command)
{
	int ret = 0;
	//#ifdef ANDROID
		ret = system(command);
	//#else
		//yos_systemcall_runSystemCMD(command, &ret);
	//#endif

	return ret;
}

long long monitorGetUDiskFreeCapacity(char *UDiskPath)
{
    struct statfs statFS;
    memset(&statFS, 0, sizeof(statFS));

    long long freeKBytes = 0;

    if (!UDiskPath)
        return -1;

     if (statfs(UDiskPath, &statFS) == -1){
        printf("statfs failed for path->[%s]\n", UDiskPath);
        return(-1);
    }

    freeKBytes = (((long long)statFS.f_bsize * (long long)statFS.f_bfree)/(long long)1024);

    printf("freeBytes = %lld\n", freeKBytes);
    return freeKBytes;
}

