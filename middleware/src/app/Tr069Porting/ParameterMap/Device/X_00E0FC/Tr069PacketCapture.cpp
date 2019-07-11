#include "Tr069PacketCapture.h"

#include "Tr069FunctionCall.h"
#include "TR069Assertions.h"
#include "tr069_port1.h"
#include "NetworkFunctions.h"

extern "C"
{
#include "mid/mid_time.h"
#include "mid/mid_timer.h"
#include "cryptoFunc.h"
#include "openssl/sha.h"
#include "openssl/evp.h"
int yos_systemcall_runSystemCMD(char *buf, int *ret);
}

#include "curl/curl.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>


struct PacketCapture {
    int State;// 1、未抓包 None 2、开始抓包 Requested 3、正在抓包 4、抓包失败 5、正在上传6、上传完成7、上传失败
    int Duration;//抓包持续时长，单位：秒
    char IP[16];//抓包过滤条件，IP
    int Port;//抓包过滤条件，Port
    char UploadURL[1024];//上传抓包文件的服务器地址，格式：sftp://ip:port/path，其中port和path是可选的
    char Username[33];//文件服务器的用户名
    char Password[33];//文件服务器的密码，需按安全红线加密
};

#define PACKETCAPTUREFILESIZE 10*1024*1024

struct PacketCapture gPacketCapture = {1, 0, "", 0, "", "", ""};
static char gPacketCaptureCommand[4096] = {0};
static int gOpenPacketCaptureFlag = 1;

int getTr069OpenPacketCaptureFlag()
{
    return gOpenPacketCaptureFlag;
}

void setTr069OpenPacketCaptureFlag(int flag)
{
    LogTr069Debug("allow tr069 packetcaptureflag is %d\n", flag);
    if (flag != gOpenPacketCaptureFlag)
        gOpenPacketCaptureFlag = flag;

    return;
}

void* startTr069PacketCapture(void* arg)
{
    int ret = 0;

    if(arg) {
        gPacketCapture.State = 3;
        LogTr069Debug("start packet capture!\n");

        yos_systemcall_runSystemCMD((char*)arg, &ret);
        if (ret < 0) {
            gPacketCapture.State = 4;
            LogTr069Error("ERROR:: system\n");
        }
    }
    return NULL;
}

static void sendTro69PacketCapturefile(const char* filename)
{
    char command[2048] = {0};
    int ret = 0;

    if (!filename || !strlen(filename)) {
        LogTr069Error("error!filename is null!\n");
        return;
    }

    if (strlen(gPacketCapture.UploadURL) > 0) {
        sprintf(command, "sftp put %s %s:%s@%s", filename, gPacketCapture.Username, gPacketCapture.Password, gPacketCapture.UploadURL + strlen("sftp://"));
        gPacketCapture.State = 5;
        LogTr069Debug("send packdt capture file!\n");
        yos_systemcall_runSystemCMD(command, &ret);
        if (ret < 0) {
            if (5 == gPacketCapture.State)
                gPacketCapture.State = 7;
            LogTr069Error("ERROR:: system\n");
        } else {
            if (5 == gPacketCapture.State)
                gPacketCapture.State = 6;
            LogTr069Debug("Send ok\n");
        }
    }
    unlink(filename);
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
        LogTr069Error("error!processname is NULL!\n");
        return 0;
    }

    if ((dp = opendir("/proc")) == NULL) {
        LogTr069Error("read directory failed!\n");
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
        yos_systemcall_runSystemCMD(command, &ret);
        if (ret < 0)
            LogTr069Error("ERROR:: system\n");
    }
}


/************************************************************
Function: tr069SftpUpload
Description: sftp上传功能函数
Input: uploadURL:  sftp://user:pwd@ip/filename
Output:
Return:
Others: TODO，本函数是从monitor复制过来， 去monitor调不好， 最好做成统一接口。。
************************************************************/
int tr069SftpUpload(char *uploadFileName, char *uploadURL)
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


