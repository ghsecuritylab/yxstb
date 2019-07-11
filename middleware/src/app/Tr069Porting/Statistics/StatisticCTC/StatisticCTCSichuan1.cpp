
#include "StatisticCTCSichuan.h"
#include "StatisticCTCSichuan1.h"

#include "TR069Assertions.h"

#include "sqm_port.h"
#include "app_tr069_alarm.h"

#include "mid/mid_ftp.h"
#include "independs/ind_cfg.h"

#include<string.h>
#include<stdlib.h>

#define STATISTIC_FILE_SIZE_16K (16 * 1024)

class StatisticCTCSichuan1 : public StatisticCTCSichuan {
public:

    int m_CPURateEnable;
    int m_MemRateEnable;
    int m_JitterEnable;
    int m_MdiMLREnable;
    int m_MdiDFEnable;

public:
    StatisticCTCSichuan1(void);
    ~StatisticCTCSichuan1(void);

    void AppendStatisticBody(void);

    void RegistRecord(void);
    void RegistUpload(void);
    void UnRegistRecord(void);
    void UnRegistUpload(void);

    int GetValue(char *name, char *str, unsigned int size);
    int SetValue(char *name, char *str, unsigned int x);
};

static StatisticCTCSichuan1 *gStatistic = NULL;

StatisticCTCSichuan1::StatisticCTCSichuan1(void) : 
    m_CPURateEnable(0),
    m_MemRateEnable(0),
    m_JitterEnable(0),
    m_MdiMLREnable(0),
    m_MdiDFEnable(0)
{
    ind_cfg_inset_object(m_cfgTree, (char*)"statistic1");

    ind_cfg_inset_int(m_cfgTree,    (char*)"statistic1.Enable",               &m_Enable);
    ind_cfg_inset_string(m_cfgTree, (char*)"statistic1.LogServerUrl",          m_LogServerUrl, 512);	
    ind_cfg_inset_int(m_cfgTree,    (char*)"statistic1.LogUploadInterval",    &m_LogUploadInterval);
    ind_cfg_inset_int(m_cfgTree,    (char*)"statistic1.LogRecordInterval",    &m_LogRecordInterval);
    ind_cfg_inset_int(m_cfgTree,    (char*)"statistic1.CPURateEnable",        &m_CPURateEnable);
    ind_cfg_inset_int(m_cfgTree,    (char*)"statistic1.MemRateEnable",        &m_MemRateEnable);
    ind_cfg_inset_int(m_cfgTree,    (char*)"statistic1.JitterEnable",         &m_JitterEnable);	
    ind_cfg_inset_int(m_cfgTree,    (char*)"statistic1.MdiMLREnable",         &m_MdiMLREnable);
    ind_cfg_inset_int(m_cfgTree,    (char*)"statistic1.MdiDFEnable",          &m_MdiDFEnable);

    statisticConfigRead((char*)"statistic1");

    m_typeChar = 'P';
    m_bodyBuf = (char *)malloc(STATISTIC_FILE_SIZE_16K + 1);
    m_bodyBuf[STATISTIC_FILE_SIZE_16K] = 0;

    StatisticRefresh( );
}

StatisticCTCSichuan1::~StatisticCTCSichuan1(void)
{
}

void StatisticCTCSichuan1::AppendStatisticBody(void)
{
    char buf[256];

    int cpuRate, memValue, jitter, mdiMLR, mdiDF;

    sqm_port_getdata(1);

    cpuRate = tr069_port_get_CPUrate();
    memValue = tr069_port_get_memValue();
    jitter = getSqmDataJitter();
    mdiMLR = getSqmDataMdiMLR();
    mdiDF = getSqmDataMdiDF();

    if(m_CPURateEnable) {
        changeDataFmt(cpuRate, buf);
        m_bodyLen += snprintf(m_bodyBuf + m_bodyLen, STATISTIC_FILE_SIZE_16K - m_bodyLen, "CPURate,%s\n",  buf);
    }

    if(m_MemRateEnable) {
        changeDataFmt(memValue, buf);
        m_bodyLen += snprintf(m_bodyBuf + m_bodyLen, STATISTIC_FILE_SIZE_16K - m_bodyLen, "MemRate,%s\n", buf);
    }

    if(m_JitterEnable) {
        changeDataFmt(jitter, buf);
        m_bodyLen += snprintf(m_bodyBuf + m_bodyLen, STATISTIC_FILE_SIZE_16K - m_bodyLen, "Jitter,%s\n", buf);
    }

    if(m_MdiMLREnable) {
        changeDataFmt(mdiMLR, buf);
        m_bodyLen += snprintf(m_bodyBuf + m_bodyLen, STATISTIC_FILE_SIZE_16K - m_bodyLen, "MdiMLR,%s\n", buf);
    }

    if(m_MdiDFEnable) {
        changeDataFmt(mdiDF, buf);
        m_bodyLen += snprintf(m_bodyBuf + m_bodyLen, STATISTIC_FILE_SIZE_16K - m_bodyLen, "MdiDF,%s\n", buf);
    }

    m_bodyLen += snprintf(m_bodyBuf + m_bodyLen, STATISTIC_FILE_SIZE_16K - m_bodyLen, "********\n");
}

