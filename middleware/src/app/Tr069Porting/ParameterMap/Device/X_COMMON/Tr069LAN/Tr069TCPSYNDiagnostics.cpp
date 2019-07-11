#include "Tr069TCPSYNDiagnostics.h"

#include "Tr069Call.h"
#include "Tr069FunctionCall.h"

#include <stdio.h>
#include <stdlib.h>

/****************************
tcpsynDiagnostics    接口暂时做了放空处理

*****************************/

int tr069_tcpsynDiagnostics_getValue(char *name, char *str, unsigned int size)
{
    str[0] = 0;
    return -1;
}

int tr069_tcpsynDiagnostics_setValue(char *name, char *str, unsigned int x)
{
    return -1;
}

////////////////////////////////////////////////////////

static int tr069_TCPSYN_DiagnosticsState_Read(char* value, unsigned int length)
{
    return tr069_tcpsynDiagnostics_getValue((char*)"DEVICE.X_CTC_IPTV.LAN.TCPSYNDiagnostics.DiagnosticsState", value, length);
}

static int tr069_TCPSYN_SuccessCount_Read(char* value, unsigned int length)
{
    return tr069_tcpsynDiagnostics_getValue((char*)"DEVICE.X_CTC_IPTV.LAN.TCPSYNDiagnostics.SuccessCount", value, length);
}

static int tr069_TCPSYN_FailureCount_Read(char* value, unsigned int length)
{
    return tr069_tcpsynDiagnostics_getValue((char*)"DEVICE.X_CTC_IPTV.LAN.TCPSYNDiagnostics.FailureCount", value, length);
}

static int tr069_TCPSYN_AverageResponseTime_Read(char* value, unsigned int length)
{
    return tr069_tcpsynDiagnostics_getValue((char*)"DEVICE.X_CTC_IPTV.LAN.TCPSYNDiagnostics.AverageResponseTime", value, length);
}
static int tr069_TCPSYN_MinimumResponseTime_Read(char* value, unsigned int length)
{
    return tr069_tcpsynDiagnostics_getValue((char*)"DEVICE.X_CTC_IPTV.LAN.TCPSYNDiagnostics.MinimumResponseTime", value, length);
}
static int tr069_TCPSYN_MaximumResponseTime_Read(char* value, unsigned int length)
{
    return tr069_tcpsynDiagnostics_getValue((char*)"DEVICE.X_CTC_IPTV.LAN.TCPSYNDiagnostics.MaximumResponseTime", value, length);
}
static int tr069_TCPSYN_DiagnosticsState_Write(char* value, unsigned int length)
{

    return tr069_tcpsynDiagnostics_setValue((char*)"DEVICE.X_CTC_IPTV.LAN.TCPSYNDiagnostics.DiagnosticsState", value, length);
}

static int tr069_TCPSYN_Host_Read(char* value, unsigned int length)
{
    return tr069_tcpsynDiagnostics_getValue((char*)"DEVICE.X_CTC_IPTV.LAN.TCPSYNDiagnostics.Host", value, length);
}

static int tr069_TCPSYN_Host_Write(char* value, unsigned int length)
{
    return tr069_tcpsynDiagnostics_setValue((char*)"DEVICE.X_CTC_IPTV.LAN.TCPSYNDiagnostics.Host", value, length);
}

static int tr069_TCPSYN_NumberOfRepetitions_Read(char* value, unsigned int length)
{
    return tr069_tcpsynDiagnostics_getValue((char*)"DEVICE.X_CTC_IPTV.LAN.TCPSYNDiagnostics.NumberOfRepetitions", value, length);
}

static int tr069_TCPSYN_NumberOfRepetitions_Write(char* value, unsigned int length)
{
    return tr069_tcpsynDiagnostics_setValue((char*)"DEVICE.X_CTC_IPTV.LAN.TCPSYNDiagnostics.NumberOfRepetitions", value, length);
}

static int tr069_TCPSYN_Timeout_Read(char* value, unsigned int length)
{
    return tr069_tcpsynDiagnostics_getValue((char*)"DEVICE.X_CTC_IPTV.LAN.TCPSYNDiagnostics.Timeout", value, length);
}


static int tr069_TCPSYN_Timeout_Write(char* value, unsigned int length)
{
    return tr069_tcpsynDiagnostics_setValue((char*)"DEVICE.X_CTC_IPTV.LAN.TCPSYNDiagnostics.Timeout", value, length);
}

Tr069TCPSYNDiagnostics::Tr069TCPSYNDiagnostics()
	: Tr069GroupCall("TCPSYNDiagnostics")
{

    Tr069Call* state = new Tr069FunctionCall("DiagnosticsState",        tr069_TCPSYN_DiagnosticsState_Read,       tr069_TCPSYN_DiagnosticsState_Write);
    Tr069Call* host = new Tr069FunctionCall("Host",                     tr069_TCPSYN_Host_Read,                   tr069_TCPSYN_Host_Write);  // ctc 中没有Read
    Tr069Call* num = new Tr069FunctionCall("NumberOfRepetitions",       tr069_TCPSYN_NumberOfRepetitions_Read,    tr069_TCPSYN_NumberOfRepetitions_Write); // ctc 中没有Read
    Tr069Call* timeout = new Tr069FunctionCall("Timeout",               tr069_TCPSYN_Timeout_Read,                tr069_TCPSYN_Timeout_Write); // ctc 中没有Read
    Tr069Call* succ = new Tr069FunctionCall("SuccessCount",             tr069_TCPSYN_SuccessCount_Read,           NULL);
    Tr069Call* fail = new Tr069FunctionCall("FailureCount",             tr069_TCPSYN_FailureCount_Read,           NULL);
    Tr069Call* avg = new Tr069FunctionCall("AverageResponseTime",       tr069_TCPSYN_AverageResponseTime_Read,    NULL);
    Tr069Call* min = new Tr069FunctionCall("MinimumResponseTime",       tr069_TCPSYN_MinimumResponseTime_Read,    NULL);
    Tr069Call* max = new Tr069FunctionCall("MaximumResponseTime",       tr069_TCPSYN_MaximumResponseTime_Read,    NULL);

    regist(state->name(), state);
    regist(host->name(), host);
    regist(num->name(), num);
    regist(timeout->name(), timeout);
    regist(succ->name(), succ);
    regist(fail->name(), fail);
    regist(avg->name(), avg);
    regist(min->name(), min);
    regist(max->name(), max);

}

Tr069TCPSYNDiagnostics::~Tr069TCPSYNDiagnostics()
{
}
