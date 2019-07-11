#include "JseHWNetDiagnoseTool.h"
#include "JseFunctionCall.h"
#include "JseRoot.h"

#include "config/webpageConfig.h"
#include "JseAssertions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NEW_DIAGNOSE 1
#include "NetworkFunctions.h"
#include "NetworkDiagnose.h"
#include "PhysicalNetDiagnose.h"
#include "AddressDiagnose.h"
#include "GatewayDiagnose.h"
#include "CentralAreaDiagnose.h"
#include "NtpDiagnose.h"
#include "DnsDiagnose.h"
#include "MulticastDiagnose.h"
#include "PingTestDiagnose.h"
#include "NetworkSpeedDiagnose.h"

#include "SysSetting.h"
#include "AppSetting.h"
#include "BrowserBridge/Huawei/BrowserEventQueue.h"
#include "json/json_tokener.h"

//will delete.
static int _ReportEventToEPG(int type, int arg1, void* arg2) //TODO for test old epg. new js interface is "NetDiagnoseTool.GetPINGPacketInfo"
{
    char buff[32] = { 0 };
    char value[4096] = { 0 };
    int status = 0, result = 0, errcode = 0;
    json_object *jo = json_object_new_object();

    NetworkDiagnose::DiagnoseProcess* diag = 0;
    NetworkDiagnose* netdiag = networkDiagnose();

    diag = netdiag->getDiagnose();
    if (!diag || !jo)
        return -1;

    status = netdiag->getTestState();
    result = netdiag->getTestResult();
    errcode = netdiag->getErrorCode();

    if (eTypePingTestDiagnose == type) { 
        PingTestDiagnose* eDiag = static_cast<PingTestDiagnose*>(diag);
        PingPacketInfo_s* info = (PingPacketInfo_s*)arg2;
        if (!info) {
            json_object_put(jo);
            return -1;
        }
        double succrate = eDiag->getSuccessRate();
        json_object_object_add(jo, "type", json_object_new_string("EVENT_NetDiagnose_EVENT"));
        json_object_object_add(jo, "TestType", json_object_new_string("Advanced.PING"));
        if (result != ND_RESULT_SUCCESS)
            json_object_object_add(jo, "Result", json_object_new_string("Fail"));
        else 
            json_object_object_add(jo, "Result", json_object_new_string("Success"));
        json_object_object_add(jo, "TestState", json_object_new_int(status));
        json_object_object_add(jo, "ClientIp", json_object_new_string(info->nHostAddr));
        json_object_object_add(jo, "DataSize", json_object_new_int(info->nPacketSize));
        json_object_object_add(jo, "DelayTime", json_object_new_double(info->nResponsTime));
        json_object_object_add(jo, "TTL", json_object_new_int(info->nRecvTTL));
        json_object_object_add(jo, "SuccessRate", json_object_new_int((int)succrate));
        json_object_object_add(jo, "Progress", json_object_new_int(arg1));
    }

    snprintf(value, 4096, json_object_to_json_string(jo));
    json_object_put(jo);
    LogJseDebug("\n%s\n", value);
    browserEventSend(value, 0);
    return 0;
}

static int JseStbNetDiagnosePageURLRead( const char* param, char* value, int len )
{
    snprintf(value , len, LOCAL_WEBPAGE_PATH_PREFIX"/diagnosticTools/STBcheck_technology.htm");

    return 0;
}

static int JseDiagnoseModeRead( const char* param, char* value, int len )
{
    return 0;
}

static int JseDiagnoseModeWrite( const char* param, char* value, int len )
{
    return 0;
}

static int JseMulticastTestInfoRead( const char* param, char* value, int len )
{
#ifndef NEW_DIAGNOSE
    json_object *json_file_info = json_object_new_object();
    json_object_object_add(json_file_info, "IGMPVer",           json_object_new_int(IGMPVer));
    json_object_object_add(json_file_info, "MulticastIP",       json_object_new_string(multicastIpAdd.c_str()));
    char portBuf[16] = {0};
    snprintf(portBuf, 16, "%d", multicastPort);
    json_object_object_add(json_file_info, "MulticastPort",     json_object_new_string(portBuf));
    json_object_object_add(json_file_info, "MulticastSrcIP",    json_object_new_string(multicastSourceIpAdd.c_str()));
    char *jsonstr_file_info = (char *)json_object_to_json_string(json_file_info);
    snprintf(value, 4096, "%s", jsonstr_file_info);
    LogJseInfo("get multicast test url[%s]\n", value);
    json_object_put(json_file_info);
#endif

    return 0;
}

