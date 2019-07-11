#include "Tr069StatisticConfiguration2.h"
#include "StatisticCTC/StatisticCTCSichuan2.h"
#include "Tr069FunctionCall.h"
#include "StatisticBase.h"

static int getTr069SecondaryLogEnable(char* value, unsigned int size)
{
    return statisticSecondaryGet((char*)"Enable", value, size);
}

static int setTr069SecondaryLogEnable(char* value, unsigned int size)
{
    return statisticSecondarySet((char*)"Enable", value, size);
}

static int getTr069SecondaryLogServerUrl(char* value, unsigned int size)
{
    return statisticSecondaryGet((char*)"LogServerUrl", value, size);
}

static int setTr069SecondaryLogServerUrl(char* value, unsigned int size)
{
    return statisticSecondarySet((char*)"LogServerUrl", value, size);
}

static int getTr069SecondaryLogUploadInterval(char* value, unsigned int size)
{
    return statisticSecondaryGet((char*)"LogUploadInterval", value, size);
}

static int setTr069SecondaryLogUploadInterval(char* value, unsigned int size)
{
    return statisticSecondarySet((char*)"LogUploadInterval", value, size);
}

static int getTr069SecondaryLogRecordInterval(char* value, unsigned int size)
{
    return statisticSecondaryGet((char*)"LogRecordInterval", value, size);
}

static int setTr069SecondaryLogRecordInterval(char* value, unsigned int size)
{
    return statisticSecondarySet((char*)"LogRecordInterval", value, size);
}

static int getTr069SecondaryBufUnderFlowNumbersEnable(char* value, unsigned int size)
{
    return statisticSecondaryGet((char*)"BufUnderFlowNumbersEnable", value, size);
}
static int setTr069SecondaryBufUnderFlowNumbersEnable(char* value, unsigned int size)
{
    return statisticSecondarySet((char*)"BufUnderFlowNumbersEnable", value, size);
}

static int getTr069SecondaryBufOverFlowNumbersEnable(char* value, unsigned int size)
{
    return statisticSecondaryGet((char*)"BufOverFlowNumbersEnable", value, size);
}
static int setTr069SecondaryBufOverFlowNumbersEnable(char* value, unsigned int size)
{
    return statisticSecondarySet((char*)"BufOverFlowNumbersEnable", value, size);
}

static int getTr069SecondaryAuthNumbersEnable(char* value, unsigned int size)
{
    return statisticSecondaryGet((char*)"AuthNumbersEnable", value, size);
}
static int setTr069SecondaryAuthNumbersEnable(char* value, unsigned int size)
{
    return statisticSecondarySet((char*)"AuthNumbersEnable", value, size);
}

static int getTr069SecondaryAuthFailNumbersEnable(char* value, unsigned int size)
{
    return statisticSecondaryGet((char*)"AuthFailNumbersEnable", value, size);
}
static int setTr069SecondaryAuthFailNumbersEnable(char* value, unsigned int size)
{
    return statisticSecondarySet((char*)"AuthFailNumbersEnable", value, size);
}

static int getTr069SecondaryMultiReqNumbersEnable(char* value, unsigned int size)
{
    return statisticSecondaryGet((char*)"MultiReqNumbersEnable", value, size);
}
static int setTr069SecondaryMultiReqNumbersEnable(char* value, unsigned int size)
{
    return statisticSecondarySet((char*)"MultiReqNumbersEnable", value, size);
}

static int getTr069SecondaryMultiFailNumbersEnable(char* value, unsigned int size)
{
    return statisticSecondaryGet((char*)"MultiFailNumbersEnable", value, size);
}
static int setTr069SecondaryMultiFailNumbersEnable(char* value, unsigned int size)
{
    return statisticSecondarySet((char*)"MultiFailNumbersEnable", value, size);
}
static int getTr069SecondaryVodReqNumbersEnable(char* value, unsigned int size)
{
    return statisticSecondaryGet((char*)"VodReqNumbersEnable", value, size);
}
static int setTr069SecondaryVodReqNumbersEnable(char* value, unsigned int size)
{
    return statisticSecondarySet((char*)"VodReqNumbersEnable", value, size);
}

