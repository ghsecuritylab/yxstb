
#include "StatisticLog.h"
#include "app_tr069_alarm.h"

#include "../StatisticHW/StatisticBase.h"
#include "TR069Assertions.h"
#include "curl/curl.h"
#include "pathConfig.h"

#include "tr069_port1.h"
#include "cryptoFunc.h"
#include "openssl/evp.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h> // temp
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


static StatisticLog* g_log = NULL;

pthread_mutex_t* StatisticLog::s_mutex = NULL;

/*************************************************
Description: 该日志模块专用作统计模块的日志上报，提供上报信息的操作接口
Input: 无
Return: 无
 *************************************************/
StatisticLog::StatisticLog()
{
    if(s_mutex == NULL) {
        s_mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
        pthread_mutex_init(s_mutex, NULL);
    }

    pthread_mutex_lock(s_mutex);
    memset(m_logServerUrl, 0, sizeof(m_logServerUrl));
    int i = 0;
    for(i = 0; i < MAX_FILE_LOG_NUM; i++) {
        m_savedFile[i] = "null";
    }
    m_nextSaveFileIndex = 0;
    pthread_mutex_unlock(s_mutex);
    #if defined(Sichuan)
        setEnable(0); // getStatisticLog()->SetEnable(0);
    #endif
}

StatisticLog::~StatisticLog()
{

}

void
StatisticLog::configReset()
{
    pthread_mutex_lock(s_mutex);
    m_logServerUrl[0] = 0;
    m_uploadInterval = 3600;
    m_recordInterval = 3600;
    pthread_mutex_unlock(s_mutex);
}

int
StatisticLog::setServerUrl(char *value)
{
    if (strncasecmp(value, "ftp://", 6)) {
        char output[256] = {0};
        char temp[256] = {0};
        unsigned char key[256] = {0};
        int len;

        app_TMS_aes_keys_set();
        app_TMS_aes_keys_get(key);
        len = EVP_DecodeBlock((unsigned char*)temp, (unsigned char*)value, strlen(value));
        aesEcbDecrypt(temp, len - 1, (char*)key, output, sizeof(output));
        pthread_mutex_lock(s_mutex);
        strcpy(m_logServerUrl, output);
        pthread_mutex_unlock(s_mutex);
    } else {
        pthread_mutex_lock(s_mutex);
        strcpy(m_logServerUrl, value);
        pthread_mutex_unlock(s_mutex);
    }
    LogTr069Debug("final m_logServerUrl = %s\n",m_logServerUrl);
    return 0;
}

int
StatisticLog::getServerUrl(char* buf, int size)
{
    if (size < strlen(m_logServerUrl))
        return -1;
    pthread_mutex_lock(s_mutex);
    strncpy(buf, m_logServerUrl, strlen(m_logServerUrl));
    buf[strlen(m_logServerUrl)] = '\0';
    pthread_mutex_unlock(s_mutex);
    return 0;
}

char*
StatisticLog::getServerUrlAddr()
{
    return (char*)m_logServerUrl; // 使用指针修改的没办法加锁
}

int
StatisticLog::getAESServerUrl(char *value, int size)
{
    char tmsAesLogUrl[512] = {0};
    unsigned char key[256] = {0};
    char temp[512] = {0};
    int ret;

    app_TMS_aes_keys_set();
    app_TMS_aes_keys_get(key);
    ret = aesEcbEncrypt(m_logServerUrl, strlen(m_logServerUrl), (char*)key, temp, sizeof(temp));
    EVP_EncodeBlock((unsigned char*)tmsAesLogUrl, (unsigned char*)temp, ret);//base64
    pthread_mutex_lock(s_mutex);
    strcpy(value, tmsAesLogUrl);
    pthread_mutex_unlock(s_mutex);
    LogTr069Debug("encrypt LogseverUrl = %s\n",value);
    return 0;
}

int
StatisticLog::setEnable(unsigned int n)
{
    pthread_mutex_lock(s_mutex);
    m_isEenable = n;
    pthread_mutex_unlock(s_mutex);
}

int
StatisticLog::getEnable()
{
    pthread_mutex_lock(s_mutex);
    int tEnable = m_isEenable;
    pthread_mutex_unlock(s_mutex);
    return tEnable;
}

int
StatisticLog::setUploadInterval(int n)
{
    pthread_mutex_lock(s_mutex);
    m_uploadInterval = n;
    pthread_mutex_unlock(s_mutex);
}

int
StatisticLog::getLogUploadInterval()
{
    pthread_mutex_lock(s_mutex);
    int tInterval = m_uploadInterval;
    pthread_mutex_unlock(s_mutex);
    return tInterval;
}

int
StatisticLog::setRecordInterval(int n)
{
    pthread_mutex_lock(s_mutex);
    m_recordInterval = n;
    pthread_mutex_unlock(s_mutex);
    return 0;
}