static void* tr069PacketCapturePthreadFunc(void* arg)
{
    char fileName[64] = {""};
    char mac[13] = {0};
    char tcpdumpCommond[4096];
    char uploadURL[256] = {0};
    char fileNamePath[256] = {0};
    int captureSize  = 0;
    int pcapStartTime = 0;
    struct stat st;
    int free_size = 0;
    long int t = (long int)(mid_time());
    struct tm *ctm = localtime(&t);
    struct sysinfo s_info;
    sysinfo(&s_info);
    free_size = s_info.freeram;

    if(free_size > 0 )
        captureSize = free_size  - 2*1024*1024;

    LogTr069Debug("MAX captureSize = %dKB   ---TOTAL free_size %dKB\n", captureSize /1024, free_size /1024);
    network_tokenmac_get(mac, 13, 0);
    sprintf(fileName, "%s_%04d%02d%02d%02d%02d%02d.pcap",
            mac, ctm->tm_year + 1900, ctm->tm_mon + 1, ctm->tm_mday,
            ctm->tm_hour, ctm->tm_min, ctm->tm_sec);
    sprintf(fileNamePath, "/var/%s", fileName);
    sprintf(tcpdumpCommond, "tcpdump -w %s %s", fileNamePath, gPacketCaptureCommand);
    LogTr069Debug("tcpdumpCommond = %s\n", tcpdumpCommond);
    pthread_t pthread_pid = 0;
    pthread_create(&pthread_pid, NULL, startTr069PacketCapture, tcpdumpCommond);
    pcapStartTime = mid_clock() / 1000;
    while (1) {
        int currentTime = mid_clock() / 1000;
        if ((currentTime - pcapStartTime) >= gPacketCapture.Duration)
            break;
        sync();
        if (stat(fileNamePath, &st) < 0) {
            if (errno == ENOENT) {
                continue;
            }
            perror("stat");
            break;
        }
        //totalLength = st.st_size;

        if (st.st_size >= captureSize){
            LogTr069Debug("runout of memory, %d/%d!!!!\n",st.st_size,  captureSize);
            break;
        }
        usleep(100000);
    }
    int pid = tr069GetPidByName("tcpdump");
    LogTr069Debug("pid = %d\n", pid);
    if (pid == 0)
        return 0;

    kill(pid, SIGKILL);
    waitpid(pid, NULL, 0);

//    if (stat(fileName, &st) < 0)
//        totalLength = PACKETCAPTUREFILESIZE;
//    else
//        totalLength = st.st_size;

    // send file;
   // sendTro69PacketCapturefile(fileName);

    sprintf(uploadURL, "sftp://%s:%s@%s/%s",  gPacketCapture.Username,  gPacketCapture.Password, gPacketCapture.UploadURL + strlen("sftp://"), fileName);
    LogTr069Debug("fileName = %s-----url = %s\n", fileName, uploadURL);
    gPacketCapture.State = 5;

    if (tr069SftpUpload(fileNamePath,  uploadURL)){
        LogTr069Error("stfp upload pcap file failed!!!\n");
        gPacketCapture.State = 7;

    }else{
        LogTr069Debug("stfp upload success, delete pcap file!\n");
        gPacketCapture.State = 6;
    }
    tr069DeleteFileByFileName(fileNamePath);
}

static void beforeTr069PacketCaptureStart(int arg)
{
    char express[1024] = {0};

    memset(express, 0, 1024);
    if (strlen(gPacketCapture.IP) >= 7) {
        LogTr069Debug("debug!ip = [%s]", gPacketCapture.IP);
        sprintf(express, "host %s", gPacketCapture.IP);
    }

    if (gPacketCapture.Port > 0 && gPacketCapture.Port <= 65535) {
        if (strlen(express) > 0)
            strcat(express, " and ");
        sprintf(express + strlen(express), "port %d", gPacketCapture.Port);
    }

    snprintf(gPacketCaptureCommand, 4096, "-i eth0 -N -n -ttt -A -v -X -l %s", express);
    pthread_t pcap_pid = 0;
    pthread_create(&pcap_pid, NULL, tr069PacketCapturePthreadFunc, NULL);
}



