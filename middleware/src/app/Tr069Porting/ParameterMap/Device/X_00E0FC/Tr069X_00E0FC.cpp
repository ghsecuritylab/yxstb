#include "Tr069X_00E0FC.h"

#include "Tr069FunctionCall.h"
#include "TR069Assertions.h"
#include "Tr069AutoOnOffConfiguration.h"
#include "Tr069LogParaConfiguration.h"

#include "record_port.h"

#include "Tr069BandwidthDiagnostics.h"
#include "Tr069PacketCapture.h"

#include "../X_COMMON/Tr069ServiceInfo.h"
#include "../X_COMMON/Tr069PlayDiagnostics.h"
#include "../X_COMMON/Tr069StatisticConfiguration.h"
#include "../X_COMMON/Tr069SQMConfiguration.h"
#include "../X_COMMON/Tr069LAN/Tr069LAN.h"
#include "../X_COMMON/Tr069ServiceStatistics.h"

#include "SysSetting.h"
#include "SettingEnum.h"

#include "mid_sys.h"
#include "AppSetting.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define IND_STRCPY(dest, src)		strcpy(dest, src)

extern Tr069Call* g_cusLan;
extern Tr069Call* g_alarm;

/*------------------------------------------------------------------------------
	STBID是由32位16进制的数字组成，是机顶盒的唯一标识码。
	格式为：AA BBBB CCCCCC DDD E FFFF GGGGGGGGGGGG
	AA		表示整个字符串的校验码，MD5校验
	BBBB	第1、2位运营商编号，后2位为终端类型
	CCCCCC	终端厂商认证编号，暂时由运营商分配
	DDD				终端型号
	E				终端的识别符类型
	FFFF			终端批次
	GGGGGGGGGGGG	终端的识别符
 ------------------------------------------------------------------------------*/
static int tr069_STBID_Read(char* value, unsigned int length)
{
    if(value) {
        mid_sys_serial(value);
        value[32] = 0;
    }
    return 0;
}

static int tr069_PhyMemSize_Read(char* value, unsigned int length)
{
    if(value) {
        sprintf(value, "262144");
    }
    return 0;
}

static int tr069_StorageSize_Read(char* value, unsigned int length)
{
    printf("----tr069_StorageSize_Read---str[%s]--length[%d]---\n", value, length);
    unsigned int totalblock = 0, freeblock = 0;
    int ret = -1;
    if(value) {
        //extern int record_port_disk_size(u_int* ptotalblock, u_int* pfreeblock);
        ret = record_port_disk_size(&totalblock, &freeblock);
        if(!ret)
            sprintf(value, "%u", totalblock * 1024);
        else
            sprintf(value, "0");
    }
    return 0;
}

static int tr069_IsEncryptMark_Read(char* value, unsigned int length)
{
    sprintf(value , "%d", 1);
    return 0;
}

static int tr069_EncryptParameters_Read(char* value, unsigned int length)
{
    char ctcEnPara[] = "Device.X_CTC_IPTV.ServiceInfo.PPPoEPassword," \
        "Device.X_CTC_IPTV.ServiceInfo.Password," \
        "Device.X_CTC_IPTVServiceInfo.DHCPPassword," \
        "Device.X_CTC_IPTV.ServiceInfo.UserPassword";

    char cuEnPara[] = "Device.X_CU_STB.STBInfo.UserPassword," \
        "Device.X_CU_STB.AuthServiceInfo.UserIDPassword";

    char huaweiEnPara[] = "Device.X_00E0FC.ServiceInfo.PPPoEPassword," \
        "Device.X_00E0FC.ServiceInfo.UserIDPassword," \
        "Device.X_00E0FC.ServiceInfo.IPTVPin," \
        "Device.X_00E0FC.PacketCapture.Password," \
        "Device.X_00E0FC.StatisticConfiguration.LogServerUrl," \
        "Device.X_00E0FC.BandwidthDiagnostics.Password,";

    if(value) {
        IND_STRCPY(value, huaweiEnPara);
		int tr069Type = 0;
		sysSettingGetInt("tr069_type", &tr069Type, 0);
        if (2 == tr069Type)
            strcat(value, cuEnPara);
        else if (1 == tr069Type)
            strcat(value, ctcEnPara);
        *(value + 1024) = '\0';
    }
    return 0;
}

static int tr069_HDMIConnect_Read(char* value, unsigned int length)
{
    unsigned int ret = mid_sys_HDMIConnect_status_get();
    sprintf(value , "%u", ret);
    return 0;
}

static int tr069_RecoveryMode_Read(char* value, unsigned int length)
{
    int recoveryMode = -1;
    appSettingGetInt("recoveryMode", &recoveryMode, 0);
    sprintf(value, "%d", recoveryMode);
    return 0;
}

static int tr069_ConnectMode_Read(char* value, unsigned int length)
{
    int netType = 0;

    value[0] = '\0';

    if (!sysSettingGetInt("nettype", &netType, 0)) {
        if (netType == NET_ETH)
            netType = 1;
        else if (netType == NET_WIRELESS)
            netType = 2;
        snprintf(value, length, "%d", netType);
    }
    return 0;
}

Tr069X_00E0FC::Tr069X_00E0FC()
	: Tr069GroupCall("X_00E0FC")
{
	/* 以下对象的注册到表root.Device.X_00E0FC  */
    Tr069Call* call;

    regist(g_tr069StatisticConfiguration->name(), g_tr069StatisticConfiguration);
    regist(g_tr69ServiceInfo->name(), g_tr69ServiceInfo);
    regist(g_tr069PlayDiagnostics->name(), g_tr069PlayDiagnostics);
    regist(g_tr69SQMConfiguration->name(), g_tr69SQMConfiguration);
    regist(g_tr069ServiceStatistics->name(), g_tr069ServiceStatistics);
    regist("LAN", g_cusLan);

    // 注册Tr069GroupCall
    call = new Tr069AutoOnOffConfiguration();
    regist(call->name(), call);

    call = new Tr069LogParaConfiguration();
    regist(call->name(), call);

    call = new Tr069BandwidthDiagnostics();
    regist(call->name(), call);

    call = new Tr069PacketCapture();
    regist(call->name(), call);

	// 注册Tr069FunctionCall

    call  = new Tr069FunctionCall("STBID",                      tr069_STBID_Read,                     NULL);
    regist(call->name(), call);

    call  = new Tr069FunctionCall("PhyMemSize",             tr069_PhyMemSize_Read,                NULL);
    regist(call->name(), call);

    call  = new Tr069FunctionCall("StorageSize",              tr069_StorageSize_Read,               NULL);
    regist(call->name(), call);

    call  = new Tr069FunctionCall("IsEncryptMark",           tr069_IsEncryptMark_Read,             NULL);
    regist(call->name(), call);

    call  = new Tr069FunctionCall("EncryptParameters",         tr069_EncryptParameters_Read,         NULL);
    regist(call->name(), call);

    call  = new Tr069FunctionCall("HDMIConnect",              tr069_HDMIConnect_Read,               NULL);
    regist(call->name(), call);

    call  = new Tr069FunctionCall("RecoveryMode",            tr069_RecoveryMode_Read,              NULL);
    regist(call->name(), call);

    //新添加，原来没有的接口
    call  = new Tr069FunctionCall("ConnectMode",        tr069_ConnectMode_Read,               NULL);
    regist(call->name(), call);
}

Tr069X_00E0FC::~Tr069X_00E0FC()
{
}