static int JseMulticastTestInfoWrite( const char* param, char* value, int len )
{
#ifndef NEW_DIAGNOSE
    json_object *json_info = json_tokener_parse(value);
    if (!json_info) {
        LogJseError("json_tokener_parse error\n");
        return 0;
    }

    json_object *json_obj = json_object_object_get(json_info, "IGMPVer");
    if (json_obj)
        IGMPVer = json_object_get_int(json_obj);

    json_obj = json_object_object_get(json_info, "MulticastIP");
    if (json_obj)
        multicastIpAdd = json_object_get_string(json_obj);

    json_obj = json_object_object_get(json_info, "MulticastPort");
    if (json_obj)
        multicastPort = atoi(json_object_get_string(json_obj));

    json_obj = json_object_object_get(json_info, "MulticastSrcIP");
    if (json_obj)
        multicastSourceIpAdd = json_object_get_string(json_obj);

    json_object_put(json_info);
#endif

    return 0;
}

static int JseTestDomainRead( const char* param, char* value, int len )
{
    return 0;
}

static int JseTestDomainWrite( const char* param, char* value, int len )
{
    return 0;
}

static int JseNetworkSpeedTestURLWrite( const char* param, char* value, int len )
{
    return 0;
}

static int JseStartTestWrite( const char* param, char* value, int len )
{
    LogJseDebug("//TODO run here[%s]\n", value);

    NetworkDiagnose* netdiag = networkDiagnose();
    if (!netdiag)
        return -1;
    const char* devname = 0;

    netdiag->setTestType(value);
    if (!strncmp(value, "Basic.PhysicalNet", strlen("Basic.PhysicalNet"))) {
        if (strstr(value, "WiFi"))    
            devname = network_wifi_devname();
        else 
            devname = network_wired_devname();
        NetworkCard* device = networkManager().getDevice(devname); //TODO
        PhysicalNetDiagnose* eDiag = 0;
        eDiag = new PhysicalNetDiagnose();
        eDiag->attach(device);
        netdiag->testStart(eDiag);
    } else if (!strncmp(value, "Basic.Address", strlen("Basic.Address"))) {
        if (strstr(value, "WiFi"))    
            devname = network_wifi_devname();
        else 
            devname = network_wired_devname();
        NetworkInterface* iface = networkManager().getInterface(devname); //TODO
        AddressDiagnose* eDiag = 0;
        eDiag = new AddressDiagnose();
        eDiag->attach(iface);
        netdiag->testStart(eDiag);
    } else if (!strncmp(value, "Basic.Gateway", strlen("Basic.Gateway"))) {
        if (strstr(value, "WiFi"))    
            devname = network_wifi_devname();
        else 
            devname = network_wired_devname();
        NetworkInterface* iface = networkManager().getInterface(devname); //TODO
        GatewayDiagnose* eDiag = 0;
        eDiag = new GatewayDiagnose();
        eDiag->attach(iface);
        netdiag->testStart(eDiag);
    } else if (!strncmp(value, "Basic.CentralArea", strlen("Basic.CentralArea"))) {
        char eds0[256] = { 0 };
        char eds1[256] = { 0 };
        sysSettingGetString("eds",  eds0, 255, 0);
        sysSettingGetString("eds1", eds1, 255, 0);
        CentralAreaDiagnose* eDiag = 0;
        if (strstr(value, "ICMP"))
            eDiag = new CentralAreaDiagnose(ND_MODE_ICMP);
        else
            eDiag = new CentralAreaDiagnose(ND_MODE_SERVICE);
        //eDiag->setUrl(eds0);
        //eDiag->setUrl(eds1);
        eDiag->setUrl("http://blog.csdn.net/");
        eDiag->setUrl("www.baidu.com");
        eDiag->setUrl("https://extensions.gnome.org");
        netdiag->testStart(eDiag);
    } else if (!strncmp(value, "Basic.NTP", strlen("Basic.NTP"))) {
        char ntp0[256] = { 0 };
        char ntp1[256] = { 0 };
        sysSettingGetString("ntp",  ntp0, 255, 0);
        sysSettingGetString("ntp1", ntp1, 255, 0);
        NtpDiagnose* eDiag = 0;
        if (strstr(value, "ICMP"))
            eDiag = new NtpDiagnose(ND_MODE_ICMP);
        else
            eDiag = new NtpDiagnose(ND_MODE_SERVICE);
        //eDiag->setUrl(ntp0);
        //eDiag->setUrl(ntp1);
        eDiag->setUrl("202.112.10.60");
        eDiag->setUrl("202.120.2.101");
        netdiag->testStart(eDiag);
    } else if (!strncmp(value, "Basic.DNS", strlen("Basic.DNS"))) {
        char testurl[256] = { 0 };
        appSettingGetString("netCheckDomain", testurl, 255, 0);
        DnsDiagnose* eDiag = 0;
        eDiag = new DnsDiagnose();
        eDiag->setUrl(testurl);
        netdiag->testStart(eDiag);
    } else if (!strncmp(value, "Basic.Multicast", strlen("Basic.Multicast"))) {
        char addr[256] = {0};
        int port = 0;
        if (strstr(value, "WiFi"))    
            devname = network_wifi_devname();
        else 
            devname = network_wired_devname();
        MulticastDiagnose* eDiag = 0;
        eDiag = new MulticastDiagnose();
        appSettingGetString("netCheckMulticastAdd", addr, 255, 0);
        eDiag->setMultiAddr(addr);
        appSettingGetInt("netCheckMulticastPort", &port, 0);
        eDiag->setMultiPort(port);
        appSettingGetString("netCheckMulticastSourceAdd", addr, 255, 0);
        eDiag->setSourceAddr(addr);
        NetworkInterface* iface = networkManager().getInterface(devname); //TODO
        eDiag->setLocalAddr(iface->getAddress(addr, 255));
        netdiag->testStart(eDiag);
    } else if (!strncmp(value, "Advanced.PingTest", strlen("Advanced.PingTest"))) {
        PingTestDiagnose* eDiag = 0;
        eDiag = new PingTestDiagnose(_ReportEventToEPG);
        eDiag->setHostName("www.baidu.com");
        eDiag->setPacketSize(64); 
        eDiag->setPacketCount(5); 
        eDiag->setTiemoutMs(5000);
        eDiag->setTTL(128);
        netdiag->testStart(eDiag);
    } else if (!strncmp(value, "Advanced.NetworkSpeed", strlen("Advanced.NetworkSpeed"))) {
        NetworkSpeedDiagnose* eDiag = new NetworkSpeedDiagnose();
        eDiag->setTestUrl("http://192.168.2.41/testt.ts");
        eDiag->setSamplePeriod(5);
        eDiag->setAutoStopTime(1 * 60);
        netdiag->testStart(eDiag);
    }
    return 0;
}

