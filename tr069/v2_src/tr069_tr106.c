/*******************************************************************************
    ��˾��
            Yuxing software
    ��¼��
            2008-1-26 21:12:26 create by Liu Jianhua
    ģ�飺
            tr069
    ������
            tr106 paraments
 *******************************************************************************/

#include "tr069_header.h"

int parameterRemain = 0;
int SH_Inform_Enable = 0;

static struct if_stats {
    int    begintime;
    u_long    if_ipackets;    /* packets received on interface */
    u_long    if_opackets;    /* packets sent on interface */
    u_long    if_ibytes;        /* total number of octets received */
    u_long    if_obytes;        /* total number of octets sent */
} g_currentday, g_quarterhour;

struct TR106Param
{
    struct {
        /*��Ҫ�����ṩ�̺�������Ӧ��Ϣ�ı�ʶ������������ȷ�������ṩ���ض��Ķ��ƺ�
        ��Ӧ�������������ı�ʱ���������ϱ��ն˹���ϵͳ��*/
        char ProvisioningCode[DEVICE_DeviceInfo_ProvisioningCode_LEN_64 + 4];
        /*������ �״γɹ������������ӵ����ں�ʱ�䡣*/
        DateTime FirstUseDate;
    } DeviceInfo;

    struct {
        /*���������û�����WAN ����Э�������ն˹���ϵͳ��URL���˲�������Ϊ��Ч�� 
            HTTP �� HTTPS URL [8].��HTTPS URL ��ʾ�ն˹���ϵͳ֧�� SSL��������
            ʹ�ô�URL �ġ����������֣��Ա���ʹ�û�����֤�ļ�Ȩʱȷ�� �ն˹���ϵͳ
            ��֤���������ı�ʱ���������ϱ��ն˹���ϵͳ��*/
        char URL[DEVICE_ManagementServer_URL_LEN_256 + 4];
        /*���������û�����WAN ����Э�������ն˹���ϵͳ�ı���URL���˲�������Ϊ��Ч�� 
            HTTP �� HTTPS URL [8].��HTTPS URL ��ʾ�ն˹���ϵͳ֧�� SSL��������
            ʹ�ô�URL �ġ����������֣��Ա���ʹ�û�����֤�ļ�Ȩʱȷ�� �ն˹���ϵͳ
            ��֤���������ı�ʱ���������ϱ��ն˹���ϵͳ��*/
        char URLBackup[DEVICE_ManagementServer_URLBackup_LEN_256 + 4];
        /*�����е�¼�ն˹����������ʹ�õ��û��������û���ֻ�ڻ���HTTP����֤ʱʹ�á�
            �����ʽΪ��<OUI>��-��<ProductClass>��-��<SerialNumber>������<OUI>��<ProductClass>��
            <SerialNumber>������Inform��Ϣ������DeviceIdStruct�ṹ�����ݱ���һ��*/
        char Username[DEVICE_ManagementServer_Username_LEN_64 + 4];
        /*��ʹ�û�����WAN ����Э�齨�����ն˹���ϵͳ������ʱʹ�õļ�Ȩ���롣ֻ��
            �������в�ʹ�ÿͻ��˻�����֤�ļ�Ȩʱ���Ż��õ������롣�ڱ���ȡʱ����
            ���������ʵ��ֵ������һ�����ַ���������ͳһ����ķ�ʽ��Ĭ��ֵΪ
            ��STBAdmin�����Ҵ�Сд����*/
        char Password[DEVICE_ManagementServer_Password_LEN_64 + 4];
        /*URLModifyFlag��һ������ֵ��ÿһλ��ʾ�Ƿ������޸ģ�˳��ӵ�λ����λΪ
          EPG��TMC������ҳ�桢STBManageTool���ߣ�ǰ��28λ������Ŀǰ��0*/
        int  URLModifyFlag;
        /*�������Ƿ�������� Inform �������ö�ʱ��������˷��ͻ�������Ϣ��*/
        Boolean PeriodicInformEnable;
        /*��PeriodicInformEnable Ϊ��ʱ�������� ���볢������ �ն˹���ϵͳ ������
            Inform������ʱ������*/
        unsigned int PeriodicInformInterval;
        /*���������з���Inform�������õ�ʱ��ο�����ֵ��ÿ��Inform���ñ�����
            PeriodicInformInterval�ӻ���˲ο�ʱ���������ʱ���С�*/
        DateTime PeriodicInformTime;
        /*�����ڷ����������µ�SetParameterValues��AddObject��DeleteObject��������
            ��ParameterKey�Ա�����ֵ�����û�д�����ã�����ֵΪ�ա�*/
        char ParameterKey[DEVICE_ManagementServer_ParameterKey_LEN_32 + 4];
        /*�ն˹���ϵͳ ������з�����������֪ͨʱ��ʹ�õ� HTTP URL������ʽ���£�
            http://host:port/path.URL �ġ����������ֿ����ǻ����й���ӿڵ� IP ��ַ��
            �������������*/
        char ConnectionRequestURL[DEVICE_ManagementServer_ConnectionRequestURL_LEN_256 + 4];
        /*�ն˹���ϵͳ ������з�����������ʱ����Ȩ����ʹ�õ��û�����Ĭ��ֵΪ��STBAdmin��*/
        char ConnectionRequestUsername[DEVICE_ManagementServer_ConnectionRequestUsername_LEN_64 + 4];
        /*�ն˹���ϵͳ ������з�����������ʱ����Ȩ����ʹ�õ����롣�ڱ���ȡʱ��
            �ò��������ʵ��ֵ������һ�����ַ�����Ĭ��ֵΪ��STBAdmin��*/
        char ConnectionRequestPassword[DEVICE_ManagementServer_ConnectionRequestPassword_LEN_64 + 4];

        /*�ն˹���ϵͳ ������з�����������֪ͨʱ��ʹ�õ� HTTP URL������ʽ���£�
            http://host:port/path.URL �ġ����������ֿ����ǻ����й���ӿڵ� IP ��ַ��
            �������������*/
        char ConnectionRequestPath[DEVICE_ManagementServer_ConnectionRequestPath_LEN_64 + 4];

        Boolean STUNEnable;
        char STUNServerAddress[DEVICE_ManagementServer_STUNServerAddress_LEN_64 + 4];
        unsigned int STUNServerPort;
        char STUNUsername[DEVICE_ManagementServer_STUNUsername_LEN_64 + 4];
        char STUNPassword[DEVICE_ManagementServer_STUNPassword_LEN_64 + 4];
        Boolean NATDetected;
        char UDPConnectionRequestAddress[DEVICE_ManagementServer_UDPConnectionRequestAddress_LEN_64 + 4];
        unsigned int UDPConnectionRequestAddressNotificationLimit;
        unsigned int STUNMaximumKeepAlivePeriod;
        unsigned int STUNMinimumKeepAlivePeriod;

    } ManagementServer;

    struct {
        struct TR106_DHCPOption {
            /*������Ǳ�ʾ��DHCP�����������󣬻�DHCP�ͻ��˷���ֵ��*/
            Boolean Request;
            /*RFC 2132�ж����DHCP option��ǩ*/
            unsigned int Tag;
            /*�������Ϊfalseʱ�����ڱ�ʶDHCP option��base64����İ˽����ַ�����*/
            Base64 Value[DEVICE_LAN_DHCPOption_Value_LEN_64 + 4];
        } DHCPOption[DHCP_OPTIONS_MAX_16];

        struct {
            /*��ʾ������ݵ����������ն˹���ϵͳҪ���������ֵ��������ǣ�
                "None"��"Requested"��"Complete"��"Error_CannotResolveHostName"��
                "Error_Internal"��"Error_Other"*/
            char DiagnosticsState[DEVICE_LAN_IPPingDiagnostics_DiagnosticsState_LEN_32 + 4];
            /*����ping��ϵ����������ַ��*/
            char Host[DEVICE_LAN_IPPingDiagnostics_Host_LEN_256 + 4];
            /*�ڱ�����֮ǰ��ping����ظ��Ĵ�����*/
            unsigned int NumberOfRepetitions;
            /*�ú����ʾ��ping��ϳ�ʱʱ��*/
            unsigned int Timeout;
            /*ÿ��ping����͵����ݴ�С�����ֽ�Ϊ��λ��Ҫ��̶���СΪ32�ֽ�*/
            unsigned int DataBlockSize;
            /*���԰�������DiffServ����㣬Ĭ��ֵΪ0*/
            unsigned int DSCP;
            /*�������ping�����гɹ��Ĵ�����*/
            unsigned int SuccessCount;
            /*�������ping������ʧ�ܵĴ���*/
            unsigned int FailureCount;
            /*�Ժ���Ϊ��λ�����һ��ping�������гɹ���Ӧ��ƽ��ʱ�䡣*/
            unsigned int AverageResponseTime;
            /*�Ժ���Ϊ��λ�����һ��ping�������гɹ���Ӧ�е����ʱ��*/
            unsigned int MinimumResponseTime;
            /*�Ժ���Ϊ��λ�����һ��ping�������гɹ���Ӧ�е��ʱ��*/
            unsigned int MaximumResponseTime;
        } IPPingDiagnostics;

        struct {
            /*��ʾ������ݵ����������ն˹���ϵͳҪ���������ֵ��������ǣ�
                "None"��"Requested"��"Complete"��"Error_CannotResolveHostName"��
                "Error_Internal"��"Error_Other"*/
            char DiagnosticsState[DEVICE_LAN_TraceRouteDiagnostics_DiagnosticsState_LEN_32 + 4];
            /*����·����ϵ����������ַ��*/
            char Host[DEVICE_LAN_TraceRouteDiagnostics_RouteHops_Host_LEN_256 + 4];
            /*�ú����ʾ��·����ϳ�ʱʱ��*/
            unsigned int Timeout;
            /*ÿ��·����Ϸ��͵����ݴ�С�����ֽ�Ϊ��λ��Ҫ��̶���СΪ32�ֽ�*/
            unsigned int DataBlockSize;
            /*���͵Ĳ������ݰ����������(���TTL��)��Ĭ��Ϊ30��*/
            unsigned int MaxHopCount;
            /*���԰�������DiffServ����㣬Ĭ��ֵΪ0.*/
            unsigned int DSCP;
            /*�Ժ����ʾ�����һ��·���������Ե���Ӧʱ�䣬����޷���������·�ɣ���Ĭ��Ϊ0*/
            unsigned int ResponseTime;
            /*���ڷ���·�ɵ�����������޷�����·�ɣ���Ĭ��Ϊ0.*/
            unsigned int NumberOfRouteHops;
            struct {
                /**/
                char HopHost[DEVICE_LAN_TraceRouteDiagnostics_RouteHops_Host_LEN_256 + 4];
                uint MinimumResponseTime;
                uint AverageResponseTime;
                uint MaximumResponseTime;
            } RouteHops[TRACEROUTE_HOPCOUNT_MAX_64];
        } TraceRouteDiagnostics;
    } LAN;
};

