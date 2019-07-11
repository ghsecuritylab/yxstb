#include "Tr069X_CU_STB.h"

#include "Tr069CUSTBInfo.h"
#include "Alarm/Tr069CUAlarm.h"
#include "../X_COMMON/Tr069ServiceInfo.h"
#include "../X_COMMON/Tr069StatisticConfiguration.h"
#include "../X_COMMON/Tr069ServiceStatistics.h"

Tr069X_CU_STB::Tr069X_CU_STB()
	: Tr069GroupCall("X_CU_STB")
{    
	/* ���¶����ע�ᵽ��root.Device.X_CU_STB  */
    Tr069Call* info = new Tr069CUSTBInfo();
    regist(info->name(), info);
	
    Tr069Call* alrm = new Tr069CUAlarm();
    regist(alrm->name(), alrm);
	
	// Tr069Call* refun1 = new Tr069CUStatisticConfig();
    regist(g_tr069StatisticConfiguration->name(), g_tr069StatisticConfiguration);

	// Tr069Call* refun2 = new Tr069CUServiceInfo();
	// ����CU���ַ�����AuthServiceInfo����CTC��HW����ServiceInfo����ͳһ������ֶ�����ַ���
    regist("AuthServiceInfo", g_tr69ServiceInfo);     

    regist(g_tr069ServiceStatistics->name(), g_tr069ServiceStatistics);    


}

Tr069X_CU_STB::~Tr069X_CU_STB()
{
}
