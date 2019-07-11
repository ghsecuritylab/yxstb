
#include "TR069Assertions.h"
#include "Tr069FunctionCall.h"
#include "Tr069LogParaConfiguration.h"

#include "LogModuleHuawei.h"

#include "independs/ind_tmr.h"

#include <stdio.h>
#include <stdlib.h>

// LogModuleForTR069.cpp 没有头文件
extern "C" unsigned int logTR069GetLogStartTime();
extern "C" void logTR069SetLogStartTime(unsigned int startTime);
extern "C" unsigned int logTR069GetLogContinueTime();
extern "C" void logTR069SetLogContinueTimeDelay(unsigned int continueTime);

/*------------------------------------------------------------------------------
 
 ------------------------------------------------------------------------------*/
static int getTr069PortLogType(char* value, unsigned int size)
{ 
    if (snprintf(value, size,"%d", huaweiGetLogType()) >= (int)size) {
        printf("Error.size is too short.");
        return -1;
    }   
    return 0;
}

static int setTr069PortLogType(char* value, unsigned int size)
{
    huaweiSetLogType(atoi(value));
    
    return 0;
}

/*------------------------------------------------------------------------------
 
 ------------------------------------------------------------------------------*/
static int getTr069PortLogLevel(char* value, unsigned int size)
{ 
    if (snprintf(value, size,"%d", huaweiGetLogLevel()) >= (int)size) {
        printf("Error.size is too short.");
        return -1;
    }   
    return 0;
}

static int setTr069PortLogLevel(char* value, unsigned int size)
{
    huaweiSetLogLevel(atoi(value));
    
    return 0;
}

/*------------------------------------------------------------------------------
 
 ------------------------------------------------------------------------------*/
static int getTr069PortLogOutPutType(char* value, unsigned int size)
{ 
    if (snprintf(value, size,"%d", huaweiGetLogOutPutType()) >= (int)size) {
        printf("Error.size is too short.");
        return -1;
    }   
    return 0;
}

static int setTr069PortLogOutPutType(char* value, unsigned int size)
{
    huaweiSetLogOutPutType(atoi(value));
    
    return 0;
}

/*------------------------------------------------------------------------------
 
 ------------------------------------------------------------------------------*/
static int getTr069PortSyslogServer(char* value, unsigned int size)
{ 
    huaweiGetLogUDPServer(value, size);

    return 0;
}

static int setTr069PortSyslogServer(char* value, unsigned int size)
{
    huaweiSetLogUDPServer(value);
    
    return 0;
}

/*------------------------------------------------------------------------------
 ------------------------------------------------------------------------------*/
static int getTr069PortSyslogStartTime(char* value, unsigned int size)
{
    unsigned int sec;
    struct ind_time t;

    sec = logTR069GetLogStartTime();
    ind_time_local(sec, &t);

    snprintf(value, size, "%04d-%02d-%02dT%02d:%02d:%02d", t.year, t.mon, t.day, t.hour, t.min, t.sec);

    return 0;
}

static int setTr069PortSyslogStartTime(char* value, unsigned int size)
{
    unsigned int sec;
    struct ind_time t;

    if(sscanf(value, "%04d-%02d-%02dT%02d:%02d:%02d", &t.year, &t.mon, &t.day, &t.hour, &t.min, &t.sec) != 6) {
        LogTr069Error("sscanf %s\n", value);
        return -1;
    }
    sec = ind_time_make(&t);

    logTR069SetLogStartTime(sec);

    return 0;
}

/*------------------------------------------------------------------------------
 ------------------------------------------------------------------------------*/
static int getTr069PortSyslogContinueTime(char* value, unsigned int size)
{ 
    if (snprintf(value, size,"%u", logTR069GetLogContinueTime()) >= (int)size) {
        printf("Error.size is too short.");
        return -1;
    }   
    return 0;
}

static int setTr069PortSyslogContinueTime(char* value, unsigned int size)
{
    logTR069SetLogContinueTimeDelay((unsigned int)atoi(value));
    
    return 0;
}


/*------------------------------------------------------------------------------
 * 1,以下对象的注册到表root.Device.X_00E0FC.LogParaConfiguration.
 * 2,实现函数会被mgmt共同使用，因此只做函数简介调用，没有把实现移过来。
 ------------------------------------------------------------------------------*/

Tr069LogParaConfiguration::Tr069LogParaConfiguration()
	: Tr069GroupCall("LogParaConfiguration")
{
	Tr069Call* call;

    call  = new Tr069FunctionCall("LogType", getTr069PortLogType, setTr069PortLogType);
    regist(call->name(), call);    

  
    call  = new Tr069FunctionCall("LogLevel", getTr069PortLogLevel, setTr069PortLogLevel);
    regist(call->name(), call);    
    
    call  = new Tr069FunctionCall("LogOutPutType", getTr069PortLogOutPutType, setTr069PortLogOutPutType);
    regist(call->name(), call);   
    
    call  = new Tr069FunctionCall("SyslogServer", getTr069PortSyslogServer, setTr069PortSyslogServer);
    regist(call->name(), call);    

  
    call  = new Tr069FunctionCall("SyslogStartTime", getTr069PortSyslogStartTime, setTr069PortSyslogStartTime);
    regist(call->name(), call);    
    
    call  = new Tr069FunctionCall("SyslogContinueTime", getTr069PortSyslogContinueTime, setTr069PortSyslogContinueTime);
    regist(call->name(), call);   
    

}

Tr069LogParaConfiguration::~Tr069LogParaConfiguration()
{
}
