#include "Tr069X_CTC_IPTV.h"

#include "Tr069FunctionCall.h"
#include "Tr069LogMsg.h"
#include "Tr069IMSInfo.h"
#include "TR069Assertions.h"
#include "../X_COMMON/Tr069ServiceInfo.h"
#include "../X_COMMON/Tr069PlayDiagnostics.h"
#include "../X_COMMON/Tr069StatisticConfiguration.h"
#include "../X_COMMON/Tr069SQMConfiguration.h"
#include "../X_COMMON/Tr069LAN/Tr069LAN.h"
#include "../X_COMMON/Tr069ServiceStatistics.h"
#include "Tr069Monitor.h"
#include "Tr069UserAccount.h"

#include "mid_sys.h"
#include "record_port.h"

#include <string.h>
#include <stdlib.h>

extern Tr069Call* g_cusLan;

static int tr069_STBID_Read(char* value, unsigned int length)
{
    if(value != NULL)
        mid_sys_serial(value);
    return 0;
}

static int tr069_PhyMemSize_Read(char* value, unsigned int length)
{
    if(value) {
        sprintf(value, "262144");
    }
    return 0;
}

/*------------------------------------------------------------------------------
	存储大小，单位KB
 ------------------------------------------------------------------------------*/

static int tr069_StorageSize_Read(char* value, unsigned int length)
{
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

Tr069X_CTC_IPTV::Tr069X_CTC_IPTV()
	: Tr069GroupCall("X_CTC_IPTV")
{
	/* 以下对象的注册到表root.Device.X_CTC_IPTV  */
	//Tr069Call* refun1 = new Tr069CTCStatisticConfig();
    regist(g_tr069StatisticConfiguration->name(), g_tr069StatisticConfiguration);

    Tr069Call* refun2 = new Tr069LogMsg();
    regist(refun2->name(), refun2);

    Tr069Call* refun3 = new Tr069IMSInfo();
    regist(refun3->name(), refun3);

	//Tr069Call* refun4 = new Tr069CTCServiceInfo();
    regist(g_tr69ServiceInfo->name(), g_tr69ServiceInfo);

	//Tr069Call* refun5 = new Tr069CTCPlayDiagnostics();
    regist(g_tr069PlayDiagnostics->name(), g_tr069PlayDiagnostics);

	//Tr069Call* refun6 = new Tr069CTCSQMConfiguration();
    regist(g_tr69SQMConfiguration->name(), g_tr69SQMConfiguration);

    regist( "LAN", g_cusLan);

    Tr069Call* UserAccount = new Tr069UserAccount();
    regist(UserAccount->name(), UserAccount);

    Tr069Call* Monitor = new Tr069Monitor();
    regist(Monitor->name(), Monitor);

    regist(g_tr069ServiceStatistics->name(), g_tr069ServiceStatistics);

    //regist functions
    Tr069Call* id  = new Tr069FunctionCall("STBID",                      tr069_STBID_Read,                 NULL);
    Tr069Call* phymem  = new Tr069FunctionCall("PhyMemSize",             tr069_PhyMemSize_Read,            NULL);
    Tr069Call* stor  = new Tr069FunctionCall("StorageSize",              tr069_StorageSize_Read,           NULL);

    regist(id->name(), id);
    regist(phymem->name(), phymem);
    regist(stor->name(), stor);
}

Tr069X_CTC_IPTV::~Tr069X_CTC_IPTV()
{
}

