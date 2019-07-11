#include "Tr069X_CTC_IPTV_Alarm.h"

#include "Tr069FunctionCall.h"
#include "Tr069AlarmConfig.h"

#include "app_tr069_alarm.h"

#include <stdio.h>
#include <stdlib.h>

static int setTr069PortAlarmSwitch(char* str, unsigned int val)
{
    app_setAlarmSwitch(str);
    return 0;
}

Tr069X_CTC_IPTV_Alarm::Tr069X_CTC_IPTV_Alarm()
	: Tr069GroupCall("X_CTC_IPTV_Alarm")
{
    Tr069Call* AlarmConfig = new Tr069AlarmConfig();
    regist(AlarmConfig->name(), AlarmConfig);

    Tr069Call* AlarmSwitch      = new Tr069FunctionCall("AlarmSwitch", NULL, setTr069PortAlarmSwitch);

    regist(AlarmSwitch->name(), AlarmSwitch);
}

Tr069X_CTC_IPTV_Alarm::~Tr069X_CTC_IPTV_Alarm()
{
}
