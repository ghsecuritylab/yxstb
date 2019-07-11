
#include "StatisticCTCSichuan.h"
#include "StatisticCTCSichuan2.h"

//#include "ProgramInfo.h"
#include "TR069Assertions.h"

#include "sqm_port.h"
#include "app_tr069_alarm.h"
#include "tr069_port_ctc.h"

#include "mid_stream.h"
#include "mid/mid_ftp.h"
#include "independs/ind_cfg.h"

#include<string.h>
#include<stdlib.h>

#define STATISTIC_FILE_SIZE_64K (64 * 1024)

class StatisticCTCSichuan2 : public StatisticCTCSichuan {
public:
    int m_BufUnderFlowNumbersEnable;
    int m_BufOverFlowNumbersEnable;
    int m_AuthNumbersEnable;
    int m_AuthFailNumbersEnable;
    int m_MultiReqNumbersEnable;
    int m_MultiFailNumbersEnable;
    int m_VodReqNumbersEnable;
    int m_VodFailNumbersEnable;
    int m_ResponseDelayEnable;
    int m_ChannelSwitchDelayEnable;
    int m_AbnormalIntervalEnable;
    int m_AuthSuccRateEnable;
    int m_PlayDurationEnable;
    int m_BadDurationEnable;
    int m_AvailabilityEnable;
    int m_ChannelNameEnable;
    int m_ChannelAddressEnable;
    int m_TransmissionEnable;
    int m_ProgramStartTimeEnable;
    int m_ProgramEndTimeEnable;
    int m_JitterEnable;
    int m_MdiMLREnable;
    int m_MdiDFEnable;
    int m_BitRateEnable;
    int m_VideoQualityEnable;
    int m_ChannelRequestFrequencyEnable;
    int m_AccessSuccessNumberEnable;
    int m_AverageAccessTimeEnable;
    int m_WatchLongEnable;
    int m_MediaStreamBandwidthEnable;

public:
    StatisticCTCSichuan2(void);
    ~StatisticCTCSichuan2(void);

    void AppendStatisticBody(void);

    void RegistRecord(void);
    void RegistUpload(void);
    void UnRegistRecord(void);
    void UnRegistUpload(void);

    int GetValue(char *name, char *str, unsigned int size);
    int SetValue(char *name, char *str, unsigned int x);
};

static StatisticCTCSichuan2 *gStatistic = NULL;