static int JseTestResultRead( const char* param, char* value, int len )
{
    char buff[33] = { 0 };
    const char* type = 0;
    int status = 0, result = 0, errcode = 0;
    json_object* jo = 0;

    NetworkDiagnose::DiagnoseProcess* diag = 0;
    NetworkDiagnose* netdiag = networkDiagnose();
    if (!netdiag)
        return -1;
    diag = netdiag->getDiagnose();
    type = netdiag->getTestType();
    if (!strstr(param, type))
        return -1;
    LogJseDebug("Type:%s\n", type);
    
    jo = json_object_new_object();
    if (!jo)
        return -1;

    status = netdiag->getTestState();
    result = netdiag->getTestResult();
    errcode = netdiag->getErrorCode();

    json_object_object_add(jo, "TestState", json_object_new_int(status));

    if (status != ND_STATE_RUNNING)
        json_object_object_add(jo, "TestResult", json_object_new_int(result));
    if (result != ND_RESULT_SUCCESS)
        json_object_object_add(jo, "ErrorCode", json_object_new_int(errcode));

    if (!diag)
        goto END;
    if (!strncmp(type, "Advanced.PingTest", strlen("Advanced.PingTest"))) {
        PingTestDiagnose* eDiag = static_cast<PingTestDiagnose*>(diag);
        int sendcnt = eDiag->getPacketSendCount();
        int recvcnt = eDiag->getPacketRecvCount();
        double succrate = eDiag->getSuccessRate();
        double mindelay = eDiag->getMinDelay();
        double maxdelay = eDiag->getMaxDelay();
        double avedelay = eDiag->getAverageDelay();
        json_object_object_add(jo, "HostAddress", json_object_new_string(eDiag->getHostAddr()));
        json_object_object_add(jo, "PacketSent", json_object_new_int(sendcnt));
        json_object_object_add(jo, "PacketReceived", json_object_new_int(recvcnt));
        json_object_object_add(jo, "PacketTimeout", json_object_new_int(eDiag->getPacketTimeoutCount())); 
        json_object_object_add(jo, "PacketError", json_object_new_int(eDiag->getPacketErrorCount()));
        if (sendcnt > 0) {
            snprintf(buff, 32, "%.1f", succrate);
            json_object_object_add(jo, "SuccessRate", json_object_new_string(buff));
        }
        if (recvcnt > 0) {
            snprintf(buff, 32, "%.1f", mindelay);
            json_object_object_add(jo, "MinimumTime", json_object_new_string(buff));
            snprintf(buff, 32, "%.1f", maxdelay);
            json_object_object_add(jo, "MaxTime", json_object_new_string(buff));
            snprintf(buff, 32, "%.1f", avedelay);
            json_object_object_add(jo, "AvarageTime", json_object_new_string(buff));
        }
    } else if (!strncmp(type, "Advanced.NetworkSpeed", strlen("Advanced.NetworkSpeed"))) {
        NetworkSpeedDiagnose* eDiag = static_cast<NetworkSpeedDiagnose*>(diag);
        double aveSpeed = eDiag->getAverageSpeed();
        double minSpeed = eDiag->getMinSpeed();
        double maxSpeed = eDiag->getMaxSpeed();
        snprintf(buff, 32, "%.2f", aveSpeed/1000000);
        json_object_object_add(jo, "AverageSpeed", json_object_new_string(buff));
        snprintf(buff, 32, "%.2f", minSpeed/1000000);
        json_object_object_add(jo, "MinSpeed", json_object_new_string(buff));
        snprintf(buff, 32, "%.2f", maxSpeed/1000000);
        json_object_object_add(jo, "MaxSpeed", json_object_new_string(buff));
    }
END:
    snprintf(value, 4096, json_object_to_json_string(jo));
    json_object_put(jo);
    LogJseDebug("\n%s\n", value);
    return 0;
}