void tr069_port_set_PacketCapture_Password(char *value)
{

}

///////////////////////////////////
static int getTr069PortPacketCaptureState(char* str, unsigned int val)
{
    snprintf(str, val, "%d", gPacketCapture.State);

    return 0;
}

static int setTr069PortPacketCaptureState(char* str, unsigned int val)
{
    int value = atoi(str);

    if (value > 7 || value < 1)
        return -1;

    if (!gOpenPacketCaptureFlag) {
        LogTr069Error("error!do not allow to packet camture!\n");
        return -1;
    }

    if (2 == value) {
        if (3 == gPacketCapture.State || 5== gPacketCapture.State ){
            LogTr069Error("tcpdump is running or uploading ,please waiting... ... finish!!!!\n");
	     return -1;
        }
        gPacketCapture.State = 3;
        mid_timer_create(3, 1, beforeTr069PacketCaptureStart, 0);
    }

    return 0;
}

static int getTr069PortPacketCaptureDuration(char* str, unsigned int val)
{
    snprintf(str, val, "%d", gPacketCapture.Duration);
    LogTr069Debug("duration=[%d]\n", gPacketCapture.Duration);
    return 0;
}

static int setTr069PortPacketCaptureDuration(char* str, unsigned int val)
{
    int value = atoi(str);

    if (value <= 0)
        return -1;

    LogTr069Debug("duration=[%d]\n", value);
    if (value != gPacketCapture.Duration)
        gPacketCapture.Duration = value;

    return 0;
}

static int getTr069PortPacketCaptureIP(char* str, unsigned int val)
{
    if (!str)
        return -1;

    strcpy(str, gPacketCapture.IP);
    LogTr069Debug("value=[%s]\n", str);

    return 0;
}

static int setTr069PortPacketCaptureIP(char* str, unsigned int val)
{
    if (!str)
        return -1;

    LogTr069Debug("value=[%s]\n", str);
    strcpy(gPacketCapture.IP, str);
    gPacketCapture.IP[15] = 0;

    return 0;
}

static int getTr069PortPacketCapturePort(char* str, unsigned int val)
{
    snprintf(str, val, "%d", gPacketCapture.Port);
    LogTr069Debug("port=[%d]\n", gPacketCapture.Port);

    return 0;
}

static int setTr069PortPacketCapturePort(char* str, unsigned int val)
{
    int value = atoi(str);

    if (value <= 0 || value > 65535)
        return -1;

    LogTr069Debug("port=[%d]\n", value);
    if (value != gPacketCapture.Port)
        gPacketCapture.Port = value;

    return 0;
}

static int getTr069PortPacketCaptureUploadURL(char* str, unsigned int val)
{
    if (!str)
        return -1;

    strcpy(str, gPacketCapture.UploadURL);
    LogTr069Debug("value=[%s]\n", str);

    return 0;
}

static int setTr069PortPacketCaptureUploadURL(char* str, unsigned int val)
{
    if (!str)
        return -1;

    LogTr069Debug("value=[%s]\n", str);
    if (!strncmp(str, "sftp://", strlen("sftp://"))) {
        strcpy(gPacketCapture.UploadURL, str);
        gPacketCapture.UploadURL[1023] = 0;
    }

    return 0;
}

static int getTr069PortPacketCaptureUsername(char* str, unsigned int val)
{
    if (!str)
        return -1;

    strcpy(str, gPacketCapture.Username);
    LogTr069Debug("value=[%s]\n", str);

    return 0;
}

static int setTr069PortPacketCaptureUsername(char* str, unsigned int val)
{
    if (!str)
        return -1;

    LogTr069Debug("value=[%s]\n", str);
    strcpy(gPacketCapture.Username, str);
    gPacketCapture.Username[32] = 0;

    return 0;
}