static int getTr069SecondaryVodFailNumbersEnable(char* value, unsigned int size)
{
    return statisticSecondaryGet((char*)"VodFailNumbersEnable", value, size);
}
static int setTr069SecondaryVodFailNumbersEnable(char* value, unsigned int size)
{
    return statisticSecondarySet((char*)"VodFailNumbersEnable", value, size);
}

static int getTr069SecondaryResponseDelayEnable(char* value, unsigned int size)
{
    return statisticSecondaryGet((char*)"ResponseDelayEnable", value, size);
}
static int setTr069SecondaryResponseDelayEnable(char* value, unsigned int size)
{
    return statisticSecondarySet((char*)"ResponseDelayEnable", value, size);
}

static int getTr069SecondaryChannelSwitchDelayEnable(char* value, unsigned int size)
{
    return statisticSecondaryGet((char*)"ChannelSwitchDelayEnable", value, size);
}
static int setTr069SecondaryChannelSwitchDelayEnable(char* value, unsigned int size)
{
    return statisticSecondarySet((char*)"ChannelSwitchDelayEnable", value, size);
}

static int getTr069SecondaryAbnormalIntervalEnable(char* value, unsigned int size)
{
    return statisticSecondaryGet((char*)"AbnormalIntervalEnable", value, size);
}
static int setTr069SecondaryAbnormalIntervalEnable(char* value, unsigned int size)
{
    return statisticSecondarySet((char*)"AbnormalIntervalEnable", value, size);
}

static int getTr069SecondaryAuthSuccRateEnable(char* value, unsigned int size)
{
    return statisticSecondaryGet((char*)"AuthSuccRateEnable", value, size);
}
static int setTr069SecondaryAuthSuccRateEnable(char* value, unsigned int size)
{
    return statisticSecondarySet((char*)"AuthSuccRateEnable", value, size);
}

static int getTr069SecondaryPlayDurationEnable(char* value, unsigned int size)
{
    return statisticSecondaryGet((char*)"PlayDurationEnable", value, size);
}
static int setTr069SecondaryPlayDurationEnable(char* value, unsigned int size)
{
    return statisticSecondarySet((char*)"PlayDurationEnable", value, size);
}

static int getTr069SecondaryBadDurationEnable(char* value, unsigned int size)
{
    return statisticSecondaryGet((char*)"BadDurationEnable", value, size);
}
static int setTr069SecondaryBadDurationEnable(char* value, unsigned int size)
{
    return statisticSecondarySet((char*)"BadDurationEnable", value, size);
}
static int getTr069SecondaryAvailabilityEnable(char* value, unsigned int size)
{
    return statisticSecondaryGet((char*)"AvailabilityEnable", value, size);
}
static int setTr069SecondaryAvailabilityEnable(char* value, unsigned int size)
{
    return statisticSecondarySet((char*)"AvailabilityEnable", value, size);
}
static int getTr069SecondaryChannelNameEnable(char* value, unsigned int size)
{
    return statisticSecondaryGet((char*)"ChannelNameEnable", value, size);
}
static int setTr069SecondaryChannelNameEnable(char* value, unsigned int size)
{
    return statisticSecondarySet((char*)"ChannelNameEnable", value, size);
}
static int getTr069SecondaryChannelAddressEnable(char* value, unsigned int size)
{
    return statisticSecondaryGet((char*)"ChannelAddressEnable", value, size);
}
static int setTr069SecondaryChannelAddressEnable(char* value, unsigned int size)
{
    return statisticSecondarySet((char*)"ChannelAddressEnable", value, size);
}
static int getTr069SecondaryTransmissionEnable(char* value, unsigned int size)
{
    return statisticSecondaryGet((char*)"TransmissionEnable", value, size);
}
static int setTr069SecondaryTransmissionEnable(char* value, unsigned int size)
{
    return statisticSecondarySet((char*)"TransmissionEnable", value, size);
	
}

static int getTr069SecondaryProgramStartTimeEnable(char* value, unsigned int size)
{
    return statisticSecondaryGet((char*)"ProgramStartTimeEnable", value, size);
}
static int setTr069SecondaryProgramStartTimeEnable(char* value, unsigned int size)
{
    return statisticSecondarySet((char*)"ProgramStartTimeEnable", value, size);
}