int
StatisticLog::getRecordInterval()
{
    pthread_mutex_lock(s_mutex);
    int n = m_recordInterval;
    pthread_mutex_unlock(s_mutex);
    return n;
}

//为系统参数管理，读取变量地址
int*
StatisticLog::getRecordIntervalAddr()
{
    return &m_recordInterval;
}

int*
StatisticLog::getUploadIntervalAddr()
{
    return &m_uploadInterval;
}

int
StatisticLog::getFailNum()
{
    pthread_mutex_lock(s_mutex);
    int n = m_nextSaveFileIndex;
    pthread_mutex_unlock(s_mutex);
    return n + 1;
}

int
StatisticLog::setFailNum(int n)
{
    pthread_mutex_lock(s_mutex);
    m_nextSaveFileIndex = n - 1;
    pthread_mutex_unlock(s_mutex);

    return m_nextSaveFileIndex + 1;
}

int*
StatisticLog::getFailNumAddr()
{
    return &m_nextSaveFileIndex;
}

int
StatisticLog::uplodSavedLog()
{
    FILE *fp = NULL;
    CURL *curl;
    struct stat file_info;
    curl_off_t fsize;
    char filePath[TEMP_FILE_PATH_LEN] = {0};

    for (int i = 0; i < MAX_FILE_LOG_NUM; i++) {
        if (!m_savedFile[i].compare("null")) {
            continue;
        }
        memset(filePath, 0, sizeof(filePath));
        sprintf(filePath, DEFAULT_MODULE_TR069_DATAPTH"/yx_statistic_stor_%d.cfg", i);

        /* get the file size of the local file */
        if(stat(filePath, &file_info)) {
            LogTr069Debug("Couldnt open '%s'\n", filePath);
            continue;
        }
        fsize = (curl_off_t)file_info.st_size;
        fp = fopen(filePath, "rb");

        curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3); /* transmission time */
            curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3); /* connnect time */
            curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
            char urlPath[TEMP_FILE_PATH_LEN] = {0};
            sprintf(urlPath, "%s/%s", m_logServerUrl, m_savedFile[i].c_str());
            LogTr069Debug("upload url uplodSavedLog[%s]\n", urlPath);
            curl_easy_setopt(curl, CURLOPT_URL, urlPath);
            curl_easy_setopt(curl, CURLOPT_READDATA, fp);
            curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)fsize);
            curl_easy_setopt(curl, CURLOPT_FTP_CREATE_MISSING_DIRS, 1);
            int ret = curl_easy_perform(curl);
            if (ret == CURLE_OK || ret == CURLE_PARTIAL_FILE) {
                fclose(fp); /* close the local file */
                remove(filePath);
                m_savedFile[i] = "null";
            } else {
                LogTr069Debug("curl_easy_perform error\n");
                fclose(fp); /* close the local file */
            }
            curl_easy_cleanup(curl);
        }
    }
    return 0;

}

int
StatisticLog::saveFileToFlash()
{
    std::string srcFile =  DEFAULT_TEMP_DATAPATH + m_fileName;
    char tempBuf[1024] = {0};

    if (access(srcFile.c_str(), 0)) {
        return -1;
    }

    setSavePoint(); //存储前设置结束时间

    LogTr069Debug("save file to flash\n");
    FILE *srcFp = fopen(srcFile.c_str(), "r");
    if (!srcFp) {
        LogTr069Debug("fopen srcfile error\n");
        return -1;
    }
    if (m_savedFile[m_nextSaveFileIndex].compare("null")) {//file saved use this index, remove this file first
        sprintf(tempBuf, DEFAULT_MODULE_TR069_DATAPTH"/yx_statistic_stor_%d.cfg", m_nextSaveFileIndex);
        remove(tempBuf);
    }
    m_savedFile[m_nextSaveFileIndex] = m_fileName;

    memset(tempBuf, 0, sizeof(tempBuf));
    sprintf(tempBuf, DEFAULT_MODULE_TR069_DATAPTH"/yx_statistic_stor_%d.cfg", m_nextSaveFileIndex);
    std::string dstFile = tempBuf;
    LogTr069Debug("\n dst save filename[%s]\n", dstFile.c_str());
    FILE *dstFp = fopen(dstFile.c_str(), "w+");
    if (!dstFp) {
        LogTr069Debug("fopen srcfile error\n");
        fclose(srcFp);
        return -1;
    }

    memset(tempBuf, 0, sizeof(tempBuf));
    while (1){
        int ret = fread(tempBuf, 1, 1024, srcFp);
        if (ret > 0) {
            fwrite(tempBuf, 1, ret, dstFp);
        } else {//data read finish
            break;
        }
    }
    fclose(srcFp);
    fclose(dstFp);
    remove(srcFile.c_str());

    if (++m_nextSaveFileIndex >= MAX_FILE_LOG_NUM)
        m_nextSaveFileIndex = 0;//roll back to list start
    return 0;

}