static int getTr069PortPacketCapturePassword(char* str, unsigned int val)
{
	if (!str)
			return -1;

#if  _HW_BASE_VER_ >= 58
		char output[256] = {'\0'};
		unsigned char key[256] = {0};
		char temp[512] = {0};
		int ret;

		app_TMS_aes_keys_get(key);
		ret = aesEcbEncrypt(gPacketCapture.Password, strlen(gPacketCapture.Password), (char*)key, temp, sizeof(temp));
		EVP_EncodeBlock((unsigned char*)output, (const unsigned char*)temp, ret);//base64
		memset(str, 0, strlen(output));
		strncpy(str, output, (strlen(output)));
		str[strlen(output)] = '\0';
#else
		strcpy(str, gPacketCapture.Password);
#endif
		LogTr069Debug("value=[%s]\n", str);

    return 0;
}

static int setTr069PortPacketCapturePassword(char* str, unsigned int val)
{
	if (!str)
			return -1;

#if _HW_BASE_VER_ >=58
	//	  int ret = 0;
		unsigned char output[33] = {'\0'};

		LogTr069Debug("tr069_port_set_PacketCapture_Password value=%s\n", str);
		if(str == NULL)
			return -1;
		if(strlen(str) < 16 || strlen(str) % 4 != 0) {
			strcpy(gPacketCapture.Password, str);
			gPacketCapture.Password[32] = 0;
			return -1;
		}
		unsigned char key[256] = {0};
		char temp[256] = {0};
		app_TMS_aes_keys_get(key);
		EVP_DecodeBlock((unsigned char*)temp, (unsigned char*)str, strlen(str));
		aesEcbDecrypt(temp, strlen(temp), (char*)key, (char*)output, sizeof(output));
		memset(str, 0, strlen(str));
		strncpy(str, (char*)output, 33);
		str[33] = '\0';
#endif
		LogTr069Debug("value=[%s]\n", str);
		strcpy(gPacketCapture.Password, str);
		gPacketCapture.Password[32] = 0;

    return 0;
}


Tr069PacketCapture::Tr069PacketCapture()
	: Tr069GroupCall("PacketCapture")
{

    Tr069Call* PacketCaptureState     = new Tr069FunctionCall("State", getTr069PortPacketCaptureState, setTr069PortPacketCaptureState);
    Tr069Call* PacketCaptureDuration  = new Tr069FunctionCall("Duration", getTr069PortPacketCaptureDuration, setTr069PortPacketCaptureDuration);
    Tr069Call* PacketCaptureIP        = new Tr069FunctionCall("IP", getTr069PortPacketCaptureIP, setTr069PortPacketCaptureIP);
    Tr069Call* PacketCapturePort      = new Tr069FunctionCall("Port", getTr069PortPacketCapturePort, setTr069PortPacketCapturePort);
    Tr069Call* PacketCaptureUploadURL = new Tr069FunctionCall("UploadURL", getTr069PortPacketCaptureUploadURL, setTr069PortPacketCaptureUploadURL);
    Tr069Call* PacketCaptureUsername  = new Tr069FunctionCall("Username", getTr069PortPacketCaptureUsername, setTr069PortPacketCaptureUsername);
    Tr069Call* PacketCapturePassword  = new Tr069FunctionCall("Password", getTr069PortPacketCapturePassword, setTr069PortPacketCapturePassword);



    regist(PacketCaptureState->name(), PacketCaptureState);
    regist(PacketCaptureDuration->name(), PacketCaptureDuration);
    regist(PacketCaptureIP->name(), PacketCaptureIP);
    regist(PacketCapturePort->name(), PacketCapturePort);
    regist(PacketCaptureUploadURL->name(), PacketCaptureUploadURL);
    regist(PacketCaptureUsername->name(), PacketCaptureUsername);
    regist(PacketCapturePassword->name(), PacketCapturePassword);


}

Tr069PacketCapture::~Tr069PacketCapture()
{
}