static struct TR069 *g_tr069 = NULL;
static struct TR106Param *g_param = NULL;
struct TR106Index *g_tr106Index = NULL;

static int g_tracert_extend = 0;

void tr069_api_tracert_extend(void)
{
    TR069Printf("========\n");
    g_tracert_extend = 1;
}

static int int_getValue_DeviceInfo(char *name, char *str, unsigned int size)
{
    if (!strcmp(name, "ProvisioningCode"))
        strcpy(str, g_param->DeviceInfo.ProvisioningCode);
    else if (!strcmp(name, "UpTime"))
        snprintf(str, size, "%d", tr069_sec( ));
    else if (!strcmp(name, "FirstUseDate"))
        tr069_time2str(g_param->DeviceInfo.FirstUseDate, str);
    else
        TR069ErrorOut("Device.DeviceInfo. name = %s\n", name);

    return 0;
Err:
    return -1;
}

static int int_getValue_ManagementServer(char *name, char *str, unsigned int size)
{
    if (!strcmp(name, "URL"))
        strcpy(str, g_param->ManagementServer.URL);
    else if (!strcmp(name, "URLBackup"))
        strcpy(str, g_param->ManagementServer.URLBackup);
    else if (!strcmp(name, "Username"))
        strcpy(str, g_param->ManagementServer.Username);
    else if (!strcmp(name, "Password"))
        strcpy(str, g_param->ManagementServer.Password);
    else if (!strcmp(name, "URLModifyFlag"))
        sprintf(str, "%d", g_param->ManagementServer.URLModifyFlag);
    else if (!strcmp(name, "PeriodicInformEnable"))
        sprintf(str, "%u", g_param->ManagementServer.PeriodicInformEnable);
    else if (!strcmp(name, "PeriodicInformInterval"))
        sprintf(str, "%u", g_param->ManagementServer.PeriodicInformInterval);
    else if (!strcmp(name, "PeriodicInformTime")) {
        static uint32_t g_periodInformTime = 0;
        uint32_t periodicInformTime = g_param->ManagementServer.PeriodicInformTime;
        if (0 == periodicInformTime) {
            if (g_periodInformTime == 0) {
                uint32_t sec = tr069_sec( );
                g_periodInformTime = time(NULL);
                if (g_periodInformTime > sec)
                    g_periodInformTime -= sec;
            }
            periodicInformTime = g_periodInformTime;
        }
        tr069_time2str(periodicInformTime, str);
    } else if (!strcmp(name, "ParameterKey"))
        strcpy(str, g_param->ManagementServer.ParameterKey);
    else if (!strcmp(name, "ConnectionRequestURL"))
        strcpy(str, g_param->ManagementServer.ConnectionRequestURL);
    else if (!strcmp(name, "ConnectionRequestUsername"))
        strcpy(str, g_param->ManagementServer.ConnectionRequestUsername);
    else if (!strcmp(name, "ConnectionRequestPassword"))
        strcpy(str, g_param->ManagementServer.ConnectionRequestPassword);
    else if (!strcmp(name, "ConnectionRequestPath"))
        strcpy(str, g_param->ManagementServer.ConnectionRequestPath);
    else if (!strcmp(name, "STUNEnable")) {
        if (g_param->ManagementServer.STUNEnable)
            sprintf(str, "%s", "true");
        else
            sprintf(str, "%s", "false");
    } else if (!strcmp(name, "STUNServerAddress"))
        strcpy(str, g_param->ManagementServer.STUNServerAddress);
    else if (!strcmp(name, "STUNServerPort"))
        sprintf(str, "%u", g_param->ManagementServer.STUNServerPort);
    else if (!strcmp(name, "STUNUsername"))
        strcpy(str, g_param->ManagementServer.STUNUsername);
    else if (!strcmp(name, "STUNPassword"))
        strcpy(str, g_param->ManagementServer.STUNPassword);
    else if (!strcmp(name, "NATDetected")) {
        if (g_param->ManagementServer.NATDetected)
            sprintf(str, "%s", "true");
        else
            sprintf(str, "%s", "false");
    } else if (!strcmp(name, "UDPConnectionRequestAddress"))
        strcpy(str, g_param->ManagementServer.UDPConnectionRequestAddress);
    else if (!strcmp(name, "UDPConnectionRequestAddressNotificationLimit"))
        sprintf(str, "%u", g_param->ManagementServer.UDPConnectionRequestAddressNotificationLimit);
    else if (!strcmp(name, "STUNMaximumKeepAlivePeriod"))
        sprintf(str, "%d", g_param->ManagementServer.STUNMaximumKeepAlivePeriod);
    else if (!strcmp(name, "STUNMinimumKeepAlivePeriod"))
        sprintf(str, "%d", g_param->ManagementServer.STUNMinimumKeepAlivePeriod);
    else
        TR069ErrorOut("Device.ManagementServer. name = %s\n", name);

    return 0;
Err:
    return -1;
}

static int int_getValue_LAN_Stats(char *name, char *str, unsigned int size)
{
    unsigned int val;

    val = 0;
    if (!strcmp(name, "ConnectionUpTime")) {
        val = tr069_sec( );
        if (val > 10)
            val -= 10;
        else
            val = 0;
    } else if (!strcmp(name, "TotalBytesSent"))
        val = tr069_get_TotalBytesSent( );
    else if (!strcmp(name, "TotalBytesReceived"))
        val = tr069_get_TotalBytesReceived( );
    else if (!strcmp(name, "TotalPacketsSent"))
        val = tr069_get_TotalPacketsSent( );
    else if (!strcmp(name, "TotalPacketsReceived"))
        val = tr069_get_TotalPacketsReceived( );
    else if (!strcmp(name, "CurrentDayInterval"))
        val = (tr069_sec( ) - (uint32_t)g_currentday.begintime);
    else if (!strcmp(name, "CurrentDayBytesSent"))
        val = (tr069_get_TotalBytesSent( ) - g_currentday.if_obytes);
    else if (!strcmp(name, "CurrentDayBytesReceived"))
        val = (tr069_get_TotalBytesReceived( ) - g_currentday.if_ibytes);
    else if (!strcmp(name, "CurrentDayPacketsSent"))
        val = (tr069_get_TotalPacketsSent( ) - g_currentday.if_opackets);
    else if (!strcmp(name, "CurrentDayPacketsReceived"))
        val = (tr069_get_TotalPacketsReceived( ) - g_currentday.if_ipackets);
    else if (!strcmp(name, "QuarterHourInterval"))
        val = (tr069_sec( ) - (uint32_t)g_quarterhour.begintime);
    else if (!strcmp(name, "QuarterHourBytesSent"))
        val = (tr069_get_TotalBytesSent( ) - g_currentday.if_obytes);
    else if (!strcmp(name, "QuarterHourBytesReceived"))
        val = (tr069_get_TotalBytesReceived( ) - g_currentday.if_ibytes);
    else if (!strcmp(name, "QuarterHourPacketsSent"))
        val = (tr069_get_TotalPacketsSent( ) - g_currentday.if_opackets);
    else if (!strcmp(name, "QuarterHourPacketsReceived"))
        val = (tr069_get_TotalPacketsReceived( ) - g_currentday.if_ipackets);
    else
        TR069ErrorOut("Device.LAN.Stats. name = %s\n", name);

    sprintf(str, "%u", val);

    return 0;
Err:
    return -1;
}

static int int_getValue_LAN_IPPingDiagnostics(char *name, char *str, unsigned int val)
{
    if (!strcmp(name, "DiagnosticsState"))
        strcpy(str, g_param->LAN.IPPingDiagnostics.DiagnosticsState);
    else if (!strcmp(name, "Host"))
        strcpy(str, g_param->LAN.IPPingDiagnostics.Host);
    else if (!strcmp(name, "NumberOfRepetitions"))
        sprintf(str, "%u", g_param->LAN.IPPingDiagnostics.NumberOfRepetitions);
    else if (!strcmp(name, "Timeout"))
        sprintf(str, "%u", g_param->LAN.IPPingDiagnostics.Timeout);
    else if (!strcmp(name, "DataBlockSize"))
        sprintf(str, "%u", g_param->LAN.IPPingDiagnostics.DataBlockSize);
    else if (!strcmp(name, "DSCP"))
        sprintf(str, "%u", g_param->LAN.IPPingDiagnostics.DSCP);
    else if (!strcmp(name, "SuccessCount"))
        sprintf(str, "%u", g_param->LAN.IPPingDiagnostics.SuccessCount);
    else if (!strcmp(name, "FailureCount"))
        sprintf(str, "%u", g_param->LAN.IPPingDiagnostics.FailureCount);
    else if (!strcmp(name, "AverageResponseTime"))
        sprintf(str, "%u", g_param->LAN.IPPingDiagnostics.AverageResponseTime);
    else if (!strcmp(name, "MinimumResponseTime"))
        sprintf(str, "%u", g_param->LAN.IPPingDiagnostics.MinimumResponseTime);
    else if (!strcmp(name, "MaximumResponseTime"))
        sprintf(str, "%u", g_param->LAN.IPPingDiagnostics.MaximumResponseTime);
    else
        TR069ErrorOut("Device.LAN.IPPingDiagnostics. name = %s\n", name);

    return 0;
Err:
    return -1;
}

