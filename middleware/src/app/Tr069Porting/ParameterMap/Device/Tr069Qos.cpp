#include "Tr069Qos.h"

#include "Tr069FunctionCall.h"



#include <stdio.h>
#include <stdlib.h>

#ifdef TR069_LIANCHUANG

static int getAppTr069PortQosResetStatistics(char* str, unsigned int val)
{
    return 0;
}

static int setAppTr069PortQosResetStatistics(char* str, unsigned int val)
{
    app_tr069_port_set_Qos_ResetStatistics(atoi(str));

    return 0;
}

static int getAppTr069PortQosPacketsReceived(char* str, unsigned int val)
{
    snprintf(str, val, "%d", app_tr069_port_get_Qos_PacketsReceived());
    
    return 0;
}

static int getAppTr069PortQosPacketsLost(char* str, unsigned int val)
{
    snprintf(str, val, "%d", app_tr069_port_get_Qos_PacketsLost());
    
    return 0;
}
static int getAppTr069PortQosBytesReceived(char* str, unsigned int val)
{
    snprintf(str, val, "%d", app_tr069_port_get_Qos_BytesReceived());
    
    return 0;
}

static int getAppTr069PortQosFractionLost(char* str, unsigned int val)
{
    app_tr069_port_get_Qos_FractionLost(str, val);
    
    return 0;
}

static int getAppTr069PortQosBitRate(char* str, unsigned int val)
{
    snprintf(str, val, "%d", app_tr069_port_get_Qos_BitRate());
    
    return 0;
}

#endif

Tr069Qos::Tr069Qos()
	: Tr069GroupCall("Qos")
{
#ifdef TR069_LIANCHUANG

    Tr069Call* ResetStatistics = new Tr069FunctionCall("ResetStatistics", getAppTr069PortQosResetStatistics, setAppTr069PortQosResetStatistics);
    Tr069Call* PacketsReceived = new Tr069FunctionCall("PacketsReceived", getAppTr069PortQosPacketsReceived, NULL);   
    Tr069Call* PacketsLost     = new Tr069FunctionCall("PacketsLost", getAppTr069PortQosPacketsLost, NULL);
    Tr069Call* BytesReceived   = new Tr069FunctionCall("BytesReceived", getAppTr069PortQosBytesReceived, NULL);
    Tr069Call* FractionLost    = new Tr069FunctionCall("FractionLost", getAppTr069PortQosFractionLost, NULL);
    Tr069Call* BitRate         = new Tr069FunctionCall("BitRate", getAppTr069PortQosBitRate, NULL);

    
    regist(ResetStatistics->name(), ResetStatistics);
    regist(PacketsReceived->name(), PacketsReceived);
    regist(PacketsLost->name(), PacketsLost);
    regist(BytesReceived->name(), BytesReceived);
    regist(FractionLost->name(), FractionLost);
    regist(BitRate->name(), BitRate);
    
#endif

}

Tr069Qos::~Tr069Qos()
{
}
