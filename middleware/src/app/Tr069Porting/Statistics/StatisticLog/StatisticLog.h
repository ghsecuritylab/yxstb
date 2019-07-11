#ifndef _StatisticLog_H_
#define _StatisticLog_H_


#include <pthread.h>

#define TEMP_FILE_PATH_LEN 256
#define LOG_SERVER_URL_SIZE 256
#define MAX_FILE_LOG_NUM 3

#ifdef __cplusplus
#include <string>
class StatisticLog
{
public:
    StatisticLog();
    ~StatisticLog();

public:
    void configReset();
    int setServerUrl(char* value);
    int getServerUrl(char* buf, int size);
    char* getServerUrlAddr();
    int setAESServerUrl(char *value);
    int getAESServerUrl(char *value, int size);
    int setEnable(unsigned int n);
    int getEnable();
    int setUploadInterval(int n);
    int getLogUploadInterval();
    int setRecordInterval(int n);
    int getRecordInterval();
    int* getRecordIntervalAddr();
    int* getUploadIntervalAddr();
    int getFailNum();
    int setFailNum(int n);
    int* getFailNumAddr();
    
    int setFileName(char* buf);
    int upload(char* buf);
    int uplodSavedLog();
    int saveFileToFlash();
    
private:
    char m_logServerUrl[LOG_SERVER_URL_SIZE];
    unsigned int m_isEenable;
    
    int m_uploadInterval;
    int m_recordInterval;
    int m_nextSaveFileIndex;
    
    std::string m_fileName;
    std::string m_savedFile[MAX_FILE_LOG_NUM];

    static pthread_mutex_t* s_mutex;
};

extern "C"
{    
StatisticLog* tr069StatisticLogInit();
StatisticLog* getStatisticLog();

int tr069StatisticSetLogFileName(char* tBuf);

unsigned int tr069StatisticGetLogenable(void);
int tr069StatisticSetLogenable(unsigned int value);

int tr069StatisticGetLogServerUrl(char *value, int size);
int tr069StatisticSetLogServerUrl(char* value);
char* tr069StatisticGetLogServerUrlAddr(); //全局的参数管理会直接修改该值

void tr069StatisticSetLogAESServerUrl(char *value);
void tr069StatisticGetLogAESServerUrl(char *value, int size);

int tr069StatisticGetLogUploadInterval(void);
void tr069StatisticSetLogUploadInterval(int n);

int tr069StatisticGetLogRecordInterval(void);
void tr069StatisticSetLogRecordInterval(int n);

}

#endif //__cplusplus

#endif // _StatisticLog_H_