static int int_getValue_LAN_TraceRouteDiagnostics(char *name, char *str, unsigned int size)
{
    if (!strcmp(name, "DiagnosticsState"))
        strcpy(str, g_param->LAN.TraceRouteDiagnostics.DiagnosticsState);
    else if (!strcmp(name, "Host"))
        strcpy(str, g_param->LAN.TraceRouteDiagnostics.Host);
    else if (!strcmp(name, "Timeout"))
        sprintf(str, "%u", g_param->LAN.TraceRouteDiagnostics.Timeout);
    else if (!strcmp(name, "DataBlockSize"))
        sprintf(str, "%u", g_param->LAN.TraceRouteDiagnostics.DataBlockSize);
    else if (!strcmp(name, "MaxHopCount"))
        sprintf(str, "%u", g_param->LAN.TraceRouteDiagnostics.MaxHopCount);
    else if (!strcmp(name, "DSCP"))
        sprintf(str, "%u", g_param->LAN.TraceRouteDiagnostics.DSCP);
    else if (!strcmp(name, "ResponseTime"))
        sprintf(str, "%u", g_param->LAN.TraceRouteDiagnostics.ResponseTime);
    else if (!strcmp(name, "NumberOfRouteHops"))
        sprintf(str, "%u", g_param->LAN.TraceRouteDiagnostics.NumberOfRouteHops);
    else if (!strncmp(name, "RouteHops.", 10)) {
        int idx;
        char *p;

        name += 10;

        p = strchr(name, '.');
        idx = atoi(name);
        if (!p || idx <= 0 || idx > TRACEROUTE_HOPCOUNT_MAX_64)
            TR069ErrorOut("Device.LAN.TraceRouteDiagnostics.RouteHops. name = %s\n", name);

        idx -= 1;
        name = p + 1;

        if (!strcmp(name, "HopHost"))
            strcpy(str, g_param->LAN.TraceRouteDiagnostics.RouteHops[idx].HopHost);
        else if (!strcmp(name, "MinimumResponseTime"))
            sprintf(str, "%u", g_param->LAN.TraceRouteDiagnostics.RouteHops[idx].MinimumResponseTime);
        else if (!strcmp(name, "AverageResponseTime"))
            sprintf(str, "%u", g_param->LAN.TraceRouteDiagnostics.RouteHops[idx].AverageResponseTime);
        else if (!strcmp(name, "MaximumResponseTime"))
            sprintf(str, "%u", g_param->LAN.TraceRouteDiagnostics.RouteHops[idx].MaximumResponseTime);
        else
            TR069ErrorOut("Device.LAN.TraceRouteDiagnostics.RouteHops.%d. name = %s\n", idx + 1, name);

    } else
        TR069ErrorOut("Device.LAN.TraceRouteDiagnostics. name = %s\n", name);

    return 0;
Err:
    return -1;
}

static int int_getValue_LAN_DHCPOption(char *name, char *str, unsigned int size)
{
    int idx;
    char *p;

    p = strchr(name, '.');
    idx = atoi(name);
    if (!p || idx <= 0 || idx > DHCP_OPTIONS_MAX_16)
        TR069ErrorOut("Device.LAN.DHCPOption. name = %s\n", name);

    idx -= 1;
    name = p + 1;

    if (!strcmp(name, "Value"))
        strcpy(str, g_param->LAN.DHCPOption[idx].Value);
    else if (!strcmp(name, "Request"))
        sprintf(str, "%u", g_param->LAN.DHCPOption[idx].Request);
    else if (!strcmp(name, "Tag"))
        sprintf(str, "%u", g_param->LAN.DHCPOption[idx].Tag);
    else
        TR069ErrorOut("Device.LAN.DHCPOption.%d. name = %s\n", idx + 1, name);

    return 0;
Err:
    return -1;
}

static int int_getValue_LAN(char *name, char *str, unsigned int size)
{
    if (!strncmp(name, "Stats.", 6)) {
        name += 6;
        return int_getValue_LAN_Stats(name, str, size);
    }
    if (!strncmp(name, "IPPingDiagnostics.", 18)) {
        name += 18;
         return int_getValue_LAN_IPPingDiagnostics(name, str, size);
    }
    if (!strncmp(name, "TraceRouteDiagnostics.", 22)) {
        name += 22;
        return int_getValue_LAN_TraceRouteDiagnostics(name, str, size);
    }
    if (!strncmp(name, "DHCPOption.", 11)) {
        name += 11;
         return int_getValue_LAN_DHCPOption(name, str, size);
    }
    TR069Error("Device.LAN. name = %s\n", name);
    return -1;
}

static int int_getValue_Time(char *name, char *str, unsigned int size)
{
    if (!strcmp(name, "CurrentLocalTime")) {
        time_t t = time(NULL);
        tr069_time2str(t, str);
        return 0;
    }
    TR069Error("Device.LAN. name = %s\n", name);
    return -1;
}

//------------------------------------------------------------------------------
static int int_getValue(char *name, char *str, unsigned int size)
{
    //�ȸ���Ĭ��ֵ
    str[0] = '\0';

    if (strncmp(name, "Device.", 7))
        TR069ErrorOut("name = %s\n", name);
    name += 7;

    if (!strncmp(name, "DeviceInfo.", 11)) {
        name += 11;
        return int_getValue_DeviceInfo(name, str, size);
    }
    if (!strncmp(name, "ManagementServer.", 17)) {
        name += 17;
        return int_getValue_ManagementServer(name, str, size);
    }
    if (!strncmp(name, "LAN.", 4)) {
        name += 4;
        return int_getValue_LAN(name, str, size);
    }
    if (!strncmp(name, "Time.", 5)) {
        name += 5;
        return int_getValue_Time(name, str, size);
    }
    TR069Error("Device. name = %s\n", name);
Err:
    return -1;
}

//------------------------------------------------------------------------------
static int int_check(char *name, char *str, unsigned len)
{
    if (strncmp(name, "Device.", 7))
        TR069ErrorOut("name = %s\n", name);
    name += 7;

    if (!strncmp(name, "ManagementServer.", 17)) {
        name += 17;

        if (!strcmp(name, "URL")) {
            if (strncmp(str, "http://", 7) && strncmp(str, "HTTP://", 7))
                TR069ErrorOut("URL invalid: %s\n", str);
        } else if (!strcmp(name, "URLBackup")) {
            if (strlen(str) > 0 && strncmp(str, "http://", 7) && strncmp(str, "HTTP://", 7))
                TR069ErrorOut("URLBackup invalid: %s\n", str);
        } else
            TR069ErrorOut("Device.ManagementServer. name = %s\n", name);

    } else if (!strncmp(name, "LAN.", 4)) {
        name += 4;

        if (!strncmp(name, "IPPingDiagnostics.", 18)) {
            name += 18;

            if (!strcmp(name, "DiagnosticsState")) {
                if (strcmp(str, "Requested"))
                    TR069ErrorOut("Device.LAN.IPPingDiagnostics.DiagnosticsState = %s\n", str);
            } else
                TR069ErrorOut("Device.LAN.IPPingDiagnostics. name = %s\n", name);

        } else if (!strncmp(name, "TraceRouteDiagnostics.", 22)) {
            name += 22;

            if (!strcmp(name, "DiagnosticsState")) {
                if (strcmp(str, "Requested"))
                    TR069ErrorOut("Device.LAN.TraceRouteDiagnostics.DiagnosticsState = %s\n", str);
            } else
                TR069ErrorOut("Device.LAN.TraceRouteDiagnostics. name = %s\n", name);

        } else
            TR069ErrorOut("Device.LAN. name = %s\n", name);

    } else
        TR069ErrorOut("Device. name = %s\n", name);

    return 0;
Err:
    return 1;
}

static int int_setValue_DeviceInfo(char *name, char *str, unsigned int len)
{
    if (!strcmp(name, "FirstUseDate"))
        tr069_str2time(str, &g_param->DeviceInfo.FirstUseDate);
    else if (!strcmp(name, "ProvisioningCode"))
        strcpy(g_param->DeviceInfo.ProvisioningCode, str);
    else
        TR069ErrorOut("Device.DeviceInfo. name = %s\n", name);

    return 0;
Err:
    return -1;
}

static int int_setValue_ManagementServer(char *name, char *str, unsigned int len)
{
    if (!strcmp(name, "URL")) {
        char *url = g_param->ManagementServer.URL;
        if (strlen(url) > 0 && strcmp(url, str)) {
            TR069Printf("URL Change\n");
            g_tr069->bootstrap = 0;
            g_tr069->save_flag = 1;
            g_tr069->retry_connect = 0;
        }
        strcpy(g_param->ManagementServer.URL, str);
    }
    else if (!strcmp(name, "URLBackup"))
        strcpy(g_param->ManagementServer.URLBackup, str);
    else if (!strcmp(name, "Username"))
        strcpy(g_param->ManagementServer.Username, str);
    else if (!strcmp(name, "Password"))
        strcpy(g_param->ManagementServer.Password, str);
    else if (!strcmp(name, "URLModifyFlag"))
        tr069_str2int(str, &g_param->ManagementServer.URLModifyFlag);
    else if (!strcmp(name, "PeriodicInformEnable"))
        tr069_str2uint(str, &g_param->ManagementServer.PeriodicInformEnable);
    else if (!strcmp(name, "PeriodicInformInterval"))
        tr069_str2uint(str, &g_param->ManagementServer.PeriodicInformInterval);
    else if (!strcmp(name, "PeriodicInformTime"))
        tr069_str2time(str, &g_param->ManagementServer.PeriodicInformTime);
    else if (!strcmp(name, "ParameterKey"))
        strcpy(g_param->ManagementServer.ParameterKey, str);
    else if (!strcmp(name, "ConnectionRequestURL"))
        strcpy(g_param->ManagementServer.ConnectionRequestURL, str);
    else if (!strcmp(name, "ConnectionRequestUsername"))
        strcpy(g_param->ManagementServer.ConnectionRequestUsername, str);
    else if (!strcmp(name, "ConnectionRequestPassword"))
        strcpy(g_param->ManagementServer.ConnectionRequestPassword, str);
    else if (!strcmp(name, "ConnectionRequestPath"))
        strcpy(g_param->ManagementServer.ConnectionRequestPath, str);
    else if (!strcmp(name, "STUNEnable"))
        tr069_str2uint(str, &g_param->ManagementServer.STUNEnable);
    else if (!strcmp(name, "STUNServerAddress"))
        strcpy(g_param->ManagementServer.STUNServerAddress, str);
    else if (!strcmp(name, "STUNServerPort"))
        tr069_str2uint(str, &g_param->ManagementServer.STUNServerPort);
    else if (!strcmp(name, "STUNUsername"))
        strcpy(g_param->ManagementServer.STUNUsername, str);
    else if (!strcmp(name, "STUNPassword"))
        strcpy(g_param->ManagementServer.STUNPassword, str);
    else if (!strcmp(name, "NATDetected"))
        tr069_str2uint(str, &g_param->ManagementServer.NATDetected);
    else if (!strcmp(name, "UDPConnectionRequestAddress"))
        strcpy(g_param->ManagementServer.UDPConnectionRequestAddress, str);
    else if (!strcmp(name, "UDPConnectionRequestAddressNotificationLimit"))
        tr069_str2uint(str, &g_param->ManagementServer.UDPConnectionRequestAddressNotificationLimit);
    else if (!strcmp(name, "STUNMaximumKeepAlivePeriod"))
        tr069_str2uint(str, &g_param->ManagementServer.STUNMaximumKeepAlivePeriod);
    else if (!strcmp(name, "STUNMinimumKeepAlivePeriod"))
        tr069_str2uint(str, &g_param->ManagementServer.STUNMinimumKeepAlivePeriod);
    else
        TR069ErrorOut("Device.ManagementServer. name = %s\n", name);

    return 0;
Err:
    return -1;
}