StatisticCTCSichuan2::StatisticCTCSichuan2(void) : 
    m_BufUnderFlowNumbersEnable(0),
    m_BufOverFlowNumbersEnable(0),
    m_AuthNumbersEnable(0),
    m_AuthFailNumbersEnable(0),
    m_MultiReqNumbersEnable(0),
    m_MultiFailNumbersEnable(0),
    m_VodReqNumbersEnable(0),
    m_VodFailNumbersEnable(0),
    m_ResponseDelayEnable(0),
    m_ChannelSwitchDelayEnable(0),
    m_AbnormalIntervalEnable(0),
    m_AuthSuccRateEnable(0),
    m_PlayDurationEnable(0),
    m_BadDurationEnable(0),
    m_AvailabilityEnable(0),
    m_ChannelNameEnable(0),
    m_ChannelAddressEnable(0),
    m_TransmissionEnable(0),
    m_ProgramStartTimeEnable(0),
    m_ProgramEndTimeEnable(0),
    m_JitterEnable(0),
    m_MdiMLREnable(0),
    m_MdiDFEnable(0),
    m_BitRateEnable(0),
    m_VideoQualityEnable(0),
    m_ChannelRequestFrequencyEnable(0),
    m_AccessSuccessNumberEnable(0),
    m_AverageAccessTimeEnable(0),
    m_WatchLongEnable(0),
    m_MediaStreamBandwidthEnable(0)
{
    ind_cfg_inset_object(m_cfgTree, (char*)"statistic2");

    ind_cfg_inset_int(m_cfgTree,    (char*)"statistic2.Enable",             &m_Enable);
    ind_cfg_inset_string(m_cfgTree, (char*)"statistic2.LogServerUrl",        m_LogServerUrl, 512);	
    ind_cfg_inset_int(m_cfgTree,    (char*)"statistic2.LogUploadInterval",  &m_LogUploadInterval);
    ind_cfg_inset_int(m_cfgTree,    (char*)"statistic2.LogRecordInterval",  &m_LogRecordInterval);

    ind_cfg_inset_int(m_cfgTree, (char*)"statistic2.BufUnderFlowNumbersEnable",    &m_BufUnderFlowNumbersEnable);
    ind_cfg_inset_int(m_cfgTree, (char*)"statistic2.BufOverFlowNumbersEnable",     &m_BufOverFlowNumbersEnable);
    ind_cfg_inset_int(m_cfgTree, (char*)"statistic2.AuthNumbersEnable",            &m_AuthNumbersEnable);	
    ind_cfg_inset_int(m_cfgTree, (char*)"statistic2.AuthFailNumbersEnable",        &m_AuthFailNumbersEnable);
    ind_cfg_inset_int(m_cfgTree, (char*)"statistic2.MultiReqNumbersEnable",        &m_MultiReqNumbersEnable);
    ind_cfg_inset_int(m_cfgTree, (char*)"statistic2.MultiFailNumbersEnable",       &m_MultiFailNumbersEnable);

    ind_cfg_inset_int(m_cfgTree, (char*)"statistic2.VodReqNumbersEnable",          &m_VodReqNumbersEnable);
    ind_cfg_inset_int(m_cfgTree, (char*)"statistic2.VodFailNumbersEnable",         &m_VodFailNumbersEnable);	
    ind_cfg_inset_int(m_cfgTree, (char*)"statistic2.ResponseDelayEnable",          &m_ResponseDelayEnable);
    ind_cfg_inset_int(m_cfgTree, (char*)"statistic2.ChannelSwitchDelayEnable",     &m_ChannelSwitchDelayEnable);
    ind_cfg_inset_int(m_cfgTree, (char*)"statistic2.AbnormalIntervalEnable",       &m_AbnormalIntervalEnable);
    ind_cfg_inset_int(m_cfgTree, (char*)"statistic2.AuthSuccRateEnable",           &m_AuthSuccRateEnable);
    ind_cfg_inset_int(m_cfgTree, (char*)"statistic2.PlayDurationEnable",           &m_PlayDurationEnable);	
    ind_cfg_inset_int(m_cfgTree, (char*)"statistic2.BadDurationEnable",            &m_BadDurationEnable);
    ind_cfg_inset_int(m_cfgTree, (char*)"statistic2.AvailabilityEnable",           &m_AvailabilityEnable);
    ind_cfg_inset_int(m_cfgTree, (char*)"statistic2.ChannelNameEnable",            &m_ChannelNameEnable);

    ind_cfg_inset_int(m_cfgTree, (char*)"statistic2.ChannelAddressEnable",         &m_ChannelAddressEnable);
    ind_cfg_inset_int(m_cfgTree, (char*)"statistic2.TransmissionEnable",           &m_TransmissionEnable);	
    ind_cfg_inset_int(m_cfgTree, (char*)"statistic2.ProgramStartTimeEnable",       &m_ProgramStartTimeEnable);
    ind_cfg_inset_int(m_cfgTree, (char*)"statistic2.ProgramEndTimeEnable",         &m_ProgramEndTimeEnable);
    ind_cfg_inset_int(m_cfgTree, (char*)"statistic2.JitterEnable",                 &m_JitterEnable);
    ind_cfg_inset_int(m_cfgTree, (char*)"statistic2.MdiMLREnable",                 &m_MdiMLREnable);
    ind_cfg_inset_int(m_cfgTree, (char*)"statistic2.MdiDFEnable",                  &m_MdiDFEnable);	
    ind_cfg_inset_int(m_cfgTree, (char*)"statistic2.BitRateEnable",                &m_BitRateEnable);
    ind_cfg_inset_int(m_cfgTree, (char*)"statistic2.VideoQualityEnable",           &m_VideoQualityEnable);
    ind_cfg_inset_int(m_cfgTree, (char*)"statistic2.ChannelRequestFrequencyEnable",&m_ChannelRequestFrequencyEnable);
    ind_cfg_inset_int(m_cfgTree, (char*)"statistic2.AccessSuccessNumberEnable",    &m_AccessSuccessNumberEnable);
    ind_cfg_inset_int(m_cfgTree, (char*)"statistic2.AverageAccessTimeEnable",      &m_AverageAccessTimeEnable);
    ind_cfg_inset_int(m_cfgTree, (char*)"statistic2.WatchLongEnable",              &m_WatchLongEnable);
    ind_cfg_inset_int(m_cfgTree, (char*)"statistic2.MediaStreamBandwidthEnable",   &m_MediaStreamBandwidthEnable);

    statisticConfigRead((char*)"statistic2");

    m_typeChar = 'S';
    m_bodyBuf = (char *)malloc(STATISTIC_FILE_SIZE_64K + 1);
    m_bodyBuf[STATISTIC_FILE_SIZE_64K] = 0;

    StatisticRefresh( );
}

