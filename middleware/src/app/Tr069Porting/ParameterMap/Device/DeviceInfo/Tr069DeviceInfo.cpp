#include "Tr069DeviceInfo.h"

#include "Tr069FunctionCall.h"
#include "Tr069X_CTC_IPTV_Monitor.h"

#include "X_CTC_IPTV_Alarm/Tr069X_CTC_IPTV_Alarm.h"
#include "tr069_port.h"
#include "tr069_port1.h"
#include "mid_sys.h"
#include "SysSetting.h"
#include "VersionSetting.h"
#include "mid/mid_time.h"
#include "TR069Assertions.h"
#include "stbinfo/stbinfo.h"

#include "customer.h"

#include <string.h>

/*************************************************
Description: ��ʼ����ע��tr069����Ľӿ� <Device.DeviceInfo.***>
Input: ��
Return: ��
 *************************************************/

#ifdef HUAWEI_C10
char g_tr069_status[16] = "Up";
//��ʱû�б����ö��ɾ�̬�����Ա��⺯������
static void tr069_port_set_DeviceStatus(const char * status)
{
	strncpy(g_tr069_status, status, 16);
}

static int getTr069PortModelID(char* str, unsigned int val)
{
    mid_sys_serial(str);

    return 0;
}
#endif
static int getTr069PortConfig(char* str, unsigned int val)
{
    strncpy(str, "yx_config_system.ini", val);

    return 0;
}

static int getTr069PortEnabledOptions(char* str, unsigned int val)
{
    strncpy(str, "option", val);

    return 0;
}

/*------------------------------------------------------------------------------
	�����������̣�����ɶ����ַ�������
 ------------------------------------------------------------------------------*/
static int getTr069PortManufacturer(char* str, unsigned int val)
{
#if defined(SHANGHAI_HD) || defined(SHANGHAI_SD)
    strncpy(str, "Hybroad", val);
#else
    strncpy(str, "Huawei", val);
#endif

    return 0;
}

/*------------------------------------------------------------------------------
	�豸��������֯��ϵΨһ��ʶ����ʾΪ��λʮ��������ֵ��ȫ�����ô�д��ĸ����������
	ǰ���㡣
 ------------------------------------------------------------------------------*/
static int getTr069PortManufacturerOUI(char* str, unsigned int val)
{
#if  defined(Sichuan)
    strncpy(str, "990060", val);
#elif defined(SHANGHAI_HD) || defined(SHANGHAI_SD)
    strncpy(str, "990030", val);
#elif defined(NEIMENGGU_HD)
    strncpy(str, "000008", val);
#else
    strncpy(str, "00E0FC", val);
#endif

    return 0;
}

/*------------------------------------------------------------------------------
	�������ͺ����ƣ�����ɶ����ַ�������
 ------------------------------------------------------------------------------*/
static int getTr069PortModelName(char* str, unsigned int val)
{
    char StbType[32] = {0};

#if defined(SHANGHAI_HD) || defined(SHANGHAI_SD)
    strncpy(str, "Hybroad ", val);
#else
    strncpy(str, "Huawei ", val);
#endif
    snprintf(StbType, sizeof(StbType), "%s", StbInfo::STB::UpgradeModel());
    strcat(str, StbType);

    return 0 ;
}

/*------------------------------------------------------------------------------
	������ �豸��ȫ����������ɶ����ַ�������
 ------------------------------------------------------------------------------*/
static int getTr069PortDescription(char* str, unsigned int val)
{
#if defined(SHANGHAI_HD) || defined(SHANGHAI_SD)
    strncpy(str, "HybroadIPSTB", val);
#else
    strncpy(str, "HuaweiIPSTB", val);
#endif

    return 0;
}

/*------------------------------------------------------------------------------
	ʹ�����кŵĲ�Ʒ����ʶ����������ָ���������̣��˲������ڱ�ʶ��Ʒ���Ʒ���
	���� SerialNumber ������Ψһ�ġ�
 ------------------------------------------------------------------------------*/
static int getTr069PortProductClass(char* str, unsigned int val)
{
    strncpy(str, (char *)(StbInfo::STB::UpgradeModel()), val);

    return 0;
}

/*------------------------------------------------------------------------------
	������ ���к�
 ------------------------------------------------------------------------------*/
static int getTr069PortSerialNumber(char* str, unsigned int val)
{
    memset(str, 0, val);
    mid_sys_serial(str);

    return 0;
}

/*------------------------------------------------------------------------------
	��ʶ�ض��������ͺźͰ汾���ַ�����
 ------------------------------------------------------------------------------*/
static int getTr069PortHardwareVersion(char* str, unsigned int val)
{
    HardwareVersion(str, val);

    return 0;
}

/*------------------------------------------------------------------------------
	�������Զ���汾��������ʽ��
	��ϵͳ.LOGO.����.����(0)
 ------------------------------------------------------------------------------*/
static int getTr069PortSoftwareVersion(char* str, unsigned int val)
{
    char *p;
    get_upgrade_version(str);

    p = strchr(str,'.');
    if ( p )
        *p = '\0';

    return 0;
}

/*------------------------------------------------------------------------------
	�κθ��Ӱ汾�Զ��ŷָ����б���ʾ�κι�Ӧ��ϣ���ṩ�ĸ���Ӳ���汾��Ϣ��
	���û�п���Ϊ���ַ�����
 ------------------------------------------------------------------------------*/
static int getTr069PortAdditionalHardwareVersion(char* str, unsigned int val)
{
    strncpy(str, "7100", val);

    return 0;
}