static int int_setValue_LAN_DHCPOption(char *name, char *str, unsigned int len)
{
    int idx;
    char *p;

    p = strchr(name, '.');
    idx = atoi(name);
    if (!p || idx <= 0 || idx > DHCP_OPTIONS_MAX_16)
        TR069ErrorOut("Device.LAN.DHCPOption. name = %s\n", name);

    idx -= 1;
    name = p + 1;

    if (!strcmp(name, "Value"))
        strcpy(g_param->LAN.DHCPOption[idx].Value, str);
    else if (!strcmp(name, "Request"))
        tr069_str2uint(str, &g_param->LAN.DHCPOption[idx].Request);
    else if (!strcmp(name, "Tag"))
        tr069_str2uint(str, &g_param->LAN.DHCPOption[idx].Tag);
    else
        TR069ErrorOut("Device.LAN.DHCPOption.%d. name = %s\n", idx + 1, name);

    return 0;
Err:
    return -1;
}

static int int_setValue_LAN_IPPingDiagnostics(char *name, char *str, unsigned int x)
{
    if (!strcmp(name, "DiagnosticsState"))
        strcpy(g_param->LAN.IPPingDiagnostics.DiagnosticsState, str);
    else if (!strcmp(name, "Host"))
        strcpy(g_param->LAN.IPPingDiagnostics.Host, str);
    else if (!strcmp(name, "NumberOfRepetitions"))
        tr069_str2uint(str, &g_param->LAN.IPPingDiagnostics.NumberOfRepetitions);
    else if (!strcmp(name, "Timeout"))
        tr069_str2uint(str, &g_param->LAN.IPPingDiagnostics.Timeout);
    else if (!strcmp(name, "DataBlockSize"))
        tr069_str2uint(str, &g_param->LAN.IPPingDiagnostics.DataBlockSize);
    else if (!strcmp(name, "DSCP"))
        tr069_str2uint(str, &g_param->LAN.IPPingDiagnostics.DSCP);
    else if (!strcmp(name, "SuccessCount"))
        g_param->LAN.IPPingDiagnostics.SuccessCount = x;
    else if (!strcmp(name, "FailureCount"))
        g_param->LAN.IPPingDiagnostics.FailureCount = x;
    else if (!strcmp(name, "AverageResponseTime"))
        g_param->LAN.IPPingDiagnostics.AverageResponseTime = x;
    else if (!strcmp(name, "MinimumResponseTime"))
        g_param->LAN.IPPingDiagnostics.MinimumResponseTime = x;
    else if (!strcmp(name, "MaximumResponseTime"))
        g_param->LAN.IPPingDiagnostics.MaximumResponseTime = x;
    else
        TR069ErrorOut("Device.LAN.IPPingDiagnostics. name = %s\n", name);

    return 0;
Err:
    return -1;
}

static int int_setValue_LAN_TraceRouteDiagnostics(char *name, char *str, unsigned int x)
{
    if (!strcmp(name, "DiagnosticsState"))
        strcpy(g_param->LAN.TraceRouteDiagnostics.DiagnosticsState, str);
    else if (!strcmp(name, "Host"))
        strcpy(g_param->LAN.TraceRouteDiagnostics.Host, str);
    else if (!strcmp(name, "Timeout"))
        tr069_str2uint(str, &g_param->LAN.TraceRouteDiagnostics.Timeout);
    else if (!strcmp(name, "DataBlockSize"))
        tr069_str2uint(str, &g_param->LAN.TraceRouteDiagnostics.DataBlockSize);
    else if (!strcmp(name, "MaxHopCount"))
        tr069_str2uint(str, &g_param->LAN.TraceRouteDiagnostics.MaxHopCount);
    else if (!strcmp(name, "DSCP"))
        tr069_str2uint(str, &g_param->LAN.TraceRouteDiagnostics.DSCP);
    else if (!strcmp(name, "ResponseTime"))
        g_param->LAN.TraceRouteDiagnostics.ResponseTime = x;
    else if (!strcmp(name, "NumberOfRouteHops"))
        g_param->LAN.TraceRouteDiagnostics.NumberOfRouteHops = x;
    else
        TR069ErrorOut("Device.LAN.TraceRouteDiagnostics. name = %s\n", name);

    return 0;
Err:
    return -1;
}

static int int_setValue_LAN(char *name, char *str, unsigned int val)
{
    if (!strncmp(name, "DHCPOption.", 11)) {
        name += 11;
        return int_setValue_LAN_DHCPOption(name, str, val);
    }
    if (!strncmp(name, "IPPingDiagnostics.", 18)) {
        name += 18;
        return int_setValue_LAN_IPPingDiagnostics(name, str, val);
    }
    if (!strncmp(name, "TraceRouteDiagnostics.", 22)) {
        name += 22;
        return int_setValue_LAN_TraceRouteDiagnostics(name, str, val);
    }
    TR069Error("Device.LAN. name = %s\n", name);
    return -1;
}

//------------------------------------------------------------------------------
static int int_setValue(char *name, char *str, unsigned int len)
{
    if (strncmp(name, "Device.", 7))
        TR069ErrorOut("name = %s\n", name);
    name += 7;

    if (!strncmp(name, "ManagementServer.", 17)) {
        name += 17;
        return int_setValue_ManagementServer(name, str, len);
    }
    if (!strncmp(name, "LAN.", 4)) {
        name += 4;
        return int_setValue_LAN(name, str, len);
    }
    if (!strncmp(name, "DeviceInfo.", 11)) {
        name += 11;
        return int_setValue_DeviceInfo(name, str, len);
    }
    TR069Error("Device. name = %s\n", name);
Err:
    return -1;
}

//------------------------------------------------------------------------------
static void int_onChange(struct TR069 *tr069, char *name)
{
    if (strncmp(name, "Device.", 7))
        TR069ErrorOut("name = %s\n", name);
    name += 7;

    if (!strncmp(name, "ManagementServer.", 17)) {
        name += 17;

        if (!strcmp(name, "URL")) {
            tr069_port_setValue("Device.ManagementServer.URL", g_param->ManagementServer.URL, 0);
        } else if (!strcmp(name, "Username")) {
            tr069_port_setValue("Device.ManagementServer.Username", g_param->ManagementServer.Username, 0);
        } else if (!strcmp(name, "Password")) {
            tr069_port_setValue("Device.ManagementServer.Password", g_param->ManagementServer.Password, 0);
        } else if (!strcmp(name, "ConnectionRequestUsername")) {
            tr069_port_setValue("Device.ManagementServer.ConnectionRequestUsername", g_param->ManagementServer.ConnectionRequestUsername, 0);
        } else if (!strcmp(name, "ConnectionRequestPassword")) {
            tr069_port_setValue("Device.ManagementServer.ConnectionRequestPassword", g_param->ManagementServer.ConnectionRequestPassword, 0);
        } else if (!strcmp(name, "STUNPassword")) {
            tr069_port_setValue("Device.ManagementServer.STUNPassword", g_param->ManagementServer.STUNPassword, 0);
        } else if (!strcmp(name, "PeriodicInformEnable") || !strcmp(name, "PeriodicInformInterval") || !strcmp(name, "PeriodicInformTime")) {
            tr069_periodic_timer(tr069);
        } else if (!strcmp(name, "STUNEnable")) {
            unsigned int stunEnable;

            stunEnable = g_param->ManagementServer.STUNEnable;
            TR069Printf("stunEnable flag change, currentState=%u\n", stunEnable);
            
            /* Change STUN detected state when stunEnable status change. */
            if( stunEnable ){
                stun_api_pipe_message( STUN_TASK_ACTIVE, 0);
            }else{
                stun_api_pipe_message( STUN_TASK_SUSPEND, 0);
            }
        } else if (!strcmp(name, "UDPConnectionRequestAddress")) {
            struct Param* param;
            param = tr069_param_hash_find(tr069, "Device.ManagementServer.NATDetected");
            if (param)
                tr069_paramChange_inset(tr069, param);
        } else {
            TR069ErrorOut("Device.ManagementServer. name = %s\n", name);
        }

    } else if (!strncmp(name, "LAN.", 4)) {
        name += 4;

        if (!strncmp(name, "IPPingDiagnostics.", 18)) {
            name += 18;

            if (!strcmp(name, "DiagnosticsState"))
                tr069->diag_ping = 1;
            else if (!strcmp(name, "Host") || !strcmp(name, "NumberOfRepetitions") || !strcmp(name, "Timeout") || !strcmp(name, "DataBlockSize") || !strcmp(name, "DSCP"))
                strcpy(g_param->LAN.IPPingDiagnostics.DiagnosticsState, "None");
            else
                TR069ErrorOut("Device.LAN.TraceRouteDiagnostics. name = %s\n", name);

        } else if (!strncmp(name, "TraceRouteDiagnostics.", 22)) {
            name += 22;

            if (!strcmp(name, "DiagnosticsState"))
                tr069->diag_trace = 1;
            else if (!strcmp(name, "Host") || !strcmp(name, "Timeout") || !strcmp(name, "DataBlockSize") || !strcmp(name, "MaxHopCount") || !strcmp(name, "DSCP"))
                strcpy(g_param->LAN.TraceRouteDiagnostics.DiagnosticsState, "None");
            else
                TR069ErrorOut("Device.LAN.TraceRouteDiagnostics. name = %s\n", name);

        } else
            TR069ErrorOut("Device.LAN. name = %s\n", name);

    } else
        TR069ErrorOut("Device. name = %s\n", name);

Err:
    return;
}