StatisticCTCSichuan2::~StatisticCTCSichuan2(void)
{
}

void StatisticCTCSichuan2::AppendStatisticBody(void)
{
    char buf[256];
    int val;

    if(m_BufUnderFlowNumbersEnable){
        val = app_tr069_port_Monitor_get_BufUnderFlowNumbers();
        changeDataFmt(val, buf);
        m_bodyLen += snprintf(m_bodyBuf + m_bodyLen, STATISTIC_FILE_SIZE_64K - m_bodyLen, "BufUnderFlowNumbers,%s\n", buf);
    }

    if(m_BufOverFlowNumbersEnable){
        val = app_tr069_port_Monitor_get_BufOverFlowNumbers();
        changeDataFmt(val, buf);
        m_bodyLen += snprintf(m_bodyBuf + m_bodyLen, STATISTIC_FILE_SIZE_64K - m_bodyLen, "BufOverFlowNumbers,%s\n", buf);
    }

    if(m_AuthNumbersEnable){
        val = app_tr069_port_Monitor_get_AuthNumbers();
        changeDataFmt(val, buf);
        m_bodyLen += snprintf(m_bodyBuf + m_bodyLen, STATISTIC_FILE_SIZE_64K - m_bodyLen, "AuthNumbers,%s\n", buf);
    }

    if(m_AuthFailNumbersEnable){
        val = app_tr069_port_Monitor_get_AuthFailNumbers();
        changeDataFmt(val, buf);
       m_bodyLen += snprintf(m_bodyBuf + m_bodyLen, STATISTIC_FILE_SIZE_64K - m_bodyLen, "AuthFailNumbers,%s\n", buf);
    }

    if(m_MultiReqNumbersEnable){;
        val = app_tr069_port_Monitor_get_MultiReqNumbers();
        changeDataFmt(val, buf);
        m_bodyLen += snprintf(m_bodyBuf + m_bodyLen, STATISTIC_FILE_SIZE_64K - m_bodyLen, "MultiReqNumbers,%s\n", buf);
    }

    if(m_VodReqNumbersEnable){
        val = app_tr069_port_Monitor_get_VodReqNumbers();
        changeDataFmt(val, buf);
        m_bodyLen += snprintf(m_bodyBuf + m_bodyLen, STATISTIC_FILE_SIZE_64K - m_bodyLen, "VodReqNumbers,%s\n", buf);
    }

    if(m_MultiFailNumbersEnable){
        val = app_tr069_port_Monitor_get_MultiFailNumbers();
        changeDataFmt(val, buf);
        m_bodyLen += snprintf(m_bodyBuf + m_bodyLen, STATISTIC_FILE_SIZE_64K - m_bodyLen, "MultiFailNumbers,%s\n", buf);
    }

    if(m_VodFailNumbersEnable){
        val = app_tr069_port_Monitor_get_VodFailNumbers();
        changeDataFmt(val, buf);
        m_bodyLen += snprintf(m_bodyBuf + m_bodyLen, STATISTIC_FILE_SIZE_64K - m_bodyLen, "VodFailNumbers,%s\n", buf);
    }

    if(m_ResponseDelayEnable){
        val = app_tr069_port_Monitor_get_ResponseDelay();
        changeDataFmt(val, buf);
        m_bodyLen += snprintf(m_bodyBuf + m_bodyLen, STATISTIC_FILE_SIZE_64K - m_bodyLen, "ResponseDelay,%s\n", buf);
    }

    if(m_ChannelSwitchDelayEnable){
        val = app_tr069_port_Monitor_get_ChannelSwitchDelay();
        changeDataFmt(val, buf);
        m_bodyLen += snprintf(m_bodyBuf + m_bodyLen, STATISTIC_FILE_SIZE_64K - m_bodyLen, "ChannelSwitchDelay,%s\n", buf);
    }

    if(m_AbnormalIntervalEnable){
        val =  mid_stream_getInt((char*)"AbnormalInterval", 0);
        changeDataFmt(val, buf);
        m_bodyLen += snprintf(m_bodyBuf + m_bodyLen, STATISTIC_FILE_SIZE_64K - m_bodyLen, "AbnormalInterval,%s\n", buf);
    }

    if(m_AuthSuccRateEnable){
            int total = app_tr069_port_Monitor_get_AuthNumbers();
            int fail = app_tr069_port_Monitor_get_AuthFailNumbers();
            if (total <= 0) {
                val = 0;
            } else {
                val = (int)((total - fail) * 100.0f / (float)total);
            }
        changeDataFmt(val, buf);
        m_bodyLen += snprintf(m_bodyBuf + m_bodyLen, STATISTIC_FILE_SIZE_64K - m_bodyLen, "AuthSuccRate,%s\n", buf);
    }
//以下几个参数需要用统计值去sqmpro获取数据
    sqm_port_getdata(1);
    if(m_PlayDurationEnable){
        val =  getSqmDataPlayduration();
        changeDataFmt(val, buf);
        m_bodyLen += snprintf(m_bodyBuf + m_bodyLen, STATISTIC_FILE_SIZE_64K - m_bodyLen, "PlayDuration,%s\n", buf);
    }

    if(m_BadDurationEnable)	{
        val =getSqmDataBadDuration();
        changeDataFmt(val, buf);
        m_bodyLen += snprintf(m_bodyBuf + m_bodyLen, STATISTIC_FILE_SIZE_64K - m_bodyLen, "BadDuration,%s\n", buf);
    }

    if(m_AvailabilityEnable){
        val = getSqmDataAvailability();
        changeDataFmt(val, buf);
        m_bodyLen += snprintf(m_bodyBuf + m_bodyLen, STATISTIC_FILE_SIZE_64K - m_bodyLen, "Availability,%s\n", buf);
    }
    m_bodyLen += snprintf(m_bodyBuf + m_bodyLen, STATISTIC_FILE_SIZE_64K - m_bodyLen, "********\n");

    //m_bodyLen += addProgramInfo(m_bodyBuf + m_bodyLen, STATISTIC_FILE_SIZE_64K - m_bodyLen);
}