static void configurationSave(int arg)
{
    gStatistic->statisticConfigWrite((char*)"statistic1");
}

static void statisticRecordFile(int arg)
{
    LogTr069Debug("\n");
    gStatistic->StatisticRecord( );
}

static void statisticUploadFile(int arg)
{
    LogTr069Debug("\n");
    gStatistic->StatisticUpload( );
}

void StatisticCTCSichuan1::RegistRecord(void)
{
    timerTaskRegist(m_LogRecordInterval, statisticRecordFile, 0);
}

void StatisticCTCSichuan1::RegistUpload(void)
{
    timerTaskRegist(m_LogUploadInterval, statisticUploadFile, 0);
}

void StatisticCTCSichuan1::UnRegistRecord(void)
{
    timerTaskUnRegist(statisticRecordFile, 0);
}

void StatisticCTCSichuan1::UnRegistUpload(void)
{
    timerTaskUnRegist(statisticUploadFile, 0);
}

int StatisticCTCSichuan1::GetValue(char *name, char *str, unsigned int size)
{
    int ret = 0;

    pthread_mutex_lock(&m_prmMutex);

    if(!strcmp(name, "Enable")){
	 snprintf(str, size, "%d", m_Enable);
    } else if (!strcmp(name, "LogServerUrl")) {
        snprintf(str, size, "%s", m_LogServerUrl);
    } else if (!strcmp(name, "LogUploadInterval")) {
        snprintf(str, size, "%d", m_LogUploadInterval);
    } else if (!strcmp(name, "LogRecordInterval")) {
        snprintf(str, size, "%d", m_LogRecordInterval);
    } else if (!strcmp(name, "CPURateEnable")) {
        snprintf(str, size, "%d", m_CPURateEnable);
    } else if (!strcmp(name, "MemRateEnable")) {
        snprintf(str, size, "%d", m_MemRateEnable);
    } else if (!strcmp(name, "JitterEnable")) {
        snprintf(str, size, "%d", m_JitterEnable);
    } else if (!strcmp(name, "MdiMLREnable")) {
        snprintf(str, size, "%d", m_MdiMLREnable);
    } else if (!strcmp(name, "MdiDFEnable")) {
        snprintf(str, size, "%d", m_MdiDFEnable);
    } else {
        LogTr069Error("name = %s!\n", name);
        ret = -1;
    }

    pthread_mutex_unlock(&m_prmMutex);

    return ret;
}

int StatisticCTCSichuan1::SetValue(char *name, char *str, unsigned int x)
{
    int val = 0;

    LogTr069Debug("name = %s, value = %s\n", name, str);

    pthread_mutex_lock(&m_prmMutex);

    if(!strcmp(name, "LogServerUrl")) {
	    strcpy(m_LogServerUrl, str);
	} else {
	    val = atoi(str);
	    if (val >= 0) {
            if (!strcmp(name, "Enable")) {
                m_Enable = val;
                StatisticRefresh( );
            } else if (!strcmp(name, "LogUploadInterval")) {
                m_LogUploadInterval = val;
                StatisticRefresh( );
            } else if(!strcmp(name, "LogRecordInterval"))
                m_LogRecordInterval = val;
        	else if(!strcmp(name, "CPURateEnable"))
                m_CPURateEnable = val;
        	else if(!strcmp(name, "MemRateEnable"))
                m_MemRateEnable = val;
        	else if(!strcmp(name, "JitterEnable"))
                m_JitterEnable = val;
        	else if(!strcmp(name, "MdiMLREnable"))
                m_MdiMLREnable = val;
        	else if(!strcmp(name, "MdiDFEnable"))
                m_MdiDFEnable = val;
            else
                val = -1;
	    }
	}

    pthread_mutex_unlock(&m_prmMutex);

    if (val >= 0) {
        timerTaskRegist(1, configurationSave, 0);
    } else {
        LogTr069Error("name = %s, value = %d!\n", name, str);
    }

	return 0;
}


void statisticPrimaryInit(void)
{
    if (gStatistic)
        return;
    gStatistic = new StatisticCTCSichuan1( );
}

int statisticPrimaryGet(char *name, char *str, unsigned int size)
{
    if (!gStatistic)
        return -1;
    return gStatistic->GetValue(name, str, size);
}

int statisticPrimarySet(char *name, char *str, unsigned int x)
{
    if (!gStatistic)
        return -1;
    return gStatistic->SetValue(name, str, x);
}