static int getTr069SecondaryProgramEndTimeEnable(char* value, unsigned int size)
{
    return statisticSecondaryGet((char*)"ProgramEndTimeEnable", value, size);
}
static int setTr069SecondaryProgramEndTimeEnable(char* value, unsigned int size)
{
    return statisticSecondarySet((char*)"ProgramEndTimeEnable", value, size);
}

static int getTr069SecondaryJitterEnable(char* value, unsigned int size)
{
    return statisticSecondaryGet((char*)"JitterEnable", value, size);
}
static int setTr069SecondaryJitterEnable(char* value, unsigned int size)
{
    return statisticSecondarySet((char*)"JitterEnable", value, size);
}

static int getTr069SecondaryMdiMLREnable(char* value, unsigned int size)
{
    return statisticSecondaryGet((char*)"MdiMLREnable", value, size);
}
static int setTr069SecondaryMdiMLREnable(char* value, unsigned int size)
{
    return statisticSecondarySet((char*)"MdiMLREnable", value, size);
	
}
static int getTr069SecondaryMdiDFEnable(char* value, unsigned int size)
{
    return statisticSecondaryGet((char*)"MdiDFEnable", value, size);
}
static int setTr069SecondaryMdiDFEnable(char* value, unsigned int size)
{
    return statisticSecondarySet((char*)"MdiDFEnable", value, size);
}
static int getTr069SecondaryBitRateEnable(char* value, unsigned int size)
{
    return statisticSecondaryGet((char*)"BitRateEnable", value, size);
}
static int setTr069SecondaryBitRateEnable(char* value, unsigned int size)
{
    return statisticSecondarySet((char*)"BitRateEnable", value, size);
}
static int getTr069SecondaryVideoQualityEnable(char* value, unsigned int size)
{
    return statisticSecondaryGet((char*)"VideoQualityEnable", value, size);
}
static int setTr069SecondaryVideoQualityEnable(char* value, unsigned int size)
{
    return statisticSecondarySet((char*)"VideoQualityEnable", value, size);
}
static int getTr069SecondaryChannelRequestFrequencyEnable(char* value, unsigned int size)
{
    return statisticSecondaryGet((char*)"ChannelRequestFrequencyEnable", value, size);
}
static int setTr069SecondaryChannelRequestFrequencyEnable(char* value, unsigned int size)
{
    return statisticSecondarySet((char*)"ChannelRequestFrequencyEnable", value, size);
}
static int getTr069SecondaryAccessSuccessNumberEnable(char* value, unsigned int size)
{
    return statisticSecondaryGet((char*)"AccessSuccessNumberEnable", value, size);
}
static int setTr069SecondaryAccessSuccessNumberEnable(char* value, unsigned int size)
{
    return statisticSecondarySet((char*)"AccessSuccessNumberEnable", value, size);
}

static int getTr069SecondaryAverageAccessTimeEnable(char* value, unsigned int size)
{
    return statisticSecondaryGet((char*)"AverageAccessTimeEnable", value, size);
}
static int setTr069SecondaryAverageAccessTimeEnable(char* value, unsigned int size)
{
    return statisticSecondarySet((char*)"AverageAccessTimeEnable", value, size);
}

static int getTr069SecondaryWatchLongEnable(char* value, unsigned int size)
{
    return statisticSecondaryGet((char*)"WatchLongEnable", value, size);
}
static int setTr069SecondaryWatchLongEnable(char* value, unsigned int size)
{
    return statisticSecondarySet((char*)"WatchLongEnable", value, size);
}

static int getTr069SecondaryMediaStreamBandwidthEnable(char* value, unsigned int size)
{
    return statisticSecondaryGet((char*)"MediaStreamBandwidthEnable", value, size);
}
static int setTr069SecondaryMediaStreamBandwidthEnable(char* value, unsigned int size)
{
    return statisticSecondarySet((char*)"MediaStreamBandwidthEnable", value, size);
}