static void configurationSave(int arg)
{
    gStatistic->statisticConfigWrite((char*)"statistic2");
}

static void statisticRecordFile(int arg)
{
    gStatistic->StatisticRecord( );
}

static void statisticUploadFile(int arg)
{
    gStatistic->StatisticUpload( );
}

void StatisticCTCSichuan2::RegistRecord(void)
{
    timerTaskRegist(m_LogRecordInterval, statisticRecordFile, 0);
}

void StatisticCTCSichuan2::RegistUpload(void)
{
    timerTaskRegist(m_LogUploadInterval, statisticUploadFile, 0);
}

void StatisticCTCSichuan2::UnRegistRecord(void)
{
    timerTaskUnRegist(statisticRecordFile, 0);
}

void StatisticCTCSichuan2::UnRegistUpload(void)
{
    timerTaskUnRegist(statisticUploadFile, 0);
}

int StatisticCTCSichuan2::GetValue(char *name, char *str, unsigned int size)
{
    int ret = 0;

    pthread_mutex_lock(&m_prmMutex);

    if (!strcmp(name, "Enable"))
	    snprintf(str, size, "%d", m_Enable);
    else if (!strcmp(name, "LogServerUrl"))
        snprintf(str, size, "%s", m_LogServerUrl);
    else if (!strcmp(name, "LogUploadInterval"))
        snprintf(str, size, "%d", m_LogUploadInterval);
    else if (!strcmp(name, "LogRecordInterval"))
        snprintf(str, size, "%d", m_LogRecordInterval);
    else if (!strcmp(name, "BufUnderFlowNumbersEnable"))
        snprintf(str, size, "%d", m_BufUnderFlowNumbersEnable);
    else if (!strcmp(name, "BufOverFlowNumbersEnable"))
        snprintf(str, size, "%d", m_BufOverFlowNumbersEnable);
    else if (!strcmp(name, "AuthNumbersEnable"))
        snprintf(str, size, "%d", m_AuthNumbersEnable);
    else if (!strcmp(name, "AuthFailNumbersEnable"))
        snprintf(str, size, "%d", m_AuthFailNumbersEnable);
    else if (!strcmp(name, "MultiReqNumbersEnable"))
        snprintf(str, size, "%d", m_MultiReqNumbersEnable);
    else if (!strcmp(name, "MultiFailNumbersEnable"))
        snprintf(str, size, "%d", m_MultiFailNumbersEnable);
    else if (!strcmp(name, "VodReqNumbersEnable"))
        snprintf(str, size, "%d", m_VodReqNumbersEnable);
    else if (!strcmp(name, "VodFailNumbersEnable"))
        snprintf(str, size, "%d", m_VodFailNumbersEnable);
    else if (!strcmp(name, "ResponseDelayEnable"))
        snprintf(str, size, "%d", m_ResponseDelayEnable);
    else if (!strcmp(name, "ChannelSwitchDelayEnable"))
        snprintf(str, size, "%d", m_ChannelSwitchDelayEnable);
    else if (!strcmp(name, "AbnormalIntervalEnable"))
        snprintf(str, size, "%d", m_AbnormalIntervalEnable);
    else if (!strcmp(name, "AuthSuccRateEnable"))
        snprintf(str, size, "%d", m_AuthSuccRateEnable);
    else if (!strcmp(name, "PlayDurationEnable"))
        snprintf(str, size, "%d", m_PlayDurationEnable);
    else if (!strcmp(name, "BadDurationEnable"))
        snprintf(str, size, "%d", m_BadDurationEnable);
    else if (!strcmp(name, "AvailabilityEnable"))
        snprintf(str, size, "%d", m_AvailabilityEnable);
    else if (!strcmp(name, "ChannelNameEnable"))
        snprintf(str, size, "%d", m_ChannelNameEnable);
    else if (!strcmp(name, "ChannelAddressEnable"))
        snprintf(str, size, "%d", m_ChannelAddressEnable);
    else if (!strcmp(name, "TransmissionEnable"))
        snprintf(str, size, "%d", m_TransmissionEnable);
    else if (!strcmp(name, "ProgramStartTimeEnable"))
        snprintf(str, size, "%d", m_ProgramStartTimeEnable);
    else if (!strcmp(name, "ProgramEndTimeEnable"))
        snprintf(str, size, "%d", m_ProgramEndTimeEnable);
    else if (!strcmp(name, "JitterEnable"))
        snprintf(str, size, "%d", m_JitterEnable);
    else if (!strcmp(name, "MdiMLREnable"))
        snprintf(str, size, "%d", m_MdiMLREnable);
    else if (!strcmp(name, "MdiDFEnable"))
        snprintf(str, size, "%d", m_MdiDFEnable);
    else if (!strcmp(name, "BitRateEnable"))
        snprintf(str, size, "%d", m_BitRateEnable);
    else if (!strcmp(name, "VideoQualityEnable"))
        snprintf(str, size, "%d", m_VideoQualityEnable);
    else if (!strcmp(name, "ChannelRequestFrequencyEnable"))
        snprintf(str, size, "%d", m_ChannelRequestFrequencyEnable);
    else if (!strcmp(name, "AccessSuccessNumberEnable"))
        snprintf(str, size, "%d", m_AccessSuccessNumberEnable);
    else if (!strcmp(name, "AverageAccessTimeEnable"))
        snprintf(str, size, "%d", m_AverageAccessTimeEnable);
    else if (!strcmp(name, "WatchLongEnable"))
        snprintf(str, size, "%d", m_WatchLongEnable);
    else if (!strcmp(name, "MediaStreamBandwidthEnable"))
        snprintf(str, size, "%d", m_MediaStreamBandwidthEnable);
    else {
        LogTr069Error("name = %s!\n", name);
        ret = -1;
    }

    pthread_mutex_unlock(&m_prmMutex);

    return ret;
}