//------------------------------------------------------------------------------
static void int_DeviceInfo_init(struct TR069 *tr069)
{
    tr069_param_new(tr069, "Device.DeviceInfo.", NULL, TR069_OBJECT, 0, TR069_ENABLE_READ, NULL, NULL, NULL, NULL);
    tr069_param_new(tr069, "Device.DeviceInfo.Manufacturer",    NULL, TR069_STRING, 0, TR069_ENABLE_READ, tr069_port_getValue, NULL, NULL, NULL);
    tr069_param_new(tr069, "Device.DeviceInfo.ManufacturerOUI", NULL, TR069_STRING, 0, TR069_ENABLE_READ, tr069_port_getValue, NULL, NULL, NULL);
    tr069_param_new(tr069, "Device.DeviceInfo.ModelName",       NULL, TR069_STRING, 0, TR069_ENABLE_READ, tr069_port_getValue, NULL, NULL, NULL);
    tr069_param_new(tr069, "Device.DeviceInfo.Description",     NULL, TR069_STRING, 0, TR069_ENABLE_READ, tr069_port_getValue, NULL, NULL, NULL);
    tr069_param_new(tr069, "Device.DeviceInfo.ProductClass",    NULL, TR069_STRING, 0, TR069_ENABLE_READ, tr069_port_getValue, NULL, NULL, NULL);
    tr069_param_new(tr069, "Device.DeviceInfo.SerialNumber",    NULL, TR069_STRING, 0, TR069_ENABLE_READ, tr069_port_getValue, NULL, NULL, NULL);
    tr069_param_new(tr069, "Device.DeviceInfo.HardwareVersion",             "HardwareVersion",          TR069_STRING,   0, TR069_ENABLE_READ_ATTR, tr069_port_getValue, NULL, NULL, NULL);
    tr069_param_new(tr069, "Device.DeviceInfo.SoftwareVersion",             "SoftwareVersion",          TR069_STRING,   0, TR069_ENABLE_READ_ATTR, tr069_port_getValue, NULL, NULL, NULL);
    tr069_param_new(tr069, "Device.DeviceInfo.AdditionalHardwareVersion",   "AdditionalHardwareVersion",TR069_STRING,   0, TR069_ENABLE_READ_ATTR, tr069_port_getValue, NULL, NULL, NULL);
    tr069_param_new(tr069, "Device.DeviceInfo.AdditionalSoftwareVersion",   "AdditionalSoftwareVersion",TR069_STRING,   0, TR069_ENABLE_READ_ATTR, tr069_port_getValue, NULL, NULL, NULL);
    tr069_param_new(tr069, "Device.DeviceInfo.ProvisioningCode",            "ProvisioningCode",         TR069_STRING,   DEVICE_DeviceInfo_ProvisioningCode_LEN_64, TR069_ENABLE_READ_WRITE_APPLY_ATTR_SAVE, int_getValue, NULL, int_setValue, NULL);
    tr069_param_new(tr069, "Device.DeviceInfo.DeviceStatus",                "DeviceStatus",             TR069_STRING,   0, TR069_ENABLE_READ_ATTR, tr069_port_getValue, NULL, NULL, NULL);
    tr069_param_new(tr069, "Device.DeviceInfo.UpTime",                      NULL,                       TR069_UNSIGNED, 0, TR069_ENABLE_READ,      int_getValue,        NULL, NULL, NULL);
    tr069_param_new(tr069, "Device.DeviceInfo.FirstUseDate",                "FirstUseDate",             TR069_DATETIME, 0, TR069_ENABLE_READ_SAVE, int_getValue,        NULL, int_setValue, NULL);
    tr069_param_new(tr069, "Device.DeviceInfo.DeviceLog",                   NULL,                       TR069_STRING,   0, TR069_ENABLE_READ,      tr069_port_getValue, NULL, NULL, NULL);
}

//------------------------------------------------------------------------------
static void int_ManagementServer_init(struct TR069 *tr069)
{
#ifdef ANDROID
#define TR106_ENABLE_SAVE 0
#else
#define TR106_ENABLE_SAVE TR069_ENABLE_SAVE
#endif
    tr069_param_new(tr069, "Device.ManagementServer.", NULL, TR069_OBJECT, 0, TR069_ENABLE_READ, NULL, NULL, NULL, NULL);
    tr069_param_new(tr069, "Device.ManagementServer.URL",           "URL",           TR069_STRING, DEVICE_ManagementServer_URL_LEN_256,       TR069_ENABLE_READ_WRITE_APPLY_ATTR | TR106_ENABLE_SAVE,   int_getValue, int_check, int_setValue, int_onChange);
    tr069_param_new(tr069, "Device.ManagementServer.URLBackup",     "URLBackup",     TR069_STRING, DEVICE_ManagementServer_URLBackup_LEN_256, TR069_ENABLE_READ_WRITE_APPLY_ATTR_SAVE,                  int_getValue, int_check, int_setValue, NULL);
    tr069_param_new(tr069, "Device.ManagementServer.Username",      "Username",      TR069_STRING, DEVICE_ManagementServer_Username_LEN_64,   TR069_ENABLE_READ_WRITE_APPLY_ATTR | TR106_ENABLE_SAVE,   int_getValue, NULL,      int_setValue, int_onChange);
    tr069_param_new(tr069, "Device.ManagementServer.Password",      "Password",      TR069_STRING, DEVICE_ManagementServer_Password_LEN_64,   TR069_ENABLE_WRITE | TR106_ENABLE_SAVE,                   int_getValue, NULL,      int_setValue, int_onChange);
    tr069_param_new(tr069, "Device.ManagementServer.URLModifyFlag", "URLModifyFlag", TR069_INT,    0,                                         TR069_ENABLE_READ_WRITE_APPLY_ATTR_CONFIG_SAVE,           int_getValue, NULL,      int_setValue, NULL);
    tr069_param_new(tr069, "Device.ManagementServer.PeriodicInformEnable",      "PeriodicInformEnable",      TR069_BOOLEAN,  0,                                                        TR069_ENABLE_READ_WRITE_APPLY_ATTR_CONFIG | TR106_ENABLE_SAVE, int_getValue, NULL, int_setValue, int_onChange);
    tr069_param_new(tr069, "Device.ManagementServer.PeriodicInformInterval",    "PeriodicInformInterval",    TR069_UNSIGNED, 0,                                                        TR069_ENABLE_READ_WRITE_APPLY_ATTR_CONFIG | TR106_ENABLE_SAVE, int_getValue, NULL, int_setValue, int_onChange);
    tr069_param_new(tr069, "Device.ManagementServer.PeriodicInformTime",        "PeriodicInformTime",        TR069_DATETIME, 0,                                                        TR069_ENABLE_READ_WRITE_APPLY_ATTR_CONFIG | TR106_ENABLE_SAVE, int_getValue, NULL, int_setValue, int_onChange);
    tr069_param_new(tr069, "Device.ManagementServer.ParameterKey",              "ParameterKey",              TR069_STRING,   DEVICE_ManagementServer_ParameterKey_LEN_32,              TR069_ENABLE_READ_SAVE,                                        int_getValue, NULL, int_setValue, NULL);
    g_tr106Index->ManagementServer_ConnectionRequestURL_index       = 
    tr069_param_new(tr069, "Device.ManagementServer.ConnectionRequestURL",      "ConnectionRequestURL",      TR069_STRING,   DEVICE_ManagementServer_ConnectionRequestURL_LEN_256,     TR069_ENABLE_READ_ATTR,                   int_getValue, NULL, int_setValue, NULL);
    tr069_param_new(tr069, "Device.ManagementServer.ConnectionRequestUsername", "ConnectionRequestUsername", TR069_STRING,   DEVICE_ManagementServer_ConnectionRequestUsername_LEN_64, TR069_ENABLE_READ_WRITE_APPLY_ATTR_SAVE,  int_getValue, NULL, int_setValue, int_onChange);
    tr069_param_new(tr069, "Device.ManagementServer.ConnectionRequestPassword", "ConnectionRequestPassword", TR069_STRING,   DEVICE_ManagementServer_ConnectionRequestPassword_LEN_64, TR069_ENABLE_WRITE | TR106_ENABLE_SAVE,   int_getValue, NULL, int_setValue, int_onChange);
    tr069_param_new(tr069, "Device.ManagementServer.UpgradesManaged",           "UpgradesManaged",           TR069_BOOLEAN,  0,                                                        TR069_ENABLE_READ_WRITE_APPLY_ATTR,   tr069_port_getValue, NULL, tr069_port_setValue, NULL);
    g_tr106Index->ManagementServer_ConnectionRequestPath_index      = 
    tr069_param_new(tr069, "Device.ManagementServer.ConnectionRequestPath",     "ConnectionRequestPath",     TR069_STRING,   DEVICE_ManagementServer_ConnectionRequestPath_LEN_64,     TR069_ENABLE_READ_WRITE_ATTR_SAVE,        int_getValue, NULL, int_setValue, NULL);

    tr069_param_new(tr069, "Device.ManagementServer.STUNEnable",        "STUNEnable",        TR069_BOOLEAN,  0,                                                TR069_ENABLE_READ_WRITE_APPLY_ATTR_SAVE, int_getValue, NULL, int_setValue, int_onChange);
    tr069_param_new(tr069, "Device.ManagementServer.STUNServerAddress", "STUNServerAddress", TR069_STRING,   DEVICE_ManagementServer_STUNServerAddress_LEN_64, TR069_ENABLE_READ_WRITE_APPLY_ATTR_SAVE, int_getValue, NULL, int_setValue, NULL);
    tr069_param_new(tr069, "Device.ManagementServer.STUNServerPort",    "STUNServerPort",    TR069_UNSIGNED, 0,                                                TR069_ENABLE_READ_WRITE_APPLY_ATTR_SAVE, int_getValue, NULL, int_setValue, NULL);
    tr069_param_new(tr069, "Device.ManagementServer.STUNUsername",      "STUNUsername",      TR069_STRING,   DEVICE_ManagementServer_STUNUsername_LEN_64,      TR069_ENABLE_READ_WRITE_APPLY_ATTR_SAVE, int_getValue, NULL, int_setValue, NULL);
    tr069_param_new(tr069, "Device.ManagementServer.STUNPassword",      "STUNPassword",      TR069_STRING,   DEVICE_ManagementServer_STUNPassword_LEN_64,      TR069_ENABLE_WRITE | TR106_ENABLE_SAVE,  int_getValue, NULL, int_setValue, int_onChange);
    tr069_param_new(tr069, "Device.ManagementServer.NATDetected",       "NATDetected",       TR069_BOOLEAN,  0,                                                TR069_ENABLE_READ_WRITE_APPLY_ATTR,      int_getValue, NULL, int_setValue, NULL);
    tr069_param_new(tr069, "Device.ManagementServer.UDPConnectionRequestAddress", "UDPConnectionRequestAddress",TR069_STRING,   DEVICE_ManagementServer_UDPConnectionRequestAddress_LEN_64, TR069_ENABLE_READ_WRITE_APPLY_ATTR,      int_getValue, NULL, int_setValue, int_onChange);
    tr069_param_new(tr069, "Device.ManagementServer.UDPConnectionRequestAddressNotificationLimit", "UDPConnectionRequestAddressNotificationLimit", TR069_UNSIGNED, 0,                       TR069_ENABLE_READ_WRITE_APPLY_ATTR,      int_getValue, NULL, int_setValue, NULL);
    tr069_param_new(tr069, "Device.ManagementServer.STUNMaximumKeepAlivePeriod",   "STUNMaximumKeepAlivePeriod", TR069_UNSIGNED, 0,                                                         TR069_ENABLE_READ_WRITE_APPLY_ATTR_SAVE, int_getValue, NULL, int_setValue, NULL);
    tr069_param_new(tr069, "Device.ManagementServer.STUNMinimumKeepAlivePeriod",   "STUNMinimumKeepAlivePeriod", TR069_UNSIGNED, 0,                                                         TR069_ENABLE_READ_WRITE_APPLY_ATTR_SAVE, int_getValue, NULL, int_setValue, NULL);
}

