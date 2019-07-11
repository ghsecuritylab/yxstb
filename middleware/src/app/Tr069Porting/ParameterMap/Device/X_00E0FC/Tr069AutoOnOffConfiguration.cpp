#include "Tr069AutoOnOffConfiguration.h"

#include "Tr069FunctionCall.h"

#include "mid/mid_time.h"
#include "mid/mid_timer.h"
#include "mid/mid_tools.h"
#include "mid_fpanel.h"
#include "sys_msg.h"
#include "MessageTypes.h"
#include "ipanel_event.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int gIsAutoPowerOn = 0;// 1:on;0:off
static char gAutoPowerOnTime[6] = "";//00:00
static char gAutoShutdownTime[6] = "";//00:00
static int gpowerOnTime = -1;
static int gshutdownTime = -1;

static void autoPowerTimer(int arg)
{
    unsigned int curtime = mid_time();
    char curTempTime[18] = "";
    mid_tool_time2string(curtime, curTempTime, ' ');
    unsigned int nowTime = atoi(curTempTime + 9) * 60 + atoi(curTempTime + 12);
    if (mid_fpanel_standby_get()) {
        if (gpowerOnTime != -1 && nowTime >= gpowerOnTime)
            sendMessageToNativeHandler(MessageType_KeyDown, EIS_IRKEY_POWER, 0, 0);
    } else {
        if (gshutdownTime != -1 && nowTime >= gshutdownTime)
            sendMessageToNativeHandler(MessageType_System, EIS_IRKEY_SHOWSTANDBY, 0, 0);
    }

    //if (gIsAutoPowerOn)
        //mid_timer_create(1, 1, autoPowerTimer, 0);
}

static void tr069_port_AutoPowerTimer_stop()
{
    mid_timer_delete_all(autoPowerTimer);
}

static void tr069_port_AutoPowerTimer_start()
{
    tr069_port_AutoPowerTimer_stop();
    if (gIsAutoPowerOn)
        mid_timer_create(1, 0, autoPowerTimer, 0);
}

/*------------------------------------------------------------------------------
 * 自动开机参数
 ------------------------------------------------------------------------------*/
static int getTr069PortIsAutoPowerOn(char* value, unsigned int size)
{ 
    if (snprintf(value, size,"%d", gIsAutoPowerOn) >= (int)size) {
        printf("Error.size is too short.");
        return -1;
    }   
    return 0;
}

static int setTr069PortIsAutoPowerOn(char* value, unsigned int size)
{
	int enable;
	enable = atoi(value);
    if (1 != enable && 0 != enable)
        return -1;

    if (enable != gIsAutoPowerOn) {
        gIsAutoPowerOn = enable;

        if (gIsAutoPowerOn)
            tr069_port_AutoPowerTimer_start();
        else
            tr069_port_AutoPowerTimer_stop();
    }   
    return 0;
}

/*------------------------------------------------------------------------------
 * 自动开机参数
 ------------------------------------------------------------------------------*/
static int getTr069PortAutoPowerOnTime(char* value, unsigned int size)
{
    strncpy(value, gAutoPowerOnTime, size);    
    return 0;
}

static int setTr069PortAutoPowerOnTime(char* value, unsigned int size)
{
    if (!value || !strlen(value))
        return -1;

    if (value[2] == ':' && atoi(value) < 24 && atoi(value + 3) < 60) {
        memset(gAutoPowerOnTime, 0, 6);
        strcpy(gAutoPowerOnTime, value);
        gAutoPowerOnTime[5] = 0;
        gpowerOnTime = atoi(gAutoPowerOnTime) * 60 + atoi(gAutoPowerOnTime + 3);
    }   
    return 0;
}

/*------------------------------------------------------------------------------
 * 自动开机参数
 ------------------------------------------------------------------------------*/
static int getTr069PortAutoShutdownTime(char* value, unsigned int size)
{
    strncpy(value, gAutoShutdownTime, size);    
    return 0;
}

static int setTr069PortAutoShutdownTime(char* value, unsigned int size)
{
    if (!value || !strlen(value))
        return -1;

    if (value[2] == ':' && atoi(value) < 24 && atoi(value + 3) < 60) {
        memset(gAutoShutdownTime, 0, 6);
        strcpy(gAutoShutdownTime, value);
        gAutoShutdownTime[5] = 0;
        gshutdownTime = atoi(gAutoShutdownTime) * 60 + atoi(gAutoShutdownTime + 3);
    }    
    return 0;
}

/*------------------------------------------------------------------------------
 * 以下对象的注册到表root.Device.X_00E0FC.AutoOnOffConfiguration
 ------------------------------------------------------------------------------*/

Tr069AutoOnOffConfiguration::Tr069AutoOnOffConfiguration()
	: Tr069GroupCall("AutoOnOffConfiguration")
{

    Tr069Call* fun1  = new Tr069FunctionCall("IsAutoPowerOn", getTr069PortIsAutoPowerOn, setTr069PortIsAutoPowerOn);
    regist(fun1->name(), fun1);    

  
    Tr069Call* fun2  = new Tr069FunctionCall("AutoPowerOnTime", getTr069PortAutoPowerOnTime, setTr069PortAutoPowerOnTime);
    regist(fun2->name(), fun2);    
    
    Tr069Call* fun3  = new Tr069FunctionCall("AutoShutdownTime", getTr069PortAutoShutdownTime, setTr069PortAutoShutdownTime);
    regist(fun3->name(), fun3);    
    

}

Tr069AutoOnOffConfiguration::~Tr069AutoOnOffConfiguration()
{
}