int StatisticCTCSichuan2::SetValue(char *name, char *str, unsigned int x)
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
            } else if(!strcmp(name, "LogRecordInterval")) {
                m_LogRecordInterval = val;
                StatisticRefresh( );
            } else if (!strcmp(name, "BufUnderFlowNumbersEnable"))
                m_BufUnderFlowNumbersEnable =val;
            else if (!strcmp(name, "BufOverFlowNumbersEnable"))
                m_BufOverFlowNumbersEnable =val;
            else if (!strcmp(name, "AuthNumbersEnable"))
                m_AuthNumbersEnable =val;
            else if (!strcmp(name, "AuthFailNumbersEnable"))
                m_AuthFailNumbersEnable =val;
            else if (!strcmp(name, "MultiReqNumbersEnable"))
                m_MultiReqNumbersEnable =val;
            else if (!strcmp(name, "MultiFailNumbersEnable"))
                m_MultiFailNumbersEnable =val;
            else if (!strcmp(name, "VodReqNumbersEnable"))
                m_VodReqNumbersEnable =val;
            else if (!strcmp(name, "VodFailNumbersEnable"))
                m_VodFailNumbersEnable =val;
            else if (!strcmp(name, "ResponseDelayEnable"))
                m_ResponseDelayEnable =val;
            else if (!strcmp(name, "ChannelSwitchDelayEnable"))
                m_ChannelSwitchDelayEnable =val;
            else if (!strcmp(name, "AbnormalIntervalEnable"))
                m_AbnormalIntervalEnable =val;
            else if (!strcmp(name, "AuthSuccRateEnable"))
                m_AuthSuccRateEnable =val;
            else if (!strcmp(name, "PlayDurationEnable"))
                m_PlayDurationEnable =val;
            else if (!strcmp(name, "BadDurationEnable"))
                m_BadDurationEnable =val;
            else if (!strcmp(name, "AvailabilityEnable"))
                m_AvailabilityEnable =val;
            else if (!strcmp(name, "ChannelNameEnable"))
                m_ChannelNameEnable =val;
            else if (!strcmp(name, "ChannelAddressEnable"))
                m_ChannelAddressEnable =val;
            else if (!strcmp(name, "TransmissionEnable"))
                m_TransmissionEnable =val;
            else if (!strcmp(name, "ProgramStartTimeEnable"))
                m_ProgramStartTimeEnable =val;
            else if (!strcmp(name, "ProgramEndTimeEnable"))
                m_ProgramEndTimeEnable =val;
            else if (!strcmp(name, "JitterEnable"))
                m_JitterEnable =val;
            else if (!strcmp(name, "MdiMLREnable"))
                m_MdiMLREnable =val;
            else if (!strcmp(name, "MdiDFEnable"))
                m_MdiDFEnable =val;
            else if (!strcmp(name, "BitRateEnable"))
                m_BitRateEnable =val;
            else if (!strcmp(name, "VideoQualityEnable"))
                m_VideoQualityEnable =val;
            else if (!strcmp(name, "ChannelRequestFrequencyEnable"))
                m_ChannelRequestFrequencyEnable =val;
            else if (!strcmp(name, "AccessSuccessNumberEnable"))
                m_AccessSuccessNumberEnable =val;
            else if (!strcmp(name, "AverageAccessTimeEnable"))
                m_AverageAccessTimeEnable =val;
            else if (!strcmp(name, "WatchLongEnable"))
                m_WatchLongEnable =val;
            else if (!strcmp(name, "MediaStreamBandwidthEnable"))
                m_MediaStreamBandwidthEnable =val;
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


void statisticSecondaryInit(void)
{
    if (gStatistic)
        return;
    gStatistic = new StatisticCTCSichuan2( );
}

int statisticSecondaryGet(char *name, char *str, unsigned int size)
{
    if (!gStatistic)
        return -1;
    return gStatistic->GetValue(name, str, size);
}

int statisticSecondarySet(char *name, char *str, unsigned int x)
{
    if (!gStatistic)
        return -1;
    return gStatistic->SetValue(name, str, x);
}