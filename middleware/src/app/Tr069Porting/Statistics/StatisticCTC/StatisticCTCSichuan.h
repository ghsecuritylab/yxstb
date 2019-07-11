#ifndef StatisticCTCSichuan_h
#define StatisticCTCSichuan_h

#include "TimerTask.h"
#include "independs/ind_cfg.h"

class StatisticCTCSichuan : public TimerTask {
public:

    pthread_mutex_t m_prmMutex;

    int m_Enable;
    char m_LogServerUrl[512];
    int m_LogUploadInterval;
    int m_LogRecordInterval;

    int m_LogUploadMultiple;
    int m_LogUploadCounter;

    CfgTree_t m_cfgTree;

    char* m_headBuf;
    int   m_headLen;
    char* m_bodyBuf;
    int   m_bodyLen;
    char* m_tailBuf;
    int   m_tailLen;

    char m_fileName[512 + 64];

    char m_typeChar;
public:

    StatisticCTCSichuan(void);
    ~StatisticCTCSichuan(void);

    void statisticConfigRead(char *rootname);
    void statisticConfigWrite(char *rootname);

    int statisticTime2str(char *str, int size, const char *prefix, const char *suffix, unsigned int sec);
    void CreateUploadHead(void);
    void CreateUploadName(void);

    virtual void AppendStatisticBody(void) = 0;
    virtual void RegistRecord(void) = 0;
    virtual void RegistUpload(void) = 0;
    virtual void UnRegistRecord(void) = 0;
    virtual void UnRegistUpload(void) = 0;

    void StatisticRecord(void);
    void StatisticUpload(void);
    void StatisticRefresh(void);

    int UploadFile(void);
};

void changeDataFmt(int a, char *fmt);

#endif //StatisticCTCSichuan_h