int
StatisticLog::upload(char* buf)
{
    FILE *fp = NULL;
    CURL *curl;
    struct stat file_info, st;
    curl_off_t fsize;
    char filePath[TEMP_FILE_PATH_LEN] = {0};
    sprintf(filePath, "%s/%s", DEFAULT_TEMP_DATAPATH, m_fileName.c_str());

    stat("/dev/random", &st);
    if(!S_ISDIR(st.st_mode)) {
        if(mkdir(DEFAULT_TEMP_DATAPATH, 0755) == -1)
            LogTr069Debug("mkdir  %s error", DEFAULT_TEMP_DATAPATH);
    }

    /*create temp file ,use for curl upload*/
    fp = fopen(filePath, "wb");
    if(!fp)
        LogTr069Debug("\n create file %s err\n", filePath);
    fwrite(buf, 1, strlen(buf), fp);
    fclose(fp);

    /* get the file size of the local file */
    if(stat(filePath, &file_info)) {
        LogTr069Debug("Couldnt open '%s'\n", filePath);
        return 1;
    }
    fsize = (curl_off_t)file_info.st_size;
    fp = fopen(filePath, "rb");
    // eg: "ftp://zl:111111@110.1.1.154/111"
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3); /* transmission time */
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3); /* connnect time */
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        char urlPath[TEMP_FILE_PATH_LEN] = {0};
        sprintf(urlPath, "%s/%s", m_logServerUrl, m_fileName.c_str());
        LogTr069Debug("----urlPath--%s-----\n", urlPath);
        curl_easy_setopt(curl, CURLOPT_URL, m_logServerUrl);
        curl_easy_setopt(curl, CURLOPT_READDATA, fp);
        curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)fsize);
        curl_easy_setopt(curl, CURLOPT_FTP_CREATE_MISSING_DIRS, 1);
        int ret = curl_easy_perform(curl);
        if (ret == CURLE_OK || ret == CURLE_PARTIAL_FILE) { // 上传日志成功，再上传之前没有上传成功的日志
            uplodSavedLog();
            fclose(fp); /* close the local file */
            remove(filePath);
        } else {  // 上传日志不成功，储存临时文件 到 /root/yx_statistic_store_[].cfg
            LogTr069Debug("curl_easy_perform error ret  [%d]\n", ret);
#ifdef Sichuan
            app_report_file_access_alarm( );
#endif
            saveFileToFlash();
            fclose(fp); /* close the local file */
        }
        curl_easy_cleanup(curl);
    }
    return 0;
}

int StatisticLog::setFileName(char* buf)
{
    m_fileName = std::string(buf);
}


extern "C"
{
int
tr069StatisticSetLogFileName(char* buf)
{
    return g_log->setFileName(buf);
}
char* tr069StatisticGetLogServerUrlAddr()
{
    return g_log->getServerUrlAddr();
}

StatisticLog* tr069StatisticLogInit()
{
    if(g_log)
       return NULL;

    g_log = new StatisticLog();
    return g_log;
}

StatisticLog* getStatisticLog()
{
    return g_log;
}

/*------------------------------------------------------------------------------
 ------------------------------------------------------------------------------*/
unsigned int tr069StatisticGetLogenable(void)
{
    return g_log->getEnable();
}

int tr069StatisticSetLogenable(unsigned int value)
{
    return g_log->setEnable(value);
}

/*------------------------------------------------------------------------------
    日志文件服务器的URL信息，应该包含上传URL的鉴权信息（与机顶盒连接终端管理服务器的鉴权方式一致）
 ------------------------------------------------------------------------------*/
int tr069StatisticGetLogServerUrl(char *value, int size)
{
    return g_log->getServerUrl(value, size);
}

int tr069StatisticSetLogServerUrl(char* value)
{
    return g_log->setServerUrl(value);
}

void tr069StatisticGetLogAESServerUrl(char *value, int size)
{
    g_log->getAESServerUrl(value, size);
}

/*------------------------------------------------------------------------------
    性能监测参数文件上报间隔
    单位：s 默认：3600（即1小时）
    该参数设置为0时，表示关闭性能监测参数文件上报功能
 ------------------------------------------------------------------------------*/
int tr069StatisticGetLogUploadInterval(void)
{
    return g_log->getLogUploadInterval();
}

void tr069StatisticSetLogUploadInterval(int n)
{
    g_log->setUploadInterval(n);
}
/*------------------------------------------------------------------------------
    性能监测统计参数的记录周期时长
    单位：s 默认：3600（即1小时）
    统计起始为每次开机，到设定的统计周期时长后，启动新的统计周期。
    如不到设定的统计周期时长就关机，则结束这个周期。
    在启动新的统计周期时，应把前个周期的记录数据上传到网管平台
 ------------------------------------------------------------------------------*/
int tr069StatisticGetLogRecordInterval(void)
{
    return g_log->getRecordInterval();
}

void tr069StatisticSetLogRecordInterval(int n)
{
    g_log->setRecordInterval(n);
}

}
