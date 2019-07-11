#include "Tr069Monitor.h"

#include "Tr069FunctionCall.h"
#include "Tr069X_CTC_IPTV_Monitor.h"
#include "app_tr069_alarm.h"
#include "StatisticBase.h"
#include <stdio.h>


#ifdef TR069_MONITOR

#ifdef SQM_VERSION_C28

static int getAppTr069PortMonitorMdiMLR(char* str, unsigned int val)
{
    snprintf(str, val, "%d", app_tr069_port_Monitor_get_MdiMLR());

    return 0;
}

static int getAppTr069PortMonitorMdiDF(char* str, unsigned int val)
{
    snprintf(str, val, "%d", app_tr069_port_Monitor_get_MdiDF());

    return 0;
}


static int getAppTr069PortMonitorJitter(char* str, unsigned int val)
{
    snprintf(str, val, "%d", app_tr069_port_Monitor_get_Jitter());

    return 0;
}

#else
static int getAppTr069PortMonitorMdiMLR(char* str, unsigned int val)
{
    return 0;
}

static int getAppTr069PortMonitorMdiDF(char* str, unsigned int val)
{
    return 0;
}

static int getAppTr069PortMonitorJitter(char* str, unsigned int val)
{
    return 0;
}

#endif

static int getAppTr069PortMonitorCPURate(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_port_get_CPUrate());

    return 0;
}
static int getAppTr069PortMonitorMemRate(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_port_get_memValue());

    return 0;
}

static int getAppTr069PortMonitorAuthNumbers(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_AuthNumbers());

    return 0;
}

static int getAppTr069PortMonitorAuthFailNumbers(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_AuthFailNumbers());

    return 0;
}

static int getAppTr069PortMonitorMultiReqNumbers(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_MultiReqNumbers());

    return 0;
}

static int getAppTr069PortMonitorVodReqNumbers(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_VodReqNumbers());

    return 0;
}

static int getAppTr069PortMonitorMultiFailNumbers(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_MultiFailNumbers());

    return 0;
}

static int getAppTr069PortMonitorVodFailNumbers(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_VodFailNumbers());

    return 0;
}

static int getAppTr069PortMonitorBufUnderFlowNumbers(char* str, unsigned int val)
{
    snprintf(str, val, "%d", app_tr069_port_Monitor_get_BufUnderFlowNumbers());

    return 0;
}

static int getAppTr069PortMonitorBufOverFlowNumbers(char* str, unsigned int val)
{
    snprintf(str, val, "%d", app_tr069_port_Monitor_get_BufOverFlowNumbers());

    return 0;
}

static int getAppTr069PortMonitorResponseDelay(char* str, unsigned int val)
{
    snprintf(str, val, "%d", app_tr069_port_Monitor_get_ResponseDelay());

    return 0;
}

static int getAppTr069PortMonitorChannelSwitchDelay(char* str, unsigned int val)
{
    snprintf(str, val, "%d", app_tr069_port_Monitor_get_ChannelSwitchDelay());

    return 0;
}

static int getAppTr069PortMonitorChannelName(char* str, unsigned int val)
{
    app_tr069_port_Monitor_get_ChannelName(str, val);

    return 0;
}

static int getAppTr069PortMonitorChannelAddress(char* str, unsigned int val)
{
    app_tr069_port_Monitor_get_ChannelAddress(str, val);

    return 0;
}

static int getAppTr069PortMonitorTransmission(char* str, unsigned int val)
{
    app_tr069_port_Monitor_get_Transmission(str, val);

    return 0;
}

static int getAppTr069PortMonitorProgramStartTime(char* str, unsigned int val)
{
    app_tr069_port_Monitor_get_ProgramStartTime(str, val);

    return 0;
}

static int getAppTr069PortMonitorProgramEndTime(char* str, unsigned int val)
{
    app_tr069_port_Monitor_get_ProgramEndTime(str, val);

    return 0;
}

static int getAppTr069PortMonitorBitRate(char* str, unsigned int val)
{
    snprintf(str, val, "%d", app_tr069_port_Monitor_get_BitRate());

    return 0;
}

static int getAppTr069PortMonitorVideoQuality(char* str, unsigned int val)
{
    app_tr069_port_Monitor_get_VideoQuality(str, val);

    return 0;
}

static int getAppTr069PortMonitorChannelRequestFrequency(char* str, unsigned int val)
{
    snprintf(str, val, "%d", app_tr069_port_Monitor_get_ChannelRequestFrequency());

    return 0;
}

static int getAppTr069PortMonitorAccessSuccessNumber(char* str, unsigned int val)
{
    snprintf(str, val, "%d", app_tr069_port_Monitor_get_AccessSuccessNumber());

    return 0;
}

static int getAppTr069PortMonitorAverageAccessTime(char* str, unsigned int val)
{
    app_tr069_port_Monitor_get_AverageAccessTime(str, val);

    return 0;
}

static int getAppTr069PortMonitorWatchLong(char* str, unsigned int val)
{
    snprintf(str, val, "%d", app_tr069_port_Monitor_get_WatchLong());

    return 0;
}

static int getAppTr069PortMonitorMediaStreamBandwidth(char* str, unsigned int val)
{
    snprintf(str, val, "%d", app_tr069_port_Monitor_get_MediaStreamBandwidth());

    return 0;
}
#endif

