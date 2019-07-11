#include "tr069_port_PacketCapture.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <dirent.h>
#include <fcntl.h>
#include "../common/curl.h"
#include "nm_dbg.h"
#include "../common/mid_timer.h"
struct PacketCapture {
    int State;// 1、未抓包 None 2、开始抓包 Requested 3、正在抓包 4、抓包失败 5、正在上传6、上传完成7、上传失败
    int Duration;//抓包持续时长，单位：秒
    char IP[16];//抓包过滤条件，IP
    int Port;//抓包过滤条件，Port
    char UploadURL[1024];//上传抓包文件的服务器地址，格式：sftp://ip:port/path，其中port和path是可选的
    char Username[33];
    char Password[33];
};

#define PACKETCAPTUREFILESIZE 10*1024*1024

struct PacketCapture gPacketCapture = {1, 0, "", 0, "", "", "",0};
static char gPacketCaptureCommand[4096] = {0};
static unsigned int gRebeginFlag = 0;
static int var = 0;
/************************************************************
Function: sftpUpload
Description: sftp上传功能函数
Input: uploadURL:  sftp://user:pwd@ip/filename
Output:
Return:
Others:
************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

typedef long long mid_msec_t;

mid_msec_t mid_clock(void)
{
    mid_msec_t msec;
    struct timespec tp;

    clock_gettime(CLOCK_MONOTONIC, &tp);

    msec = tp.tv_sec;
    msec = msec * 1000 + tp.tv_nsec / 1000000;
    return msec;
}
static int sftpUpload(char *uploadFileName, char *uploadURL)
{
    int   res       = 0;
    void* lCurl     = NULL;
    long  lSize     = 0;
    char  lMac[64]  = { 0 };
    char  lFtpUrl[256] = { 0 };

    nm_msg("sftp upload start\n");
    FILE* lReadFp = fopen(uploadFileName, "r");

    if (!lReadFp) {
        nm_msg("fopen error\n");
        return -1;
    }

	fseek(lReadFp, 0L, SEEK_END);
	lSize = ftell(lReadFp);
	fseek(lReadFp, 0L, SEEK_SET);

    lCurl = curl_easy_init(); /* start a libcurl easy session */
    if (!lCurl) {
        nm_msg("curl init error\n");
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
        nm_msg("failed: %s\n", curl_easy_strerror(res));
    curl_easy_cleanup(lCurl); /* end a libcurl easy session */

    fclose(lReadFp);

    nm_msg("sftp Upload End!\n");
    return res;
}

static void startTr069PacketCapture(void* arg)
{
    int ret = 0;

    if(arg) {
        gPacketCapture.State = 3;
        nm_msg("start packet capture!\n");
        ret = system((char*)arg);
	 nm_msg("-------------------------------ret = %d\n",ret);
    }
    return;
}

static int tr069GetPidByName(char* processName)
{
    struct dirent *dirp;
    DIR *dp;
    int fd;
    char path[32] = {0};
    char buff[128] = {0};

    if (!processName || !strlen(processName)) {
        nm_msg("error!processname is NULL!\n");
        return 0;
    }

    if ((dp = opendir("/proc")) == NULL) {
        nm_msg("read directory failed!\n");
        return 0;
    }

    while ((dirp = readdir(dp)) != NULL) {
        if (strcmp(dirp->d_name, ".") == 0 ||
           strcmp(dirp->d_name, "..") == 0 ||
           dirp->d_type != DT_DIR)
            continue;
        if (atoi(dirp->d_name) != 0) {
            sprintf(path, "/proc/%s/cmdline", dirp->d_name);
            if((fd = open(path, O_RDONLY)) != -1) {
                memset(buff, 0, 128);
                read(fd, buff, 128);
                close(fd);
                if (strstr(buff, processName)) {
                    closedir(dp);
                    return atoi(dirp->d_name);
                }
            }
        }
    }
    closedir(dp);

    return 0;
}