//------------------------------------------------------------------------------
static void int_Time_init(struct TR069 *tr069)
{
    tr069_param_new(tr069, "Device.Time.", NULL, TR069_OBJECT, 0, TR069_ENABLE_READ, NULL, NULL, NULL, NULL);
    tr069_param_new(tr069, "Device.Time.NTPServer1",       "NTPServer1",    TR069_STRING,   DEVICE_Time_NTPServer_LEN_31,     TR069_ENABLE_READ_WRITE_APPLY_ATTR, tr069_port_getValue, NULL, tr069_port_setValue, NULL);
    tr069_param_new(tr069, "Device.Time.NTPServer2",       "NTPServer2",    TR069_STRING,   DEVICE_Time_NTPServer_LEN_31,     TR069_ENABLE_READ_WRITE_APPLY_ATTR, tr069_port_getValue, NULL, tr069_port_setValue, NULL);
    tr069_param_new(tr069, "Device.Time.CurrentLocalTime", NULL,            TR069_DATETIME, 0,                                TR069_ENABLE_READ,                  int_getValue, NULL, NULL, NULL);
    tr069_param_new(tr069, "Device.Time.LocalTimeZone",    "LocalTimeZone", TR069_STRING,   DEVICE_Time_LocalTimeZone_LEN_64, TR069_ENABLE_READ_WRITE_APPLY_ATTR, tr069_port_getValue, NULL, tr069_port_setValue, NULL);
}

//------------------------------------------------------------------------------
static void int_UserInterface_init(struct TR069 *tr069)
{
    tr069_param_new(tr069, "Device.UserInterface.", NULL, TR069_OBJECT, 0, TR069_ENABLE_READ, NULL, NULL, NULL, NULL);
    tr069_param_new(tr069, "Device.UserInterface.AutoUpdateServer",   NULL, TR069_STRING,    DEVICE_UserInterface_AutoUpdateServer_LEN_256, TR069_ENABLE_READ_WRITE_APPLY, tr069_port_getValue, NULL, tr069_port_setValue, NULL);
    tr069_param_new(tr069, "Device.UserInterface.AvailableLanguages", NULL, TR069_STRING,    0,                                             TR069_ENABLE_READ,             tr069_port_getValue, NULL, NULL,                NULL);
    tr069_param_new(tr069, "Device.UserInterface.CurrentLanguage",    NULL, TR069_STRING,    DEVICE_UserInterface_CurrentLanguage_LEN_16,   TR069_ENABLE_READ_WRITE_APPLY, tr069_port_getValue, NULL, tr069_port_setValue, NULL);
}

//------------------------------------------------------------------------------
static void int_LAN_DHCPOption_init(struct TR069 *tr069)
{
    tr069_param_new(tr069, "Device.LAN.DHCPOption.", "DHCPOption", TR069_OBJECT, 0, TR069_ENABLE_ADD | TR069_ENABLE_READ_WRITE_ATTR, NULL, NULL, NULL, NULL);

    tr069_param_virtual(tr069, "Device.LAN.DHCPOption.", "Request", TR069_BOOLEAN,  0,                                  TR069_ENABLE_READ_WRITE_APPLY,  int_getValue, int_setValue);
    tr069_param_virtual(tr069, "Device.LAN.DHCPOption.", "Tag",     TR069_UNSIGNED, 0,                                  TR069_ENABLE_READ_WRITE_APPLY,  int_getValue, int_setValue);
    tr069_param_virtual(tr069, "Device.LAN.DHCPOption.", "Value",   TR069_BASE64,   DEVICE_LAN_DHCPOption_Value_LEN_64, TR069_ENABLE_READ_WRITE_APPLY,  int_getValue, int_setValue);
}

void tr106_CurrentDay_ontime(struct TR069 *tr069)
{
    unsigned int current = tr069_sec( );

    g_currentday.begintime = current;
    g_currentday.if_obytes = tr069_get_TotalBytesSent( );
    g_currentday.if_ibytes = tr069_get_TotalBytesReceived( );
    g_currentday.if_opackets = tr069_get_TotalPacketsSent( );
    g_currentday.if_ipackets = tr069_get_TotalPacketsReceived( );
    tr069_timer_create(tr069, current + INTERVAL_CurrentDay_SEC, tr106_CurrentDay_ontime);
}

void tr106_QuarterHour_ontime(struct TR069 *tr069)
{
    unsigned int current = tr069_sec( );

    g_quarterhour.begintime = current;
    g_quarterhour.if_obytes = tr069_get_TotalBytesSent( );
    g_quarterhour.if_ibytes = tr069_get_TotalBytesReceived( );
    g_quarterhour.if_opackets = tr069_get_TotalPacketsSent( );
    g_quarterhour.if_ipackets = tr069_get_TotalPacketsReceived( );
    tr069_timer_create(tr069, current + INTERVAL_QuarterHour_SEC, tr106_QuarterHour_ontime);
}

//------------------------------------------------------------------------------
static void int_LAN_Stats_init(struct TR069 *tr069)
{
    tr069_param_new(tr069, "Device.LAN.Stats.", NULL, TR069_OBJECT, 0, TR069_ENABLE_READ, NULL, NULL, NULL, NULL);
    tr069_param_new(tr069, "Device.LAN.Stats.ConnectionUpTime",     NULL, TR069_UNSIGNED, 0, TR069_ENABLE_READ, int_getValue, NULL, NULL, NULL);
    tr069_param_new(tr069, "Device.LAN.Stats.TotalBytesSent",       NULL, TR069_UNSIGNED, 0, TR069_ENABLE_READ, int_getValue, NULL, NULL, NULL);
    tr069_param_new(tr069, "Device.LAN.Stats.TotalBytesReceived",   NULL, TR069_UNSIGNED, 0, TR069_ENABLE_READ, int_getValue, NULL, NULL, NULL);
    tr069_param_new(tr069, "Device.LAN.Stats.TotalPacketsSent",     NULL, TR069_UNSIGNED, 0, TR069_ENABLE_READ, int_getValue, NULL, NULL, NULL);
    tr069_param_new(tr069, "Device.LAN.Stats.TotalPacketsReceived", NULL, TR069_UNSIGNED, 0, TR069_ENABLE_READ, int_getValue, NULL, NULL, NULL);

    tr069_param_new(tr069, "Device.LAN.Stats.CurrentDayInterval",        NULL, TR069_UNSIGNED, 0, TR069_ENABLE_READ, int_getValue, NULL, NULL, NULL);
    tr069_param_new(tr069, "Device.LAN.Stats.CurrentDayBytesSent",       NULL, TR069_UNSIGNED, 0, TR069_ENABLE_READ, int_getValue, NULL, NULL, NULL);
    tr069_param_new(tr069, "Device.LAN.Stats.CurrentDayBytesReceived",   NULL, TR069_UNSIGNED, 0, TR069_ENABLE_READ, int_getValue, NULL, NULL, NULL);
    tr069_param_new(tr069, "Device.LAN.Stats.CurrentDayPacketsSent",     NULL, TR069_UNSIGNED, 0, TR069_ENABLE_READ, int_getValue, NULL, NULL, NULL);
    tr069_param_new(tr069, "Device.LAN.Stats.CurrentDayPacketsReceived", NULL, TR069_UNSIGNED, 0, TR069_ENABLE_READ, int_getValue, NULL, NULL, NULL);

    tr069_param_new(tr069, "Device.LAN.Stats.QuarterHourInterval",        NULL, TR069_UNSIGNED, 0, TR069_ENABLE_READ, int_getValue, NULL, NULL, NULL);
    tr069_param_new(tr069, "Device.LAN.Stats.QuarterHourBytesSent",       NULL, TR069_UNSIGNED, 0, TR069_ENABLE_READ, int_getValue, NULL, NULL, NULL);
    tr069_param_new(tr069, "Device.LAN.Stats.QuarterHourBytesReceived",   NULL, TR069_UNSIGNED, 0, TR069_ENABLE_READ, int_getValue, NULL, NULL, NULL);
    tr069_param_new(tr069, "Device.LAN.Stats.QuarterHourPacketsSent",     NULL, TR069_UNSIGNED, 0, TR069_ENABLE_READ, int_getValue, NULL, NULL, NULL);
    tr069_param_new(tr069, "Device.LAN.Stats.QuarterHourPacketsReceived", NULL, TR069_UNSIGNED, 0, TR069_ENABLE_READ, int_getValue, NULL, NULL, NULL);
}

//------------------------------------------------------------------------------
static void int_LAN_IPPingDiagnostics_init(struct TR069 *tr069)
{
    tr069_param_new(tr069, "Device.LAN.IPPingDiagnostics.", NULL, TR069_OBJECT, 0, TR069_ENABLE_READ, NULL, NULL, NULL, NULL);
    tr069_param_new(tr069, "Device.LAN.IPPingDiagnostics.DiagnosticsState",    "Ping_DiagnosticsState",    TR069_STRING,   DEVICE_LAN_IPPingDiagnostics_DiagnosticsState_LEN_32, TR069_ENABLE_READ_WRITE_APPLY,      int_getValue, int_check, int_setValue, int_onChange);
    tr069_param_new(tr069, "Device.LAN.IPPingDiagnostics.Host",                "Ping_Host",                TR069_STRING,   DEVICE_LAN_IPPingDiagnostics_Host_LEN_256,            TR069_ENABLE_READ_WRITE_APPLY_SAVE, int_getValue, NULL,        int_setValue, int_onChange);
    tr069_param_new(tr069, "Device.LAN.IPPingDiagnostics.NumberOfRepetitions", "Ping_NumberOfRepetitions", TR069_UNSIGNED, 0,                                                    TR069_ENABLE_READ_WRITE_APPLY_SAVE, int_getValue, NULL,        int_setValue, int_onChange);
    tr069_param_new(tr069, "Device.LAN.IPPingDiagnostics.Timeout",             "Ping_Timeout",             TR069_UNSIGNED, 0,                                                    TR069_ENABLE_READ_WRITE_APPLY_SAVE, int_getValue, NULL,        int_setValue, int_onChange);
    tr069_param_new(tr069, "Device.LAN.IPPingDiagnostics.DataBlockSize",       "Ping_DataBlockSize",       TR069_UNSIGNED, 0,                                                    TR069_ENABLE_READ_WRITE_APPLY_SAVE, int_getValue, NULL,        int_setValue, int_onChange);
    tr069_param_new(tr069, "Device.LAN.IPPingDiagnostics.DSCP",                "Ping_DSCP",                TR069_UNSIGNED, 0,                                                    TR069_ENABLE_READ_WRITE_APPLY_SAVE, int_getValue, NULL,        int_setValue, int_onChange);
    tr069_param_new(tr069, "Device.LAN.IPPingDiagnostics.SuccessCount",        NULL, TR069_UNSIGNED, 0, TR069_ENABLE_READ, int_getValue, NULL, NULL, NULL);
    tr069_param_new(tr069, "Device.LAN.IPPingDiagnostics.FailureCount",        NULL, TR069_UNSIGNED, 0, TR069_ENABLE_READ, int_getValue, NULL, NULL, NULL);
    tr069_param_new(tr069, "Device.LAN.IPPingDiagnostics.AverageResponseTime", NULL, TR069_UNSIGNED, 0, TR069_ENABLE_READ, int_getValue, NULL, NULL, NULL);
    tr069_param_new(tr069, "Device.LAN.IPPingDiagnostics.MinimumResponseTime", NULL, TR069_UNSIGNED, 0, TR069_ENABLE_READ, int_getValue, NULL, NULL, NULL);
    tr069_param_new(tr069, "Device.LAN.IPPingDiagnostics.MaximumResponseTime", NULL, TR069_UNSIGNED, 0, TR069_ENABLE_READ, int_getValue, NULL, NULL, NULL);
}