static int JseStopTestWrite( const char* param, char* value, int len )
{
    NetworkDiagnose* netdiag = networkDiagnose();
    if (!netdiag)
        return -1;
    netdiag->testStop();
    return 0;
}

static int JseExitTestWrite( const char* param, char* value, int len )
{
    return 0;
}

static int JsePingTestConfigWrite( const char* param, char* value, int len )
{
#ifndef NEW_DIAGNOSE
    json_object *json_info = json_tokener_parse(value);
    if (!json_info) {
        LogJseError("json_tokener_parse error\n");
        return 0;
    }

    json_object *json_obj = json_object_object_get(json_info, "Url");
    if (json_obj)
        pingTestUrl = json_object_get_string(json_obj);
    json_obj = json_object_object_get(json_info, "DataSize");
    if (json_obj)
        pingTestDataSize = json_object_get_int(json_obj);
    json_obj = json_object_object_get(json_info, "TimeOut");
    if (json_obj)
        pingTestTimeOut = json_object_get_int(json_obj);
    json_obj = json_object_object_get(json_info, "Count");
    if (json_obj)
        pingTestCount = json_object_get_int(json_obj);
    json_obj = json_object_object_get(json_info, "TTL");
    if (json_obj)
        pingTestTTL = json_object_get_int(json_obj);

    json_object_put(json_info);
#endif

    return 0;
}

JseHWNetDiagnoseTool::JseHWNetDiagnoseTool()
    : JseGroupCall("NetDiagnoseTool")
{
    JseCall* call;

    call = new JseFunctionCall("DiagnoseMode", JseDiagnoseModeRead, JseDiagnoseModeWrite);
    regist(call->name(), call);

    call = new JseFunctionCall("TestMulticast", JseMulticastTestInfoRead, JseMulticastTestInfoWrite);
    regist(call->name(), call);
    
    call = new JseFunctionCall("TestDomain", JseTestDomainRead, JseTestDomainWrite);
    regist(call->name(), call);

    call = new JseFunctionCall("NetworkSpeedTestURL", NULL, JseNetworkSpeedTestURLWrite);
    regist(call->name(), call);

    call = new JseFunctionCall("TestStart", NULL, JseStartTestWrite);
    regist(call->name(), call);
    
    call = new JseFunctionCall("GetTestResult", JseTestResultRead, NULL);
    regist(call->name(), call);
    
    call = new JseFunctionCall("TestStop", NULL, JseStopTestWrite);
    regist(call->name(), call);
    
    call = new JseFunctionCall("ExitNotify", NULL, JseExitTestWrite);
    regist(call->name(), call);

    call = new JseFunctionCall("PingTestConfig", NULL, JsePingTestConfigWrite);
    regist(call->name(), call);

}

JseHWNetDiagnoseTool::~JseHWNetDiagnoseTool()
{
}


int JseHWNetDiagnoseToolInit()
{
    JseCall* call;

    call = new JseFunctionCall("stbNetDiagnosePageURL", JseStbNetDiagnosePageURLRead, NULL);
    JseRootRegist(call->name(), call);

    call = new JseHWNetDiagnoseTool();
    JseRootRegist(call->name(), call);
    
    return 0;
}

