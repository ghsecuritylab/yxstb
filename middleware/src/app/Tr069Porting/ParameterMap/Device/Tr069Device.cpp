
#include "Tr069Device.h"

#include "Tr069ManagementServer.h"
#include "Tr069STBService.h"
#include "Tr069Config.h"
#include "Tr069GatewayInfo.h"
#include "Tr069Time.h"
#include "LAN/Tr069LAN1.h"
#include "UserInterface/Tr069UserInterface.h"
#include "X_CTC_IPTV/Tr069X_CTC_IPTV.h"
#include "X_CU_STB/Tr069X_CU_STB.h"
#include "X_00E0FC/Tr069X_00E0FC.h"
#include "Tr069Qos.h"
#include "Tr069PmInfo.h"
#include "Tr069WLAN.h"
#include "STBDevice/Tr069STBDevice.h"
#include "DeviceInfo/Tr069DeviceInfo.h"

/*************************************************
Description: 初始化Tr069定义的接口,这里是root.Device表
Input: 无
Return: 无
 *************************************************/
Tr069Device::Tr069Device()
	: Tr069GroupCall("Device")
{
	/* 以下对象的注册到表root.Device */
	// Tr069GroupCall类对象
    Tr069Call* regr1 = new Tr069X_CTC_IPTV();
    regist(regr1->name(), regr1);

    Tr069Call* regr2 = new Tr069X_CU_STB();
    regist(regr2->name(), regr2);	

    Tr069Call* regr3 = new Tr069X_00E0FC();
    regist(regr3->name(), regr3);

#if defined(HUAWEI_C10)
    Tr069Call* gateway = new Tr069GatewayInfo();
    regist(gateway->name(), gateway);
#endif // HUAWEI_C10

    Tr069Call* config = new Tr069Config();
    regist(config->name(), config);

    Tr069Call* lan1 = new Tr069LAN1();
    regist(lan1->name(), lan1);

    Tr069Call* time = new Tr069Time();
    regist(time->name(), time);  
	
    Tr069Call* interf = new Tr069UserInterface();
    regist(interf->name(), interf);  
    
	// Tr069FunctionCall类对象
    Tr069Call* refun1 = new Tr069STBService();
    regist(refun1->name(), refun1);

    Tr069Call* management = new Tr069ManagementServer();
    regist(management->name(), management);    

    Tr069Call* Qos = new Tr069Qos();
    regist(Qos->name(), Qos);    
    
    Tr069Call* PmInfo = new Tr069PmInfo();
    regist(PmInfo->name(), PmInfo);    
    
    Tr069Call* WLAN = new Tr069WLAN();
    regist(WLAN->name(), WLAN);    
 
    Tr069Call* STBDevice = new Tr069STBDevice();
    regist(STBDevice->name(), STBDevice);       
    
    Tr069Call* DeviceInfo = new Tr069DeviceInfo();
    regist(DeviceInfo->name(), DeviceInfo);       
}

Tr069Device::~Tr069Device()
{
}

