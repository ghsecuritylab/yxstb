
#include "Tr069AlarmConfig.h"
#include "Tr069FunctionCall.h"

#include "TR069Assertions.h"

#include "app_tr069_alarm.h"

static int setTr069PortPacketsLostAlarmValue(char* str, unsigned int val)
{
    app_setPacketsLostAlarmValue(str);
    return 0;
}

#ifdef Sichuan
static int setTr069PortFramesLostAlarmValue(char* str, unsigned int val)
{
    app_setFramesLostAlarmValue(str);
    return 0;
}

static int setTr069PortTimeLapseAlarmValue(char* str, unsigned int val)
{
    return 0;
}

static int setTr069PortCushionAlarmValue(char* str, unsigned int val)
{
    return 0;
}
#endif

Tr069AlarmConfig::Tr069AlarmConfig()
	: Tr069GroupCall("AlarmConfig")
{
	
	
    Tr069Call* PacketsLostAlarmValue = new Tr069FunctionCall("PacketsLostAlarmValue", NULL, setTr069PortPacketsLostAlarmValue);
#ifdef Sichuan
    Tr069Call* FramesLostAlarmValue  = new Tr069FunctionCall("FramesLostAlarmValue",  NULL, setTr069PortFramesLostAlarmValue);
    Tr069Call* TimeLapseAlarmValue   = new Tr069FunctionCall("TimeLapseAlarmValue",   NULL, setTr069PortTimeLapseAlarmValue);
    Tr069Call* CushionAlarmValue     = new Tr069FunctionCall("CushionAlarmValue",     NULL, setTr069PortCushionAlarmValue);
#endif

    regist(PacketsLostAlarmValue->name(), PacketsLostAlarmValue);
#ifdef Sichuan
    regist(FramesLostAlarmValue->name(),  FramesLostAlarmValue);
    regist(TimeLapseAlarmValue->name(),   TimeLapseAlarmValue);
    regist(CushionAlarmValue->name(),     CushionAlarmValue);
#endif
   
}

Tr069AlarmConfig::~Tr069AlarmConfig()
{
}