Tr069Monitor::Tr069Monitor()
	: Tr069GroupCall("Monitor")
{
#ifdef TR069_MONITOR

    Tr069Call* MdiMLR              = new Tr069FunctionCall("MdiMLR", getAppTr069PortMonitorMdiMLR, NULL);
    Tr069Call* MdiDF               = new Tr069FunctionCall("MdiDF", getAppTr069PortMonitorMdiDF, NULL);
    Tr069Call* Jitter              = new Tr069FunctionCall("Jitter", getAppTr069PortMonitorJitter, NULL);
    Tr069Call* CPURate             = new Tr069FunctionCall("CPURate", getAppTr069PortMonitorCPURate, NULL);
    Tr069Call* MemRate             = new Tr069FunctionCall("MemRate", getAppTr069PortMonitorMemRate, NULL);
    Tr069Call* AuthNumbers         = new Tr069FunctionCall("AuthNumbers", getAppTr069PortMonitorAuthNumbers, NULL);
    Tr069Call* AuthFailNumbers     = new Tr069FunctionCall("AuthFailNumbers", getAppTr069PortMonitorAuthFailNumbers, NULL);
    Tr069Call* MultiReqNumbers     = new Tr069FunctionCall("MultiReqNumbers", getAppTr069PortMonitorMultiReqNumbers, NULL);
    Tr069Call* VodReqNumbers       = new Tr069FunctionCall("VodReqNumbers", getAppTr069PortMonitorVodReqNumbers, NULL);
    Tr069Call* MultiFailNumbers    = new Tr069FunctionCall("MultiFailNumbers", getAppTr069PortMonitorMultiFailNumbers, NULL);
    Tr069Call* VodFailNumbers      = new Tr069FunctionCall("VodFailNumbers", getAppTr069PortMonitorVodFailNumbers, NULL);
    Tr069Call* BufUnderFlowNumbers = new Tr069FunctionCall("BufUnderFlowNumbers", getAppTr069PortMonitorBufUnderFlowNumbers, NULL);
    Tr069Call* BufOverFlowNumbers  = new Tr069FunctionCall("BufOverFlowNumbers", getAppTr069PortMonitorBufOverFlowNumbers, NULL);
    Tr069Call* ResponseDelay       = new Tr069FunctionCall("ResponseDelay", getAppTr069PortMonitorResponseDelay, NULL);
    Tr069Call* ChannelSwitchDelay  = new Tr069FunctionCall("ChannelSwitchDelay", getAppTr069PortMonitorChannelSwitchDelay, NULL);
    Tr069Call* ChannelName         = new Tr069FunctionCall("ChannelName", getAppTr069PortMonitorChannelName, NULL);
    Tr069Call* ChannelAddress      = new Tr069FunctionCall("ChannelAddress", getAppTr069PortMonitorChannelAddress, NULL);
    Tr069Call* Transmission        = new Tr069FunctionCall("Transmission", getAppTr069PortMonitorTransmission, NULL);
    Tr069Call* ProgramStartTime    = new Tr069FunctionCall("ProgramStartTime", getAppTr069PortMonitorProgramStartTime, NULL);
    Tr069Call* ProgramEndTime      = new Tr069FunctionCall("ProgramEndTime", getAppTr069PortMonitorProgramEndTime, NULL);
    Tr069Call* BitRate             = new Tr069FunctionCall("BitRate", getAppTr069PortMonitorBitRate, NULL);

    Tr069Call* VideoQuality              = new Tr069FunctionCall("VideoQuality", getAppTr069PortMonitorVideoQuality, NULL);
    Tr069Call* ChannelRequestFrequency   = new Tr069FunctionCall("ChannelRequestFrequency", getAppTr069PortMonitorChannelRequestFrequency, NULL);
    Tr069Call* AccessSuccessNumber       = new Tr069FunctionCall("AccessSuccessNumber", getAppTr069PortMonitorAccessSuccessNumber, NULL);
    Tr069Call* AverageAccessTime         = new Tr069FunctionCall("AverageAccessTime", getAppTr069PortMonitorAverageAccessTime, NULL);
    Tr069Call* WatchLong                 = new Tr069FunctionCall("WatchLong", getAppTr069PortMonitorWatchLong, NULL);
    Tr069Call* MediaStreamBandwidth      = new Tr069FunctionCall("MediaStreamBandwidth", getAppTr069PortMonitorMediaStreamBandwidth, NULL);	

    regist(MdiMLR->name(), MdiMLR);
    regist(MdiDF->name(), MdiDF);
    regist(Jitter->name(), Jitter);
    regist(CPURate->name(), CPURate);
    regist(MemRate->name(), MemRate);
    regist(AuthNumbers->name(), AuthNumbers);
    regist(AuthFailNumbers->name(), AuthFailNumbers);
    regist(MultiReqNumbers->name(), MultiReqNumbers);
    regist(VodReqNumbers->name(), VodReqNumbers);
    regist(MultiFailNumbers->name(), MultiFailNumbers);
    regist(VodFailNumbers->name(), VodFailNumbers);
    regist(BufUnderFlowNumbers->name(), BufUnderFlowNumbers);
    regist(BufOverFlowNumbers->name(), BufOverFlowNumbers);
    regist(ResponseDelay->name(), ResponseDelay);
    regist(ChannelSwitchDelay->name(), ChannelSwitchDelay);
    regist(ChannelName->name(), ChannelName);
    regist(ChannelAddress->name(), ChannelAddress);
    regist(Transmission->name(), Transmission);
    regist(ProgramStartTime->name(), ProgramStartTime);
    regist(ProgramEndTime->name(), ProgramEndTime);
    regist(BitRate->name(), BitRate);

    regist(VideoQuality->name(), VideoQuality);
    regist(ChannelRequestFrequency->name(), ChannelRequestFrequency);
    regist(AccessSuccessNumber->name(), AccessSuccessNumber);
    regist(AverageAccessTime->name(), AverageAccessTime);
    regist(WatchLong->name(), WatchLong);
    regist(MediaStreamBandwidth->name(), MediaStreamBandwidth);
#endif

}

Tr069Monitor::~Tr069Monitor()
{
}