/*------------------------------------------------------------------------------
	�κθ��Ӱ汾�Զ��ŷָ����б���ʾ�κι�Ӧ��ϣ���ṩ�ĸ�������汾��Ϣ��
	���û�п���Ϊ���ַ�����
 ------------------------------------------------------------------------------*/
static int getTr069PortAdditionalSoftwareVersion(char* str, unsigned int val)
{
    strncpy(str, StbInfo::STB::Version::HWVersion(), val);

    return 0;
}

/*------------------------------------------------------------------------------
	�����е�ǰ��ѡ״̬�� ��Up�� ��Initializing�� ��Error�� ��Disabled��
 ------------------------------------------------------------------------------*/
static int getTr069PortDeviceStatus(char* str, unsigned int val)
{
    strcpy(str, g_tr069_status);
    return 0;
}

/*------------------------------------------------------------------------------
	�ӻ��������һ���������ʱ�䣨����ƣ���
 ------------------------------------------------------------------------------*/
static int getTr069PortUpTime(char* str, unsigned int val)
{
    long long clk = mid_clock( );
    snprintf(str, val, "%d", (unsigned int)(clk / 1000));

    return 0;
}

/*------------------------------------------------------------------------------
	�ն˼�¼����־��Ϣ
 ------------------------------------------------------------------------------*/
static int getTr069PortDeviceLog(char* str, unsigned int val)
{
    strncpy(str, "Log", val);

    return 0;
}


Tr069DeviceInfo::Tr069DeviceInfo()
	: Tr069GroupCall("DeviceInfo")
{
    Tr069Call* X_CTC_IPTV_Alarm      = new Tr069X_CTC_IPTV_Alarm();
    Tr069Call* X_CTC_IPTV_Monitor   = new Tr069X_CTC_IPTV_Monitor();

    regist(X_CTC_IPTV_Alarm->name(), X_CTC_IPTV_Alarm);
    regist(X_CTC_IPTV_Monitor->name(), X_CTC_IPTV_Monitor);

#ifdef HUAWEI_C10

#ifdef NEIMENGGU_HD
    Tr069Call* ModelID            = new Tr069FunctionCall("ModelID", getTr069PortModelID, NULL);
    Tr069Call* EnabledOptions     = new Tr069FunctionCall("EnabledOptions", getTr069PortEnabledOptions, NULL);
    regist(ModelID->name(), ModelID);
	regist(EnabledOptions->name(), EnabledOptions);
#endif

	Tr069Call* ConfigFileVersion  = new Tr069FunctionCall("ConfigFileVersion", getTr069PortConfig, NULL);
    regist(ConfigFileVersion->name(), ConfigFileVersion);

#endif



    Tr069Call* Manufacturer              = new Tr069FunctionCall("Manufacturer", getTr069PortManufacturer, NULL);
    Tr069Call* ManufacturerOUI           = new Tr069FunctionCall("ManufacturerOUI", getTr069PortManufacturerOUI, NULL);
    Tr069Call* ModelName                 = new Tr069FunctionCall("ModelName", getTr069PortModelName, NULL);
    Tr069Call* Description               = new Tr069FunctionCall("Description", getTr069PortDescription, NULL);
    Tr069Call* ProductClass              = new Tr069FunctionCall("ProductClass", getTr069PortProductClass, NULL);
    Tr069Call* SerialNumber              = new Tr069FunctionCall("SerialNumber", getTr069PortSerialNumber, NULL);
    Tr069Call* HardwareVersion           = new Tr069FunctionCall("HardwareVersion", getTr069PortHardwareVersion, NULL);
    Tr069Call* SoftwareVersion           = new Tr069FunctionCall("SoftwareVersion", getTr069PortSoftwareVersion, NULL);
    Tr069Call* AdditionalHardwareVersion = new Tr069FunctionCall("AdditionalHardwareVersion", getTr069PortAdditionalHardwareVersion, NULL);
    Tr069Call* AdditionalSoftwareVersion = new Tr069FunctionCall("AdditionalSoftwareVersion", getTr069PortAdditionalSoftwareVersion, NULL);
//    Tr069Call* ProvisioningCode          = new Tr069FunctionCall("ProvisioningCode", getTr069PortModelID, NULL);
    Tr069Call* DeviceStatus              = new Tr069FunctionCall("DeviceStatus", getTr069PortDeviceStatus, NULL);
    Tr069Call* UpTime                    = new Tr069FunctionCall("UpTime", getTr069PortUpTime, NULL);
//    Tr069Call* FirstUseDate              = new Tr069FunctionCall("FirstUseDate", getTr069PortModelID, NULL);
    Tr069Call* DeviceLog                 = new Tr069FunctionCall("DeviceLog", getTr069PortDeviceLog, NULL);

    regist(Manufacturer->name(), Manufacturer);
    regist(ManufacturerOUI->name(), ManufacturerOUI);
    regist(ModelName->name(), ModelName);
    regist(Description->name(), Description);
    regist(ProductClass->name(), ProductClass);
    regist(SerialNumber->name(), SerialNumber);
    regist(HardwareVersion->name(), HardwareVersion);
    regist(SoftwareVersion->name(), SoftwareVersion);
    regist(AdditionalHardwareVersion->name(), AdditionalHardwareVersion);
    regist(AdditionalSoftwareVersion->name(), AdditionalSoftwareVersion);
//    regist(EnabledOptions->name(), EnabledOptions);
    regist(DeviceStatus->name(), DeviceStatus);
    regist(UpTime->name(), UpTime);
//   regist(EnabledOptions->name(), EnabledOptions);
    regist(DeviceLog->name(), DeviceLog);




}

Tr069DeviceInfo::~Tr069DeviceInfo()
{
}
