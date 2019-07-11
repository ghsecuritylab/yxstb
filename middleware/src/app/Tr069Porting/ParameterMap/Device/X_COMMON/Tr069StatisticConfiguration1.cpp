#include "Tr069StatisticConfiguration1.h"
#include "StatisticCTC/StatisticCTCSichuan1.h"
#include "Tr069FunctionCall.h"
#include "StatisticBase.h"

static int getLogEnable(char* value, unsigned int size)
{
    return statisticPrimaryGet((char*)(char*)"Enable", value, size);
}

static int setLogEnable(char* value, unsigned int size)
{
    return statisticPrimarySet((char*)"Enable", value, size);
}

static int getLogServerUrl(char* value, unsigned int size)
{
    return statisticPrimaryGet((char*)"LogServerUrl", value, size);
}

static int setLogServerUrl(char* value, unsigned int size)
{
    return statisticPrimarySet((char*)"LogServerUrl", value, size);
}

static int getLogUploadInterval(char* value, unsigned int size)
{
    return statisticPrimaryGet((char*)"LogUploadInterval", value, size);
}

static int setLogUploadInterval(char* value, unsigned int size)
{
    return statisticPrimarySet((char*)"LogUploadInterval", value, size);
}

static int gettLogRecordInterval(char* value, unsigned int size)
{
    return statisticPrimaryGet((char*)"LogRecordInterval", value, size);
}

static int settLogRecordInterval(char* value, unsigned int size)
{
    return statisticPrimarySet((char*)"LogRecordInterval", value, size);
}

static int getCPURateEnable(char* value, unsigned int size)
{
    return statisticPrimaryGet((char*)"CPURateEnable", value, size);
}
static int setCPURateEnable(char* value, unsigned int size)
{
    return statisticPrimarySet((char*)"CPURateEnable", value, size);
}

static int getMemRateEnable(char* value, unsigned int size)
{
    return statisticPrimaryGet((char*)"MemRateEnable", value, size);
}
static int setMemRateEnable(char* value, unsigned int size)
{
    return statisticPrimarySet((char*)"MemRateEnable", value, size);
}

static int getJitterEnable(char* value, unsigned int size)
{
    return statisticPrimaryGet((char*)"JitterEnable", value, size);
}
static int setJitterEnable(char* value, unsigned int size)
{
    return statisticPrimarySet((char*)"JitterEnable", value, size);
}

static int getMdiMLREnable(char* value, unsigned int size)
{
    return statisticPrimaryGet((char*)"MdiMLREnable", value, size);
}
static int setMdiMLREnable(char* value, unsigned int size)
{
    return statisticPrimarySet((char*)"MdiMLREnable", value, size);
}

static int getMdiDFEnable(char* value, unsigned int size)
{
    return statisticPrimaryGet((char*)"MdiDFEnable", value, size);
}
static int setMdiDFEnable(char* value, unsigned int size)
{
    return statisticPrimarySet((char*)"MdiDFEnable", value, size);
}


StatisticConfiguration1::StatisticConfiguration1()
	: Tr069GroupCall("1")
{
    statisticPrimaryInit( );

    Tr069Call* fun1  = new Tr069FunctionCall("Enable", getLogEnable, setLogEnable);
    regist(fun1->name(), fun1); 
    
    Tr069Call* fun2  = new Tr069FunctionCall("LogServerUrl", getLogServerUrl, setLogServerUrl);
    regist(fun2->name(), fun2);    
    
    Tr069Call* fun3  = new Tr069FunctionCall("LogUploadInterval", getLogUploadInterval, setLogUploadInterval);
    regist(fun3->name(), fun3);    
    
    Tr069Call* fun4  = new Tr069FunctionCall("LogRecordInterval", gettLogRecordInterval, settLogRecordInterval);
    regist(fun4->name(), fun4);
    
    
    Tr069Call* fun5  = new Tr069FunctionCall("CPURateEnable", getCPURateEnable, setCPURateEnable);
    regist(fun5->name(), fun5); 
    
    Tr069Call* fun6  = new Tr069FunctionCall("MemRateEnable", getMemRateEnable, setMemRateEnable);
    regist(fun6->name(), fun6);    
    
    Tr069Call* fun7  = new Tr069FunctionCall("JitterEnable", getJitterEnable, setJitterEnable);
    regist(fun7->name(), fun7);    
    
    Tr069Call* fun8  = new Tr069FunctionCall("MdiMLREnable", getMdiMLREnable, setMdiMLREnable);
    regist(fun8->name(), fun8);  
        
    Tr069Call* fun9  = new Tr069FunctionCall("MdiDFEnable", getMdiDFEnable, setMdiDFEnable);
    regist(fun9->name(), fun9);  
}

StatisticConfiguration1::~StatisticConfiguration1()
{
}