//------------------------------------------------------------------------------
static void int_LAN_TraceRouteDiagnostics_init(struct TR069 *tr069)
{
    tr069_param_new(tr069, "Device.LAN.TraceRouteDiagnostics.", NULL, TR069_OBJECT, 0, TR069_ENABLE_READ, NULL, NULL, NULL, NULL);
    tr069_param_new(tr069, "Device.LAN.TraceRouteDiagnostics.DiagnosticsState",  "Trace_DiagnosticsState", TR069_STRING,   DEVICE_LAN_TraceRouteDiagnostics_DiagnosticsState_LEN_32, TR069_ENABLE_READ_WRITE_APPLY,      int_getValue, int_check, int_setValue, int_onChange);
    tr069_param_new(tr069, "Device.LAN.TraceRouteDiagnostics.Host",              "Trace_Host",             TR069_STRING,   DEVICE_LAN_TraceRouteDiagnostics_RouteHops_Host_LEN_256,  TR069_ENABLE_READ_WRITE_APPLY_SAVE, int_getValue, NULL,        int_setValue, int_onChange);
    tr069_param_new(tr069, "Device.LAN.TraceRouteDiagnostics.Timeout",           "Trace_Timeout",          TR069_UNSIGNED, 0,                                                        TR069_ENABLE_READ_WRITE_APPLY_SAVE, int_getValue, NULL,        int_setValue, int_onChange);
    tr069_param_new(tr069, "Device.LAN.TraceRouteDiagnostics.DataBlockSize",     "Trace_DataBlockSize",    TR069_UNSIGNED, 0,                                                        TR069_ENABLE_READ_WRITE_APPLY_SAVE, int_getValue, NULL,        int_setValue, int_onChange);
    tr069_param_new(tr069, "Device.LAN.TraceRouteDiagnostics.MaxHopCount",       "Trace_MaxHopCount",      TR069_UNSIGNED, 0,                                                        TR069_ENABLE_READ_WRITE_APPLY_SAVE, int_getValue, NULL,        int_setValue, int_onChange);
    tr069_param_new(tr069, "Device.LAN.TraceRouteDiagnostics.DSCP",              "Trace_DSCP",             TR069_UNSIGNED, 0,                                                        TR069_ENABLE_READ_WRITE_APPLY_SAVE, int_getValue, NULL,        int_setValue, int_onChange);
    tr069_param_new(tr069, "Device.LAN.TraceRouteDiagnostics.ResponseTime",      NULL,                     TR069_UNSIGNED, 0, TR069_ENABLE_READ, int_getValue, NULL, NULL, NULL);
    tr069_param_new(tr069, "Device.LAN.TraceRouteDiagnostics.NumberOfRouteHops", NULL,                     TR069_UNSIGNED, 0, TR069_ENABLE_READ, int_getValue, NULL, NULL, NULL);

    tr069_param_new(tr069, "Device.LAN.TraceRouteDiagnostics.RouteHops.",    NULL,                  TR069_OBJECT,   0, TR069_ENABLE_ADD | TR069_ENABLE_READ, NULL, NULL, NULL, NULL);

    tr069_param_virtual(tr069, "Device.LAN.TraceRouteDiagnostics.RouteHops.", "HopHost",             TR069_STRING,   0, TR069_ENABLE_READ, int_getValue, NULL);
    tr069_param_virtual(tr069, "Device.LAN.TraceRouteDiagnostics.RouteHops.", "MinimumResponseTime", TR069_UNSIGNED, 0, TR069_ENABLE_READ, int_getValue, NULL);
    tr069_param_virtual(tr069, "Device.LAN.TraceRouteDiagnostics.RouteHops.", "AverageResponseTime", TR069_UNSIGNED, 0, TR069_ENABLE_READ, int_getValue, NULL);
    tr069_param_virtual(tr069, "Device.LAN.TraceRouteDiagnostics.RouteHops.", "MaximumResponseTime", TR069_UNSIGNED, 0, TR069_ENABLE_READ, int_getValue, NULL);
}

void tr106_tr106_set_routehop(int index, struct RouteHop* routehop)
{
        strcpy(g_param->LAN.TraceRouteDiagnostics.RouteHops[index].HopHost, routehop->hophost);

        g_param->LAN.TraceRouteDiagnostics.RouteHops[index].MinimumResponseTime = routehop->minimumResponseTime;
        g_param->LAN.TraceRouteDiagnostics.RouteHops[index].AverageResponseTime = routehop->averageResponseTime;
        g_param->LAN.TraceRouteDiagnostics.RouteHops[index].MaximumResponseTime = routehop->maximumResponseTime;
}

//------------------------------------------------------------------------------
static int int_LAN_init(struct TR069 *tr069)
{
    tr069_param_new(tr069, "Device.LAN.", NULL, TR069_OBJECT, 0, TR069_ENABLE_ATTR, NULL, NULL, NULL, NULL);
    g_tr106Index->LAN_AddressingType_index = tr069_param_new(tr069, "Device.LAN.AddressingType", "AddressingType", TR069_STRING, DEVICE_LAN_AddressingType_LEN_15, TR069_ENABLE_READ_WRITE_ATTR, tr069_port_getValue, NULL, tr069_port_setValue, NULL);
    g_tr106Index->LAN_IPAddress_index      = tr069_param_new(tr069, "Device.LAN.IPAddress",      "IPAddress",      TR069_STRING, DEVICE_LAN_IPAddress_LEN_15,      TR069_ENABLE_READ_WRITE_ATTR, tr069_port_getValue, NULL, tr069_port_setValue, NULL);
    g_tr106Index->LAN_SubnetMask_index     = tr069_param_new(tr069, "Device.LAN.SubnetMask",     "SubnetMask",     TR069_STRING, DEVICE_LAN_SubnetMask_LEN_15,     TR069_ENABLE_READ_WRITE_ATTR, tr069_port_getValue, NULL, tr069_port_setValue, NULL);
    g_tr106Index->LAN_DefaultGateway_index = tr069_param_new(tr069, "Device.LAN.DefaultGateway", "DefaultGateway", TR069_STRING, DEVICE_LAN_DefaultGateway_LEN_15, TR069_ENABLE_READ_WRITE_ATTR, tr069_port_getValue, NULL, tr069_port_setValue, NULL);
    g_tr106Index->LAN_DNSServers_index     = tr069_param_new(tr069, "Device.LAN.DNSServers",     "DNSServers",     TR069_STRING, DEVICE_LAN_DNSServers_LEN_64,     TR069_ENABLE_READ_WRITE_ATTR, tr069_port_getValue, NULL, tr069_port_setValue, NULL);
    g_tr106Index->LAN_MACAddress_index     = tr069_param_new(tr069, "Device.LAN.MACAddress",     "MACAddress",     TR069_STRING, 0,                                TR069_ENABLE_READ_ATTR,       tr069_port_getValue, NULL, NULL,                NULL);

    int_LAN_DHCPOption_init(tr069);
    int_LAN_Stats_init(tr069);
    int_LAN_IPPingDiagnostics_init(tr069);
    int_LAN_TraceRouteDiagnostics_init(tr069);

    return 0;
}

//------------------------------------------------------------------------------
int tr069_tr106_init(struct TR069 *tr069)
{
    if (g_param)
        return -1;
    g_tr069 = tr069;

    g_param = (struct TR106Param *)IND_CALLOC(sizeof(struct TR106Param), 1);
    g_tr106Index = (struct TR106Index *)IND_CALLOC(sizeof(struct TR106Index), 1);

    memset(&g_currentday, 0, sizeof(struct if_stats));
    memset(&g_quarterhour, 0, sizeof(struct if_stats));

    g_currentday.begintime = (int)tr069_sec( );
    g_quarterhour.begintime = (int)tr069_sec( );

    memset(g_param, 0, sizeof(struct TR106Param));

    tr069_param_new(tr069, "Device.", NULL, TR069_OBJECT, 0, TR069_ENABLE_READ, NULL, NULL, NULL, NULL);
    tr069_param_new(tr069, "Device.DeviceSummary", NULL, TR069_STRING, 0, TR069_ENABLE_READ, tr069_port_getValue, NULL, NULL, NULL);

    int_DeviceInfo_init(tr069);
    int_ManagementServer_init(tr069);
    int_Time_init(tr069);
    int_UserInterface_init(tr069);
    int_LAN_init(tr069);

    strcpy(g_param->LAN.IPPingDiagnostics.DiagnosticsState, "None");
    strcpy(g_param->LAN.TraceRouteDiagnostics.DiagnosticsState, "None");

    if (parameterRemain) {
        tr069_param_restore_regist(tr069, "Device.ManagementServer.URL");
        tr069_param_restore_regist(tr069, "Device.ManagementServer.Username");//
        tr069_param_restore_regist(tr069, "Device.ManagementServer.Password");//�Ϻ��淶û�涨����������ֻ����DEVICE_ManagementServer_URL_INDEXʵ���в�ͨ

        tr069_param_restore_regist(tr069, "Device.LAN.AddressingType");
        tr069_param_restore_regist(tr069, "Device.LAN.IPAddress");
        tr069_param_restore_regist(tr069, "Device.LAN.SubnetMask");
        tr069_param_restore_regist(tr069, "Device.LAN.DefaultGateway");
        tr069_param_restore_regist(tr069, "Device.LAN.DNSServers");
    }
    if (SH_Inform_Enable) {
        tr069_param_boot_regist(tr069, "Device.DeviceInfo.SoftwareVersion");
        tr069_param_inform_regist(tr069, "Device.LAN.AddressingType");
        tr069_param_inform_regist(tr069, "Device.LAN.IPAddress");
    } else {
        tr069_param_boot_regist(tr069, "Device.DeviceSummary");
        tr069_param_boot_regist(tr069, "Device.DeviceInfo.ModelName");//workssys
        tr069_param_boot_regist(tr069, "Device.DeviceInfo.HardwareVersion");
        tr069_param_boot_regist(tr069, "Device.DeviceInfo.SoftwareVersion");
        tr069_param_boot_regist(tr069, "Device.DeviceInfo.UpTime");//workssys
        tr069_param_boot_regist(tr069, "Device.DeviceInfo.FirstUseDate");//workssys
        tr069_param_boot_regist(tr069, "Device.ManagementServer.UpgradesManaged");
        tr069_param_boot_regist(tr069, "Device.ManagementServer.ConnectionRequestURL");
        tr069_param_inform_regist(tr069, "Device.ManagementServer.ParameterKey");
        tr069_param_inform_regist(tr069, "Device.LAN.AddressingType");
        tr069_param_inform_regist(tr069, "Device.LAN.IPAddress");
        tr069_param_inform_regist(tr069, "Device.LAN.MACAddress");//workssys
    }
    
    return 0;
}