static void tr069DeleteFileByFileName(char* filename)
{
    char command[2048] = {""};
    int ret = 0;

    if (filename && strlen(filename) > 0) {
        sprintf(command, "rm %s -rf", filename);
        system(command);
        if (ret < 0)
            nm_msg("ERROR:: system\n");
    }
}
static void beforeTr069PacketCaptureStart(void)
{
    char express[1024] = {0};

    memset(express, 0, 1024);
    if (strlen(gPacketCapture.IP) >= 7) {
        nm_msg("debug!ip = [%s]", gPacketCapture.IP);
        sprintf(express, "host %s", gPacketCapture.IP);
    }
#if 0
    if (gPacketCapture.Port > 0 && gPacketCapture.Port <= 65535) {
        if (strlen(express) > 0)
            strcat(express, " and ");
        sprintf(express + strlen(express), "port %d", gPacketCapture.Port);
    }
#endif
    snprintf(gPacketCaptureCommand, 4096, "-i eth0 -N -n -ttt -A -v -X -l %s", express);
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
static void tr069PacketCapturePthreadFunc(int arg)
{
    char fileName[64] = {""};
    char mac[32] = {0};
    char tcpdumpCommond[4096];
    char uploadURL[256] = {0};
    int pcapStartTime = 0;
    char fileNamePath[256] = {0};
    struct stat st;

    long int t = (long int)time(NULL);
    struct tm *ctm = localtime(&t);
    char newMac[32] = {0};
    beforeTr069PacketCaptureStart();
    tr069_port_getValue("Device.LAN.MACAddress", mac, 32);
    changeMacAddress(mac, newMac);
    nm_msg("mac = %s, newMac = %s\n", mac, newMac);
    sprintf(fileName, "%s_%04d%02d%02d%02d%02d%02d.pcap",
            newMac, ctm->tm_year + 1900, ctm->tm_mon + 1, ctm->tm_mday,
            ctm->tm_hour, ctm->tm_min, ctm->tm_sec);
    sprintf(fileNamePath, "/var/%s", fileName);
 //   if (mid_ntp_status() <= 0)
 //       strcat(fileName, "-no-ntp-sync");
    sprintf(tcpdumpCommond, "tcpdump -w %s %s", fileNamePath, gPacketCaptureCommand);

	nm_msg("tcpdumpCommond = %s\n", tcpdumpCommond);
    unsigned int pthread_pid = 0;
    pthread_create(&pthread_pid, NULL, (void *)startTr069PacketCapture, tcpdumpCommond);
    pcapStartTime = mid_clock() / 1000;
    while (1) {
        int currentTime = mid_clock() / 1000;
        if ((currentTime - pcapStartTime) >= gPacketCapture.Duration || gRebeginFlag)
            break;
        sync();
        if (stat(fileName, &st) < 0) {
            if (errno == ENOENT) {
                continue;
            }
            perror("stat");
            break;
        }
        //totalLength = st.st_size;
        if (st.st_size >= PACKETCAPTUREFILESIZE)
            break;

        usleep(100000);
    }
    int pid = tr069GetPidByName("tcpdump");
    nm_msg("pid = %d\n", pid);
    if (pid == 0)
        return 0;

    kill(pid, SIGKILL);
    waitpid(pid, NULL, 0);

    nm_msg("gRebeginFlag = %d\n", gRebeginFlag);
    if (gRebeginFlag) {
        tr069DeleteFileByFileName(fileName);
        gRebeginFlag = 0;
        return ;
    }
    sprintf(uploadURL, "sftp://%s:%s@%s/%s",  gPacketCapture.Username,  gPacketCapture.Password, gPacketCapture.UploadURL + strlen("sftp://"), fileName);
    //nm_msg("fileName = %s-----url = %s\n", fileName, uploadURL);
    gPacketCapture.State = 5;

    if (sftpUpload(fileNamePath,  uploadURL)){
        nm_msg("stfp upload pcap file failed!!!\n");
	  gPacketCapture.State = 7;
    } else{
        nm_msg("stfp upload success, delete pcap file!\n");
   	  gPacketCapture.State = 6;
   }
        tr069DeleteFileByFileName(fileNamePath);
}

void tr069SetPacketCaptureParamValue(char * name , char *str, int len)
{
    int encrypt = 0;
    char mark[12] = {0};
    nm_msg("set PacketCapture.name[%s] = %s\n",name, str);
    tr069_port_getValue("Device.X_00E0FC.IsEncryptMark", mark, 12);
    encrypt = atoi(mark);
    if(!strcmp(name, "State")){
        gPacketCapture.State = atoi(str);
    }
    else if(!strcmp(name, "Duration")){
	 gPacketCapture.Duration = atoi(str);
    }
    else if(!strcmp(name, "IP")){
        strcpy(gPacketCapture.IP, str);
    }
    else if(!strcmp(name, "Port")){
        gPacketCapture.Port = atoi(str);
    }
    else if(!strcmp(name, "UploadURL")){
        strcpy(gPacketCapture.UploadURL, str);
    }
    else if(!strcmp(name, "Username")){
        strcpy(gPacketCapture.Username, str);
    }
    else if(!strcmp(name, "Password")){
        if(encrypt  != 0)
	     decryptACSCiphertext(str, gPacketCapture.Password);
       else
            strcpy(gPacketCapture.Password, str);
    }else
       nm_msg("unknown param!\n");
    var = var +1;
    nm_msg("var = %d\n",var);
    if(var == 7){
        mid_timer_create(2, 1, tr069PacketCapturePthreadFunc, 0);
	  var = 0;
    }
		
}
void tr069GetPacketCaptureParamValue(char *name , char *str, int len)
{
    int encrypt = 0;
    char mark[12] = {0};

    nm_msg("get PacketCapture.name[%s] = %s\n",name, str);
    tr069_port_getValue("Device.X_00E0FC.IsEncryptMark", mark, 12);
    encrypt = atoi(mark);

    if(!strcmp(name, "State")){
	 sprintf(str, "%d", gPacketCapture.State);
    }
    else if(!strcmp(name, "Duration")){
        sprintf(str, "%d", gPacketCapture.Duration);
    }
    else if(!strcmp(name, "IP")){
        strcpy(str, gPacketCapture.IP);
    }
    else if(!strcmp(name, "Port")){
        sprintf(str, "%d", gPacketCapture.Port);
    }
    else if(!strcmp(name, "UploadURL")){
        strcpy(str, gPacketCapture.UploadURL);
    }
    else if(!strcmp(name, "Username")){
        strcpy(str, gPacketCapture.Username);
    }
    else if(!strcmp(name, "Password")){
        if(encrypt != 0)
            encryptACSCiphertext(gPacketCapture.Password, str);
        else
	        strcpy(str, gPacketCapture.Password);
    }else
       nm_msg("unknown param!\n");
}
#ifdef __cplusplus
}
#endif