StatisticConfiguration2::StatisticConfiguration2()
	: Tr069GroupCall("2")
{
    statisticSecondaryInit( );

    Tr069Call* fun1  = new Tr069FunctionCall("Enable", getTr069SecondaryLogEnable, setTr069SecondaryLogEnable);
    regist(fun1->name(), fun1); 
    
    Tr069Call* fun2  = new Tr069FunctionCall("LogServerUrl", getTr069SecondaryLogServerUrl, setTr069SecondaryLogServerUrl);
    regist(fun2->name(), fun2);    
    
    Tr069Call* fun3  = new Tr069FunctionCall("LogUploadInterval", getTr069SecondaryLogUploadInterval, setTr069SecondaryLogUploadInterval);
    regist(fun3->name(), fun3);    
    
    Tr069Call* fun4  = new Tr069FunctionCall("LogRecordInterval", getTr069SecondaryLogRecordInterval, setTr069SecondaryLogRecordInterval);
    regist(fun4->name(), fun4);


    Tr069Call* fun5  = new Tr069FunctionCall("BufUnderFlowNumbersEnable", getTr069SecondaryBufUnderFlowNumbersEnable, setTr069SecondaryBufUnderFlowNumbersEnable);
    regist(fun5->name(), fun5); 
    
    Tr069Call* fun6  = new Tr069FunctionCall("BufOverFlowNumbersEnable", getTr069SecondaryBufOverFlowNumbersEnable, setTr069SecondaryBufOverFlowNumbersEnable);
    regist(fun6->name(), fun6);    
    
    Tr069Call* fun7  = new Tr069FunctionCall("AuthNumbersEnable", getTr069SecondaryAuthNumbersEnable, setTr069SecondaryAuthNumbersEnable);
    regist(fun7->name(), fun7);    
    
    Tr069Call* fun8  = new Tr069FunctionCall("AuthFailNumbersEnable", getTr069SecondaryAuthFailNumbersEnable, setTr069SecondaryAuthFailNumbersEnable);
    regist(fun8->name(), fun8);


    Tr069Call* fun9  = new Tr069FunctionCall("MultiReqNumbersEnable", getTr069SecondaryMultiReqNumbersEnable, setTr069SecondaryMultiReqNumbersEnable);
    regist(fun9->name(), fun9); 
    
    Tr069Call* fun10  = new Tr069FunctionCall("MultiFailNumbersEnable", getTr069SecondaryMultiFailNumbersEnable, setTr069SecondaryMultiFailNumbersEnable);
    regist(fun10->name(), fun10);    
    
    Tr069Call* fun11  = new Tr069FunctionCall("VodReqNumbersEnable", getTr069SecondaryVodReqNumbersEnable, setTr069SecondaryVodReqNumbersEnable);
    regist(fun11->name(), fun11);    
    
    Tr069Call* fun12  = new Tr069FunctionCall("VodFailNumbersEnable", getTr069SecondaryVodFailNumbersEnable, setTr069SecondaryVodFailNumbersEnable);
    regist(fun12->name(), fun12);

    Tr069Call* fun13  = new Tr069FunctionCall("ResponseDelayEnable", getTr069SecondaryResponseDelayEnable, setTr069SecondaryResponseDelayEnable);
    regist(fun13->name(), fun13); 
    
    Tr069Call* fun14  = new Tr069FunctionCall("ChannelSwitchDelayEnable", getTr069SecondaryChannelSwitchDelayEnable, setTr069SecondaryChannelSwitchDelayEnable);
    regist(fun14->name(), fun14);    
    
    Tr069Call* fun15  = new Tr069FunctionCall("AbnormalIntervalEnable", getTr069SecondaryAbnormalIntervalEnable, setTr069SecondaryAbnormalIntervalEnable);
    regist(fun15->name(), fun15);    
    
    Tr069Call* fun16  = new Tr069FunctionCall("AuthSuccRateEnable", getTr069SecondaryAuthSuccRateEnable, setTr069SecondaryAuthSuccRateEnable);
    regist(fun16->name(), fun16);

    Tr069Call* fun17  = new Tr069FunctionCall("PlayDurationEnable", getTr069SecondaryPlayDurationEnable, setTr069SecondaryPlayDurationEnable);
    regist(fun17->name(), fun17); 
    
    Tr069Call* fun18  = new Tr069FunctionCall("BadDurationEnable", getTr069SecondaryBadDurationEnable, setTr069SecondaryBadDurationEnable);
    regist(fun18->name(), fun18);    
    
    Tr069Call* fun19  = new Tr069FunctionCall("AvailabilityEnable", getTr069SecondaryAvailabilityEnable, setTr069SecondaryAvailabilityEnable);
    regist(fun19->name(), fun19);    
    
    Tr069Call* fun20  = new Tr069FunctionCall("ChannelNameEnable", getTr069SecondaryChannelNameEnable, setTr069SecondaryChannelNameEnable);
    regist(fun20->name(), fun20);	

    Tr069Call* fun21  = new Tr069FunctionCall("ChannelAddressEnable", getTr069SecondaryChannelAddressEnable, setTr069SecondaryChannelAddressEnable);
    regist(fun21->name(), fun21);	

   Tr069Call* fun22 = new Tr069FunctionCall("TransmissionEnable", getTr069SecondaryTransmissionEnable, setTr069SecondaryTransmissionEnable);
    regist(fun22->name(), fun22);	

   Tr069Call* fun23  = new Tr069FunctionCall("ProgramStartTimeEnable", getTr069SecondaryProgramStartTimeEnable, setTr069SecondaryProgramStartTimeEnable);
    regist(fun23->name(), fun23);	

   Tr069Call* fun24  = new Tr069FunctionCall("ProgramEndTimeEnable", getTr069SecondaryProgramEndTimeEnable, setTr069SecondaryProgramEndTimeEnable);
    regist(fun24->name(), fun24);	

    Tr069Call* fun25  = new Tr069FunctionCall("JitterEnable", getTr069SecondaryJitterEnable, setTr069SecondaryJitterEnable);
    regist(fun25->name(), fun25);	

    Tr069Call* fun26  = new Tr069FunctionCall("MdiMLREnable", getTr069SecondaryMdiMLREnable, setTr069SecondaryMdiMLREnable);
    regist(fun26->name(), fun26);	

   Tr069Call* fun27  = new Tr069FunctionCall("MdiDFEnable", getTr069SecondaryMdiDFEnable, setTr069SecondaryMdiDFEnable);
    regist(fun27->name(), fun27);	

   Tr069Call* fun28  = new Tr069FunctionCall("BitRateEnable", getTr069SecondaryBitRateEnable, setTr069SecondaryBitRateEnable);
    regist(fun28->name(), fun28);	

   Tr069Call* fun29  = new Tr069FunctionCall("VideoQualityEnable", getTr069SecondaryVideoQualityEnable, setTr069SecondaryVideoQualityEnable);
    regist(fun29->name(), fun29);
	
   Tr069Call* fun30  = new Tr069FunctionCall("ChannelRequestFrequencyEnable", getTr069SecondaryChannelRequestFrequencyEnable, setTr069SecondaryChannelRequestFrequencyEnable);
    regist(fun30->name(), fun30);	

   Tr069Call* fun31  = new Tr069FunctionCall("AccessSuccessNumberEnable", getTr069SecondaryAccessSuccessNumberEnable, setTr069SecondaryAccessSuccessNumberEnable);
    regist(fun31->name(), fun31);	
             
     Tr069Call* fun32  = new Tr069FunctionCall("AverageAccessTimeEnable", getTr069SecondaryAverageAccessTimeEnable, setTr069SecondaryAverageAccessTimeEnable);
    regist(fun32->name(), fun32);	

    Tr069Call* fun33 = new Tr069FunctionCall("WatchLongEnable", getTr069SecondaryWatchLongEnable, setTr069SecondaryWatchLongEnable);
    regist(fun33->name(), fun33);	

    Tr069Call* fun34  = new Tr069FunctionCall("MediaStreamBandwidthEnable", getTr069SecondaryMediaStreamBandwidthEnable, setTr069SecondaryMediaStreamBandwidthEnable);
    regist(fun34->name(), fun34);	

                                
}

StatisticConfiguration2::~StatisticConfiguration2()
{
}