void tr069_tr106_reset(struct TR069 *tr069)
{
    g_param->ManagementServer.URLModifyFlag = 15;
    strcpy(g_param->ManagementServer.ConnectionRequestPath, "digest");

    tr069_param_write_attr(tr069, "Device.ManagementServer.URL",                        NOTIFICATION_ACTIVE);
    tr069_param_write_attr(tr069, "Device.ManagementServer.NATDetected",                NOTIFICATION_ACTIVE);
    tr069_param_write_attr(tr069, "Device.ManagementServer.UDPConnectionRequestAddress",NOTIFICATION_ACTIVE);
    //2011-2-16 18:28:45 ��Ϊ����Ҫ��NTP�޸ĺ��ϱ�
    tr069_param_write_attr(tr069, "Device.Time.NTPServer1",                       NOTIFICATION_ACTIVE);
    tr069_param_write_attr(tr069, "Device.Time.LocalTimeZone",                    NOTIFICATION_ACTIVE);
    tr069_param_write_attr(tr069, "Device.LAN.AddressingType",                    NOTIFICATION_ACTIVE);
    tr069_param_write_attr(tr069, "Device.ManagementServer.ConnectionRequestURL", NOTIFICATION_ACTIVE);
}

unsigned int tr069_tr106_getUnsigned(char *name)
{
    unsigned int val = 0;

    if (strncmp(name, "Device.", 7))
        TR069ErrorOut("name = %s\n", name);
    name += 7;

    if (!strncmp(name, "ManagementServer.", 17)) {
        name += 17;

        if (!strcmp(name, "URLModifyFlag"))
            val = g_param->ManagementServer.URLModifyFlag;
        else if (!strcmp(name, "PeriodicInformEnable"))
            val = g_param->ManagementServer.PeriodicInformEnable;
        else if (!strcmp(name, "PeriodicInformInterval"))
            val = g_param->ManagementServer.PeriodicInformInterval;
        else if (!strcmp(name, "PeriodicInformTime"))
            val = g_param->ManagementServer.PeriodicInformTime;
        else if (!strcmp(name, "STUNEnable"))
            val = g_param->ManagementServer.STUNEnable;
        else
            TR069ErrorOut("Device.ManagementServer. name = %s\n", name);

    } else if (!strncmp(name, "LAN.", 4)) {
        name += 4;

        if (!strncmp(name, "IPPingDiagnostics.", 18)) {
            name += 18;

            if (!strcmp(name, "NumberOfRepetitions"))
                val = g_param->LAN.IPPingDiagnostics.NumberOfRepetitions;
            else if (!strcmp(name, "Timeout"))
                val = g_param->LAN.IPPingDiagnostics.Timeout;
            else if (!strcmp(name, "DataBlockSize"))
                val = g_param->LAN.IPPingDiagnostics.DataBlockSize;
            else if (!strcmp(name, "DSCP"))
                val = g_param->LAN.IPPingDiagnostics.DSCP;
            else
                TR069ErrorOut("Device.LAN.IPPingDiagnostics. name = %s\n", name);
        } else if (!strncmp(name, "TraceRouteDiagnostics.", 22)) {
            name += 22;

            if (!strcmp(name, "Timeout"))
                val = g_param->LAN.TraceRouteDiagnostics.Timeout;
            else if (!strcmp(name, "DataBlockSize"))
                val = g_param->LAN.TraceRouteDiagnostics.DataBlockSize;
            else if (!strcmp(name, "MaxHopCount"))
                val = g_param->LAN.TraceRouteDiagnostics.MaxHopCount;
            else if (!strcmp(name, "DSCP"))
                val = g_param->LAN.TraceRouteDiagnostics.DSCP;
            else
                TR069ErrorOut("Device.LAN.TraceRouteDiagnostics. name = %s\n", name);
        } else {
            TR069ErrorOut("Device.LAN. name = %s\n", name);
        }
    } else if (!strncmp(name, "DeviceInfo.", 11)) {
        name += 11;

        if (!strcmp(name, "FirstUseDate"))
            val = g_param->DeviceInfo.FirstUseDate;
        else
            TR069ErrorOut("Device.DeviceInfo. name = %s\n", name);
    } else {
        TR069ErrorOut("Device. name = %s\n", name);
    }
    
Err:
    return val;
}

void tr069_tr106_SetParamValue(char *name, char *str, unsigned int val)
{
    if (strncmp(name, "Device.", 7))
        TR069ErrorOut("name = %s\n", name);
    name += 7;

	if (!strncmp(name, "LAN.", 4)) {
        name += 4;

        if (!strncmp(name, "IPPingDiagnostics.", 18)) {
            name += 18;
            if (!strcmp(name, "DiagnosticsState"))
                strcpy(g_param->LAN.IPPingDiagnostics.DiagnosticsState, str);
            else if (!strcmp(name, "SuccessCount"))
                g_param->LAN.IPPingDiagnostics.SuccessCount = val;
            else if (!strcmp(name, "FailureCount"))
                g_param->LAN.IPPingDiagnostics.FailureCount = val;
            else if (!strcmp(name, "AverageResponseTime"))
                g_param->LAN.IPPingDiagnostics.AverageResponseTime = val;
            else if (!strcmp(name, "MinimumResponseTime"))
                g_param->LAN.IPPingDiagnostics.MinimumResponseTime = val;
            else if (!strcmp(name, "MaximumResponseTime"))
                g_param->LAN.IPPingDiagnostics.MaximumResponseTime = val;
            else
                TR069ErrorOut("Device.LAN.IPPingDiagnostics. name = %s\n", name);
        } else if(!strncmp(name, "TraceRouteDiagnostics.", 22)) {
            name += 22;
            if (!strcmp(name, "DiagnosticsState"))
                strcpy(g_param->LAN.TraceRouteDiagnostics.DiagnosticsState, str);
            else if (!strcmp(name, "ResponseTime"))
                g_param->LAN.TraceRouteDiagnostics.ResponseTime = val;
            else if (!strcmp(name, "NumberOfRouteHops"))
                g_param->LAN.TraceRouteDiagnostics.NumberOfRouteHops = val;
            else
                TR069ErrorOut("Device.LAN.TraceRouteDiagnostics. name = %s\n", name);
        } else {
           TR069ErrorOut("Device.LAN. name = %s\n", name);
        }
    } else if (!strncmp(name, "DeviceInfo.", 11)) {
        name += 11;
        if (!strcmp(name, "FirstUseDate"))
            g_param->DeviceInfo.FirstUseDate = val;
        else
            TR069ErrorOut("Device.DeviceInfo. name = %s\n", name);
    } else if (!strncmp(name, "ManagementServer.", 17)) {
        name += 17;
        if (!strcmp(name, "URLModifyFlag"))
            g_param->ManagementServer.URLModifyFlag = val;
        else if (!strcmp(name, "PeriodicInformTime"))
            g_param->ManagementServer.PeriodicInformTime = val;
        else if (!strcmp(name, "PeriodicInformTime"))
            g_param->ManagementServer.PeriodicInformTime = val;
        else if (!strcmp(name, "ConnectionRequestURL"))
            strcpy(g_param->ManagementServer.ConnectionRequestURL, str);
        else if (!strcmp(name, "UDPConnectionRequestAddress"))
            strcpy(g_param->ManagementServer.UDPConnectionRequestAddress, str);
        else
            TR069ErrorOut("Device.ManagementServer. name = %s\n", name);
    } else {
           TR069ErrorOut("Device. name = %s\n", name);
    }
Err:
    return;
}

int tr069_tr106_getString(char *name, char *buf, int size)
{
    if (strncmp(name, "Device.", 7))
        TR069ErrorOut("name = %s\n", name);
    name += 7;

    if (!strncmp(name, "ManagementServer.", 17)) {
        name += 17;

        if (!strcmp(name, "ConnectionRequestPath"))
            strcpy(buf, g_param->ManagementServer.ConnectionRequestPath);
        else if (!strcmp(name, "URL"))
            strcpy(buf, g_param->ManagementServer.URL);
        else if (!strcmp(name, "URLBackup"))
            strcpy(buf, g_param->ManagementServer.URLBackup);
        else if (!strcmp(name, "Username"))
            strcpy(buf, g_param->ManagementServer.Username);
        else if (!strcmp(name, "Password"))
            strcpy(buf, g_param->ManagementServer.Password);
        else if (!strcmp(name, "ConnectionRequestURL"))
            strcpy(buf, g_param->ManagementServer.ConnectionRequestURL);
        else if (!strcmp(name, "ConnectionRequestUsername"))
            strcpy(buf, g_param->ManagementServer.ConnectionRequestUsername);
        else if (!strcmp(name, "ConnectionRequestPassword"))
            strcpy(buf, g_param->ManagementServer.ConnectionRequestPassword);
        else
            TR069ErrorOut("Device.ManagementServer. name = %s\n", name);

    } else if (!strncmp(name, "LAN.", 4)) {
        name += 4;

        if (!strcmp(name, "IPPingDiagnostics.Host"))
            strcpy(buf, g_param->LAN.IPPingDiagnostics.Host);
        else if (!strcmp(name, "TraceRouteDiagnostics.Host"))
            strcpy(buf, g_param->LAN.TraceRouteDiagnostics.Host);
        else
            TR069ErrorOut("Device.LAN. name = %s\n", name);
    } else {
        TR069ErrorOut("Device. name = %s\n", name);
    }

    return 0;
Err:
    return -1;
}
