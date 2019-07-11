#include "Tr069PmInfo.h"

#include "Tr069FunctionCall.h"

#include <stdio.h>

#ifdef TR069_LIANCHUANG

static int getAppTr069PortQosPacketsLost(char* str, unsigned int val)
{
    snprintf(str, val, "%d", app_tr069_port_get_Qos_PacketsLost());
    
    return 0; 				
}

static int getAppTr069PortQosBitRate(char* str, unsigned int val)
{
    snprintf(str, val, "%d", app_tr069_port_get_Qos_BitRate());
    
    return 0; 		
}

static int getAppTr069PortQosFractionLost(char* str, unsigned int val)
{
    app_tr069_port_get_Qos_FractionLost(str, val);
    
    return 0;	
}

static int getAppTr069PortPminfoMinDF(char* str, unsigned int val)
{
    snprintf(str, val, "%d", app_tr069_port_get_Pminfo_MinDF());
    
    return 0; 			
}

static int getAppTr069PortPminfoAvgDF(char* str, unsigned int val)
{
    snprintf(str, val, "%d", app_tr069_port_get_Pminfo_AvgDF());
    
    return 0; 		
}

static int getAppTr069PortPminfoMaxDF(char* str, unsigned int val)
{
    snprintf(str, val, "%d", app_tr069_port_get_Pminfo_MaxDF());
    
    return 0; 		
}

static int getAppTr069PortPminfoDithering(char* str, unsigned int val)
{
    snprintf(str, val, "%d", app_tr069_port_get_Pminfo_Dithering());
    
    return 0; 		
}

#endif

Tr069PmInfo::Tr069PmInfo()
	: Tr069GroupCall("PmInfo")
{
#ifdef TR069_LIANCHUANG

    Tr069Call* PmLossPacketsNum = new Tr069FunctionCall("PmLossPacketsNum", getAppTr069PortQosPacketsLost, NULL);
    Tr069Call* PmBitRate        = new Tr069FunctionCall("PmBitRate", getAppTr069PortQosBitRate, NULL);   
    Tr069Call* PmLostRate       = new Tr069FunctionCall("PmLostRate", getAppTr069PortQosFractionLost, NULL);
    Tr069Call* MinDF            = new Tr069FunctionCall("MinDF", getAppTr069PortPminfoMinDF, NULL);
    Tr069Call* AvgDF            = new Tr069FunctionCall("AvgDF", getAppTr069PortPminfoAvgDF, NULL);
    Tr069Call* MaxDF            = new Tr069FunctionCall("MaxDF", getAppTr069PortPminfoMaxDF, NULL);
    Tr069Call* Dithering        = new Tr069FunctionCall("Dithering", getAppTr069PortPminfoDithering, NULL);

    
    regist(PmLossPacketsNum->name(), PmLossPacketsNum);
    regist(PmBitRate->name(), PmBitRate);
    regist(PmLostRate->name(), PmLostRate);
    regist(MinDF->name(), MinDF);
    regist(AvgDF->name(), AvgDF);
    regist(MaxDF->name(), MaxDF);
    regist(Dithering->name(), Dithering);

#endif

}

Tr069PmInfo::~Tr069PmInfo()
{
}
