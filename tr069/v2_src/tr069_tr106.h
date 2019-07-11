/*******************************************************************************
    公司：
            Yuxing software
    纪录：
            2008-1-26 21:12:26 create by Liu Jianhua
    模块：
            tr069
    简述：
            tr106 paraments
 *******************************************************************************/

#ifndef __TR069_TR106_H__
#define __TR069_TR106_H__

#include "tr069_diag.h"

#define INTERVAL_CurrentDay_SEC                                     (12 * 60 * 60)
#define INTERVAL_QuarterHour_SEC                                    (15 * 60 * 60)

#define DHCP_OPTIONS_MAX_16                                         4
#define TRACEROUTE_HOPCOUNT_MAX_64                                  64
#define TRACEROUTE_OBJECT_SIZE_5                                    5

#define DEVICE_DeviceInfo_ProvisioningCode_LEN_64                   64

#define DEVICE_ManagementServer_URL_LEN_256                         256
#define DEVICE_ManagementServer_URLBackup_LEN_256                   256
#define DEVICE_ManagementServer_Username_LEN_64                     64
#define DEVICE_ManagementServer_Password_LEN_64                     64
#define DEVICE_ManagementServer_ParameterKey_LEN_32                 32
#define DEVICE_ManagementServer_ConnectionRequestURL_LEN_256        256
#define DEVICE_ManagementServer_ConnectionRequestUsername_LEN_64    64
#define DEVICE_ManagementServer_ConnectionRequestPassword_LEN_64    64
#define DEVICE_ManagementServer_ConnectionRequestPath_LEN_64        64

#define DEVICE_ManagementServer_STUNServerAddress_LEN_64            64
#define DEVICE_ManagementServer_STUNUsername_LEN_64                 64
#define DEVICE_ManagementServer_STUNPassword_LEN_64                 64
#define DEVICE_ManagementServer_UDPConnectionRequestAddress_LEN_64  64


#define DEVICE_Time_NTPServer_LEN_31                                31
#define DEVICE_Time_LocalTimeZone_LEN_64                            64

#define DEVICE_UserInterface_AutoUpdateServer_LEN_256               256
#define DEVICE_UserInterface_CurrentLanguage_LEN_16                 16

#define DEVICE_LAN_AddressingType_LEN_15                            15
#define DEVICE_LAN_IPAddress_LEN_15                                 15
#define DEVICE_LAN_SubnetMask_LEN_15                                15
#define DEVICE_LAN_DefaultGateway_LEN_15                            15
#define DEVICE_LAN_DNSServers_LEN_64                                64

#define DEVICE_LAN_DHCPOption_Value_LEN_64                          64//256

#define DEVICE_LAN_IPPingDiagnostics_DiagnosticsState_LEN_32        32
#define DEVICE_LAN_IPPingDiagnostics_Host_LEN_256                   256

#define DEVICE_LAN_TraceRouteDiagnostics_DiagnosticsState_LEN_32    32
#define DEVICE_LAN_TraceRouteDiagnostics_RouteHops_Host_LEN_256     256
#define DEVICE_LAN_TraceRouteDiagnostics_RouteHops_HOPHOST_LEN_32   32

struct TR106Index {
    int ManagementServer_ConnectionRequestURL_index;
    int ManagementServer_ConnectionRequestPath_index;

    int LAN_AddressingType_index;
    int LAN_IPAddress_index;
    int LAN_SubnetMask_index;
    int LAN_DefaultGateway_index;
    int LAN_DNSServers_index;
    int LAN_MACAddress_index;
};

extern struct TR106Index *g_tr106Index;

void tr106_CurrentDay_ontime(struct TR069 *tr069);
void tr106_QuarterHour_ontime(struct TR069 *tr069);

int tr069_tr106_init(struct TR069 *tr069);
void tr069_tr106_reset(struct TR069 *tr069);
void tr106_tr106_set_routehop(int index, struct RouteHop* routehop);

int tr069_tr106_getString(char *name, char *buf, int size);
unsigned int tr069_tr106_getUnsigned(char *name);
void tr069_tr106_SetParamValue(char *name, char *str, unsigned int val);

#endif //__TR069_TR106_H__
