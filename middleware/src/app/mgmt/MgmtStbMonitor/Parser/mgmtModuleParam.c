#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "sys/types.h"
#include "sys/stat.h"

#include "AppSetting.h"
#include "SysSetting.h"
#include "VersionSetting.h"
#include "SettingModuleNetwork.h"
#include "mgmtModuleParam.h"
#include "mid/mid_tools.h"
#include "mid/mid_time.h"
#include "config.h"
#include "BrowserAgentTakin.h"
#include "Tr069.h"
#ifdef TVMS_OPEN
#include "tvms_setting.h"
#endif
#include "mid_sys.h"
#include "TAKIN_browser.h"
#include "TAKIN_setting_type.h"
#include "mid_stream.h"
#include "app_sys.h"
#include "LogModuleHuawei.h"
#include "TerminalControl.h"
#include "Session.h"
#include "Verimatrix.h"

#include "customer.h"

#include "Tr069Setting.h"
#include "stbinfo/stbinfo.h"

#define URL_LEN 512
 int MgmtHomepageRead(char  *buf, int len)
{
    char HomePage[URL_LEN] = {0};

    sysSettingGetString("eds", HomePage, URL_LEN, 0);
    sprintf(buf, "%s", HomePage);
    return 0;
}
int MgmtHomepageWrite(char *buf, int len)
{
   if (!buf || (strncmp(buf, "http://", 7) != 0))
        return -1;

    sysSettingSetString("eds", buf);
    settingManagerSave();
    return 0;
}
int MgmtHomepageBackupRead(char  *buf, int len)
{
    char HomepageBackup[URL_LEN] = {0};

    sysSettingGetString("eds1", HomepageBackup, URL_LEN, 0);
    sprintf(buf, "%s", HomepageBackup);
    return 0;
}
int MgmtHomepageBackupWrite(char *buf, int len)
{
    if (!buf || (strncmp(buf, "http://", 7) != 0))
        return -1;

    sysSettingSetString("eds1", buf);
    settingManagerSave();
    return 0;
}
 int MgmtNTPRead(char *buf, int len)
{
    char NtpUrl[URL_LEN] = {0};

    sysSettingGetString("ntp", NtpUrl, URL_LEN, 0);

    if(strlen(NtpUrl) == 0)
        strcpy(NtpUrl, "(void *)");

    sprintf(buf, "%s", NtpUrl);
    return 0;
}

 int MgmtNTPWrite(char *buf, int len)
{
    sysSettingSetString("ntp", buf);
    settingManagerSave();
    return 0;
}
 int MgmtNTPBackupRead(char *buf, int len)
{
    char NtpUrlBackup[URL_LEN] = {0};

    sysSettingGetString("ntp1", NtpUrlBackup, URL_LEN, 0);

    if(strlen(NtpUrlBackup) == 0)
        strcpy(NtpUrlBackup, "(void *)");

    sprintf(buf, "%s", NtpUrlBackup);
    return 0;
}

 int MgmtNTPBackupWrite(char *buf, int len)
{
    sysSettingSetString("ntp1", buf);
    settingManagerSave();
    return 0;
}
int MgmtNetUserIDRead(char *buf, int len)
{
    char NetUserID[URL_LEN] = {0};

    sysSettingGetString("netuser", NetUserID, 32, 0);
    sprintf(buf, "%s", NetUserID);
    return 0;
}

int MgmtNetUserIDWrite(char *buf, int len)
{
    sysSettingSetString("netuser", buf);
    settingManagerSave();
    return 0;
}

/************************************************************
* ntvuseraccount    业务账号，支持读取、写入
*************************************************************/
int MgmtNtvUserAccountRead(char * buf, int len)
{
    char NtvUseraAccount[URL_LEN] = {0};

    //sysSettingGetString("netuser", NtvUseraAccount, 32, 0);
    appSettingGetString("ntvuser", NtvUseraAccount, 32, 0);

    sprintf(buf, "%s", NtvUseraAccount);
    return 0;
}

int MgmtNtvUserAccountWrite(char * buf, int len)
{
    appSettingSetString("ntvuser", buf);
    settingManagerSave();
    return 0;
}

/************************************************************
*ntvuserpassword  业务密码，支持更改，不支持从客户端读出。
*************************************************************/
int MgmtNtvUserPasswordRead(char * buf, int len)
{
    appSettingGetString("ntvAESpasswd", buf, len, 0);
    return 0;
}

int MgmtNtvUserPasswordWrite(char * buf, int len)
{
    appSettingSetString("ntvAESpasswd", buf);
    settingManagerSave();
    return 0;
}

/************************************************************
*netuserpassword  pppoe接入密码，支持更改，不支持从客户端读出。
*************************************************************/
int MgmtNetUserPasswordRead(char * buf, int len)
{
    sysSettingGetString("netAESpasswd", buf, len, 0);
    return 0;
}

int MgmtNetUserPasswordWrite(char * buf, int len)
{
    sysSettingSetString("netAESpasswd", buf);
    settingManagerSave();
    return 0;
}

/************************************************************
* dhcpaccount    dhcp接入账号，支持读取、写入
*************************************************************/
int MgmtDhcpAccountRead(char * buf, int len)
{
    sysSettingGetString("dhcpuser", buf, len, 0);
    return 0;
}

int MgmtDhcpAccountWrite(char * buf, int len)
{
    sysSettingSetString("dhcpuser", buf);
    settingManagerSave();
    return 0;
}

/************************************************************
* dhcpaccount    dhcp接入密码，支持读取、写入
*************************************************************/
int MgmtDhcpPasswordRead(char * buf, int len)
{
    sysSettingGetString("ipoeAESpasswd", buf, len, 0);
    return 0;
}

int MgmtDhcpPasswordWrite(char * buf, int len)
{
    sysSettingSetString("ipoeAESpasswd", buf);
    settingManagerSave();
    return 0;
}

/************************************************************
* connecttype   1：PPPoE，2：DHCP，3：StaticIP
************************************************************/
int MgmtConnectTypeRead(char * buf, int len)
{
    int connectType = -1;
    sysSettingGetInt("connecttype", &connectType, 0);
    switch(connectType) {
	case NETTYPE_PPPOE:
	    sprintf(buf, "%s", "PPPOE");
		break;
	case NETTYPE_DHCP:
	    sprintf(buf, "%s", "DHCP");
		break;
	case NETTYPE_STATIC:
	    sprintf(buf, "%s", "Static");
		break;
//	case NETTYPE_DHCP_ENCRYPT:
//	case NETTYPE_DHCP_DBLVLAN:
//	case NETTYPE_DBL_STACK:
//	    sprintf(buf, "%s", "DHCP");
//		break;
	default:
		return -1;
    }
	return 0;
}

int MgmtConnectTypeWrite(char * buf, int len)
{
    if(strncmp(buf,"PPPOE",5) == 0)
        sysSettingSetInt("connecttype", NETTYPE_PPPOE);
	else if(strncmp(buf,"DHCP",4) == 0)
        sysSettingSetInt("connecttype", NETTYPE_DHCP);
    else if(strncmp(buf,"Static",6) == 0)
        sysSettingSetInt("connecttype", NETTYPE_STATIC);
    else
		return -1;
	settingManagerSave();
    return 0;
}

int MgmtIpAddressRead(char * buf, int len)
{
    char ifname[URL_LEN] = { 0 };
    network_default_ifname(ifname, URL_LEN);
    network_address_get(ifname, buf, len);
    return 0;
}

int MgmtIpAddressWrite(char * buf, int len)
{
    sysSettingSetString("ip", buf);
    settingManagerSave();
    return 0;
}

int MgmtNetmaskRead(char * buf, int len)
{
    char ifname[URL_LEN] = { 0 };
    network_default_ifname(ifname, URL_LEN);
    network_netmask_get(ifname, buf, len);
    return 0;
}

int MgmtNetmaskWrite(char * buf, int len)
{
    sysSettingSetString("netmask", buf);
    settingManagerSave();
    return 0;
}

int MgmtNetGatewayRead(char * buf, int len)
{
    char ifname[URL_LEN] = { 0 };
    network_default_ifname(ifname, URL_LEN);
    network_gateway_get(ifname, buf, len);
	return 0;
}

int MgmtNetGatewayWrite(char * buf, int len)
{
    sysSettingSetString("gateway", buf);
    settingManagerSave();
	return 0;
}

/************************************************************
* DNS   主DNS   在客户端的主端口中只需要显示主DNS。
*************************************************************/
int MgmtNetDns1Read(char * buf, int len)
{
    char ifname[URL_LEN] = { 0 };
    network_default_ifname(ifname, URL_LEN);
    network_dns0_get(ifname, buf, len);
	return 0;
}

int MgmtNetDns1Write(char * buf, int len)
{
    sysSettingSetString("dns", buf);
    settingManagerSave();
    return 0;
}

/************************************************************
* DNS1  备DNS   在客户端的主端口中只需要显示备DNS。
*************************************************************/
int MgmtNetDnsBackupRead(char * buf, int len)
{
    char ifname[URL_LEN] = { 0 };
    network_default_ifname(ifname, URL_LEN);
    network_dns1_get(ifname, buf, len);
    return 0;
}

int MgmtNetDnsBackupWrite(char * buf, int len)
{
    sysSettingSetString("dns1", buf);
    settingManagerSave();
    return 0;
}

/************************************************************
* defContAcc    支持读取、写入
*************************************************************/
int MgmtdefContAccRead(char * buf, int len)
{
    char defContAcc[URL_LEN] = {0};

	appSettingGetString("defContAcc", defContAcc, URL_LEN, 0);
    sprintf(buf, "%s", defContAcc);
    return 0;
}

int MgmtdefContAccWrite(char * buf, int len)
{
    appSettingSetString("defContAcc", buf);
    settingManagerSave();
    return 0;
}

/************************************************************
*defContPwd 支持更改，不支持从客户端读出。
*************************************************************/
int MgmtdefContPwdWrite(char * buf, int len)
{
    appSettingSetString("defAESContpwd", buf);
    settingManagerSave();
    return 0;
}

/************************************************************
* directplay    0：不支持； 1：支持；2：直接进DVB
*************************************************************/
int MgmtDirectplayRead(char * buf, int len)
{
    int directPlay = 0;
    sysSettingGetInt("lastChannelPlay", &directPlay, 0);
    sprintf(buf, "%d", directPlay);
    return 0;
}

int MgmtDirectplayWrite(char * buf, int len)
{
    sysSettingSetInt("lastChannelPlay", atoi(buf));
    settingManagerSave();
    return 0;
}

/************************************************************
* epgurl    当前的EPG地址   支持读取
*************************************************************/
int MgmtEpgurlRead(char * buf, int len)
{
    appSettingGetString("epg", buf, len, 0);
	return 0;
}

int MgmtEpgurlWrite(char * buf, int len)
{
    appSettingSetString("epg", buf);
    return 0;
}

/************************************************************
* timezone  当前的timezone地址  支持读取
*************************************************************/
int MgmtTimezoneRead(char * buf, int len)
{
    int tTimezone = -1;

	sysSettingGetInt("timezone", &tTimezone, 0);
    sprintf(buf, "%d", tTimezone);
    return 0;
}

int MgmtTimezoneWrite(char * buf, int len)
{
    int zone;
    char *pos1 = NULL;
	char *pos2 = NULL;
	char *pos3 = NULL;
	char tmp[8] = {0};

	pos1 = strchr(buf, '+');
	pos2 = strchr(buf, '-');
	pos3 = strchr(buf, ':');
	if(pos1 == NULL&& pos2 == NULL)
		return -1;
	if(pos1 != NULL){
	    if(pos3 != NULL)
			strncpy(tmp,pos1 + 1,(pos3 - pos1));
		else
			strncpy(tmp,pos1 + 1,8);
		zone = atoi(tmp);
	}
	if(pos2 != NULL){
		if(pos3 != NULL)
			strncpy(tmp,pos2 + 1,(pos3 - pos2));
		else
			strncpy(tmp,pos2 + 1,8);
	    zone = - atoi(tmp);
	}
    sysSettingSetInt("timezone", zone);
    settingManagerSave();
    return 0;
}

/************************************************************
* time  当前的系统时间  支持读取
*************************************************************/
int MgmtLocalTimeRead(char * buf, int len)
{
    char temp[URL_LEN] = {0};
    int sec = 0;

    sec = mid_time();
    if(MID_UTC_SUPPORT != 0) {
        sec += mid_get_times_sec();
    }
    mid_tool_time2string(sec, temp, 0);
    sprintf(buf, "%s", temp);
    return 0;
}

int  MgmtCAregisterRead(char * buf, int len)
{
    int caFlag = 0;
	appSettingGetInt("nCAFlag", &caFlag, 0);
    switch(caFlag) {
    case CA_FLAG_NOT_CA_ACCOUNT:
        strncpy(buf, "0", 1);
        break;
    case CA_FLAG_SECURE_MEDIA:
        strncpy(buf, "1", 1);
        break;
    case CA_FLAG_IRDETO_HARD:
        strncpy(buf, "2", 1);
        break;
    case CA_FLAG_IRDETO_SOFT:
        strncpy(buf, "3", 1);
        break;
    case CA_FLAG_VERIMATRIX:
        strncpy(buf, "4", 1);
        break;
    default:
        return -1;
    }
    return 0;
}

int MgmtCAregisterWrite(char * buf, int len)
{
    if(appSettingSetInt("nCAFlag", atoi(buf)) == -1) {
        printf("sysCAFlagSet error!\n");
        return -1;
    } else {
        settingManagerSave();
        return 0;
    }
}

int MgmtCAModeRead(char* buf, int len)
{
    ca_vm_get_type(buf);

    return 0;
}


/************************************************************
* LogServerUrl  日志服务器地址  支持读取、写入
*************************************************************/

int MgmtLogServerUrlRead(char * buf, int len)
{
    char LogServerUrl[URL_LEN] = {0};

    tr069StatisticGetLogServerUrl(LogServerUrl, 256);
    sprintf(buf, "%s", LogServerUrl);
    return 0;
}

int MgmtLogServerUrlWrite(char * buf, int len)
{
    tr069_statistic_setConfiguration("LogServerUrl", buf, len);

    tr069StatisticConfigSave();
    return 0;
}

/************************************************************
* LogRecordInterval 日志记录周期    支持读取、写入
*************************************************************/
int MgmtLogRecordIntervalRead(char * buf, int len)
{
    sprintf(buf, "%s", tr069StatisticGetLogRecordInterval());
    return 0;
}

int MgmtLogRecordIntervalWrite(char * buf, int len)
{

    tr069_statistic_setConfiguration("LogRecordInterval", buf, len);

    tr069StatisticConfigSave();
    return 0;
}

/************************************************************
* LogUploadInterval 日志上报周期    支持读取、写入
*************************************************************/
int MgmtLogUploadIntervalRead(char * buf, int len)
{
    sprintf(buf, "%d", tr069StatisticGetLogUploadInterval());
    return 0;
}
int MgmtLogUploadIntervalWrite(char * buf, int len)
{
    tr069_statistic_setConfiguration("LogUploadInterval", buf, len);

    tr069StatisticConfigSave();
    return 0;
}
 int MgmtQosLogSwitchRead(char * buf, int len)
{
    int flag = 0;

    sysSettingGetInt("logSend", &flag, 0);
    sprintf(buf, "%d", flag);
    return 0;
}

int MgmtQosLogSwitchWrite(char * buf, int len)
{
    sysSettingSetInt("logSend", atoi(buf));
    settingManagerSave();
    return 0;
}

#ifdef TVMS_OPEN
/************************************************************
*TVMSGWIP   TVMS的GateWay服务器集群的IP地址 支持读取、写入
*************************************************************/
int MgmtTVMSGWIPRead(char * buf, int len)
{
    char TVMSGWIP_buf[URL_LEN] = {0};

    //tvms_conf_tvmsgwip_get(TVMSGWIP_buf); //old linux monitor have del

    sprintf(buf, "%s", TVMSGWIP_buf);
    return 0;
}

int MgmtTVMSGWIPWrite(char * buf, int len)
{
    printf("Can't support this function !\n");
    return 0;
}

/************************************************************
*TVMSHeartbitInterval   TVMS的心跳周期，单位为秒    支持读取、写入
*************************************************************/
int MgmtTVMSHeartbitIntervalRead(char * buf, int len)
{
    char TVMSHeartbitInterval[URL_LEN] = {0};
    int tTime = 0;

    tvms_conf_tvmsheartbitinterval_get(&tTime);
    sprintf(buf, "%d", tTime);

    return 0;
}

int MgmtTVMSHeartbitIntervalWrite(char * buf, int len)
{
    tvms_conf_tvmsheartbitinterval_set(atoi(buf));
    return 0;
}

/************************************************************
* TVMSDelayLength   TVMS的消息冲突时的展示延迟时长  支持读取、写入
*************************************************************/
int  MgmtTVMSDelayLengthRead(char * buf, int len)
{
    char TVMSDelayLength[URL_LEN] = {0};
    int tTime = 0;

    tvms_conf_tvmsdelaylength_get(&tTime);
    sprintf(buf, "%d", tTime);
    return 0;
}

int  MgmtTVMSDelayLengthWrite(char * buf, int len)
{
    tvms_conf_tvmsdelaylength_set(atoi(buf));
    return 0;
}

/************************************************************
* TVMSHeartbitUrl   TVMS的消息心跳地址  支持读取、写入
*************************************************************/
int  MgmtTVMSHeartbitUrlRead(char * buf, int len)
{
    char TVMSHeartbitUrl[URL_LEN] = {0};

    tvms_conf_tvmsheartbiturl_get(TVMSHeartbitUrl);
    snprintf(buf, len, "%s", TVMSHeartbitUrl);
    return 0;
}

int  MgmtTVMSHeartbitUrlWrite(char * buf, int len)
{
    tvms_conf_tvmsheartbiturl_set(buf);
    return 0;
}

/************************************************************
* TVMSVODHeartbitUrl    TVMS的VOD消息心跳地址   支持读取、写入
*************************************************************/
int  MgmtTVMSVODHeartbitUrlRead(char * buf, int len)
{
    char TVMSVODHeartbitUrl[URL_LEN] = {0};

    tvms_conf_tvmsvodheartbiturl_get(TVMSVODHeartbitUrl);

    sprintf(buf, "%s", TVMSVODHeartbitUrl);
    return 0;
}

int  MgmtTVMSVODHeartbitUrlWrite(char * buf, int len)
{
    tvms_conf_tvmsvodheartbiturl_set(buf);
    return 0;
}
#endif

/************************************************************
* templateName  EPG模版名称 支持读取、写入
*************************************************************/
int  MgmtTemplateNameRead(char * buf, int len)
{
    char templateName[URL_LEN] = {0};

	appSettingGetString("templateName", templateName, URL_LEN, 0);

    sprintf(buf, "%s", templateName);
    return 0;
}

int  MgmtTemplateNameWrite(char * buf, int len)
{
    appSettingSetString("templateName", buf);
    settingManagerSave();
    return 0;
}

/************************************************************
* areaid    areaid  支持读取、写入
*************************************************************/
int  MgmtareaidRead(char * buf, int len)
{
    char templateName[URL_LEN] = {0};

	appSettingGetString("areaid", templateName, URL_LEN, 0);

    sprintf(buf, "%s", templateName);
    return 0;
}

int  MgmtareaidWrite(char * buf, int len)
{
    appSettingSetString("areaid", buf);
    settingManagerSave();
    return 0;
}

#if INCLUDE_TR069
// TMS相关已经替换成原linux的实现
/************************************************************
*TMSURL 终端网管URL 支持读取、写入
*************************************************************/
int  MgmtTMSURLRead(char * buf, int len)
{
    TR069_API_GETVALUE("Device.ManagementServer.URL", buf, len);
    return 0;
}
int  MgmtTMSURLWrite(char * buf, int len)
{
#if defined(HUAWEI_C10) && !defined(Sichuan)
   unsigned int flag = 0;

   char str[16];
   str[0] = 0;
   TR069_API_GETVALUE("Device.ManagementServer.URLModifyFlag", str, 16);
   flag = atoi(str);

   if(!(flag & 0x08))
      return -1;
#endif

   TR069_API_SETVALUE("Device.ManagementServer.URL", buf, len);
   TR069_API_SETVALUE("Config.Save", "", 1);

   return 0;
}

/************************************************************
* TMSUsername   终端网管用户    支持读取、写入
*************************************************************/
int  MgmtTMSUsernameRead(char* buf, int len)
{
    TR069_API_GETVALUE("Device.ManagementServer.Username", buf, len);
    return 0;
}
int  MgmtTMSUsernameWrite(char* buf, int len)
{
   TR069_API_SETVALUE("Device.ManagementServer.Username", buf, len);
   TR069_API_SETVALUE("Config.Save", "", 1);
   return 0;
}

/************************************************************
* TMSPassword   终端网管密码    支持读取
*************************************************************/
int  MgmtTMSPasswordRead(char * buf, int len)
{
    TR069_API_GETVALUE("Device.ManagementServer.Password", buf, len);
    return 0;
}
int  MgmtTMSPasswordWrite(char * buf, int len)
{
    TR069_API_SETVALUE("Device.ManagementServer.Password", buf, len);
    TR069_API_SETVALUE("Config.Save", "", 1);
    return 0;
}

/************************************************************
* TMSHeartBit   终端网管标记    0－不支持，1－支持，支持读取、写入
*************************************************************/
int  MgmtTMSEnableRead(char * buf, int len)
{
    int tr069Enable = 0;
    sysSettingGetInt("tr069_enable", &tr069Enable, 0);
    sprintf(buf, "%d", tr069Enable);
    return 0;
}
int  MgmtTMSEnableWrite(char * buf, int len)
{
    sysSettingSetInt("tr069_enable", atoi(buf));
    settingManagerSave();
    return 0;
}

int  MgmtTMSHeartBitRead(char * buf, int len)
{
    TR069_API_GETVALUE("Device.ManagementServer.PeriodicInformEnable", buf, len);
    return 0;
}
int  MgmtTMSHeartBitWrite(char * buf, int len)
{
    TR069_API_SETVALUE("Device.ManagementServer.PeriodicInformEnable", buf, len);
    TR069_API_SETVALUE("Config.Save", NULL, 1);
    return 0;
}

int MgmtTMSHeartBitIntervalRead(char * buf, int len)
{
    TR069_API_GETVALUE("Device.ManagementServer.PeriodicInformInterval", buf, len);
    return 0;
}
// 这个周期应该保存在哪个文件里呀?
int MgmtTMSHeartBitIntervalWrite(char * buf, int len)
{
    TR069_API_SETVALUE("Device.ManagementServer.PeriodicInformInterval", buf, len);
    return 0;
}
#endif // INCLUDE_TR069

/************************************************************
*SupportHD  是否支持高清播放    0－不支持，1－支持，支持读取、写入
*************************************************************/
int  MgmtSupportHDRead(char * buf, int len)
{
#if(SUPPORTE_HD)
    sprintf(buf, "%d", 1);
#else
    sprintf(buf, "%d", 0);
#endif
    return 0;
}

int  MgmtSupportHDWrite(char * buf, int len)
{
    printf("Can't support this function !\n");
    return 0;
}
int  MgmtAspectRatioRead(char * buf, int len)
{
    int aspectRatioaspectRatio = -1;

	appSettingGetInt("hd_aspect_mode", &aspectRatioaspectRatio,0);
    sprintf(buf, "%d", aspectRatioaspectRatio);
	return 0;
}

int  MgmtAspectRatioWrite(char * buf, int len)
{
    appSettingSetInt("hd_aspect_mode", atoi(buf));
    settingManagerSave();

	return 0;
}

int  MgmtSDVideoStandardRead(char * buf, int len)
{
    int ret;

    sysSettingGetInt("videoformat",&ret,0);
    switch(ret) {
    case 0:
        strcpy(buf, "NTSC");
        break;
    case 1:
    case 7:
        strcpy(buf, "PAL");
        break;
    case 2:
    case 8:
        strcpy(buf, "NTSC");
        break;
    case 3:
        strcpy(buf, "NTSC");
        break;
    case 4:
        strcpy(buf, "PAL");
        break;
    default:
        break;
    }
    return 0;
}

int  MgmtSDVideoStandardWrite(char * buf, int len)
{
    if(strncmp(buf, "PAL", strlen("PAL")) == 0)
        sysSettingSetInt("videoformat",7);
    else if(strncmp(buf, "NTSC", strlen("NTSC")) == 0)
        sysSettingSetInt("videoformat",8);

    settingManagerSave();
    return 0;
}

int  MgmtHDVideoStandardRead(char * buf, int len)
{
    int ret = -1;

#if 0//standard video
    sprintf(buf,"%d",0);
#else
    sysSettingGetInt("videoformat",&ret,0);
    sprintf(buf,"%d",ret);
#endif
    return 0;
}

int  MgmtHDVideoStandardWrite(char * buf, int len)
{
    sysSettingSetInt("videoformat",atoi(buf));
    settingManagerSave();
    return 0;
}

/************************************************************
* Mgmtmodel  机顶盒型号  支持读取、写入
*************************************************************/
int  MgmtProductClassRead(char * buf, int len)
{
    sprintf(buf, "%s", StbUpgradeModel());
    return 0;
}

int  MgmtProductClassWrite(char * buf, int len)
{
    char *str = NULL;
    char ch[2] = {0};

    str = buf;
    if(strcmp(str, "EC2108") == 0)
        ch[0] = 'A';
    else  if(strcmp(str, "EC2108G") == 0)
        ch[0] = 'B';
    else  if(strcmp(str, "EC2118") == 0)
        ch[0] = 'C';
    else  if(strcmp(str, "EC2118S") == 0)
        ch[0] = 'D';
    else  if(strcmp(str, "EC5108") == 0)
        ch[0] = 'E';
    else {
        printf("the para is error!");
        return -1;
    }
	sysSettingSetString("Mgmtsofttype", ch);
    settingManagerSave();
    return 0;
}

/*******************softwareversion*******************************/
int  MgmtSoftwareVersionRead(char * buf, int len)
{
    char SoftwareVersion[URL_LEN] = {0};

    get_upgrade_version(SoftwareVersion);

    sprintf(buf, "%s", SoftwareVersion);
    return 0;
}

int  MgmtSoftwareVersionWrite(char * buf, int len)
{
    printf("Can't support this function !\n");
    return 0;
}

/***************华为版本*********************************/
int  MgmtHWVersionRead(char * buf, int len)
{
    sprintf(buf, "%s", HwSoftwareVersion(1));
    return 0;
}

/*******************CompTime*******************************/
int  MgmtComptimeRead(char * buf, int len)
{
    char CompTime[URL_LEN] = {0};
    sprintf(buf, "%s", StbVersionBuildtime());
    printf("[%s %s %d]comptime:%s\n", __FILE__, __FUNCTION__, __LINE__, buf);
    return 0;
}

int  MgmtComptimeWrite(char * buf, int len)
{
    printf("Can't support this function !\n");
    return 0;
}

/*******************hardwareversion*******************************/
int  MgmtHardwareVersionRead(char * buf, int len)
{
    char HWversion[URL_LEN] = {0};
    char HardType[12] = {0};
    HardwareVersion(HWversion, 12);
    sprintf(buf, "%s", HWversion);
    return 0;
}

int  MgmtHardwareVersionWrite(char * buf, int len)
{
    printf("Can't support this function !\n");
    return 0;
}

/*******************browserversion*******************************/
int  MgmtBrowserVersionRead(char * buf, int len)
{
    int tBrowserVersion = 0;
    char *tBrowserBuildTime = NULL;
    char *tBrowserBuilder = NULL;

    epgBrowserAgentGetTakinVersion(&tBrowserVersion, &tBrowserBuildTime, &tBrowserBuilder);

    sprintf(buf, "%d", tBrowserVersion);
    printf("BrowserVersion = %d\n", tBrowserVersion);
    return 0;
}

int  MgmtBrowserVersionWrite(char * buf, int len)
{
    printf("Can't support this function !\n");
    return 0;
}

/**********************************************************************/
int  MgmtBrowserTimeRead(char * buf, int len)
{
    int tBrowserVersion = 0;
    char *tBrowserBuildTime = NULL;
    char *tBrowserBuilder = NULL;

    epgBrowserAgentGetTakinVersion(&tBrowserVersion, &tBrowserBuildTime, &tBrowserBuilder);

    sprintf(buf, "%s", tBrowserBuildTime);
    return 0;
}

int  MgmtSerialNumberRead(char * buf, int len)
{
    char Serial[34] = {0};

    mid_sys_serial(Serial);
    sprintf(buf, "%s", Serial);
    return 0;
}

int  MgmtSerialNumberWrite(char * buf, int len)
{
    int i = 0;
    char *buf32 = buf;

    for(i = 0; i < 32; i++) {
        if((*(buf32 + i) >= '0' && *(buf32 + i) <= '9') || (*(buf32 + i) >= 'a' && *(buf32 + i) <= 'z') || (*(buf32 + i) >= 'A' && *(buf32 + i) <= 'Z')) {
            continue;
        } else {
            return -1;
        }
    }
    if(strlen(buf) != 32) {
        printf("BUF != 32\n");
        return -1;
    } else {
        char sn17[20] = {0};
        char serial_code[34] = {0};


        printf("Serial code(%s)\n", (buf));
        strncpy(serial_code, (buf), 34);
//wankang
   //     SN32toSN17(serial_code, sn17);
        mid_sys_serial_set(sn17);
    }
    return 0;
}
int MgmtChipSeriaNumberRead(char * buf, int len)
{
   buf[0] = '1';
   return 0;
}

int MgmtChipSeriaNumberWrite(char * buf, int len)
{
   return 0;
}

int  MgmtMACAddressRead(char * buf, int len)
{
    char ifname[URL_LEN] = { 0 };
    char ifmac[URL_LEN] = { 0 };
    network_default_ifname(ifname, URL_LEN);
    const char* pmac = network_ifacemac_get(ifname, ifmac, URL_LEN);
    if (pmac)
        snprintf(buf, len, "%02x:%02x:%02x:%02x:%02x:%02x", pmac[0], pmac[1], pmac[2], pmac[3], pmac[4], pmac[5]);
    return 0;
}

int  MgmtUpgradeUrlRead(char * buf, int len)
{
    char upgradeUrl[URL_LEN] = {0};
    sysSettingGetString("upgradeUrl",upgradeUrl,URL_LEN, 0);
    if(strlen(upgradeUrl) == 0)
        strcpy(upgradeUrl, "NULL");
    sprintf(buf, "%s", upgradeUrl);
    return 0;
}

int  MgmtUpgradeUrlWrite(char * buf, int len)
{
    sysSettingSetString("upgradeUrl",buf);
    settingManagerSave();
    return 0;
}

/******************log*************************************/
int  MgmtSysLogLevelRead(char * buf, int len)
{
    int level = 0;
    char temp[32] = {0};

    level = huaweiGetLogLevel();
    sprintf(buf, "%d", level);
    return 0;
}

int  MgmtSysLogLevelWrite(char * buf, int len)
{
    int level = 0 ;

    level = atoi(buf);
    huaweiSetLogLevel(level);

    return 0;
}

int  MgmtSysLogTypeRead(char * buf, int len)
{
    sprintf(buf, "%02d", huaweiGetLogType());
    return 0;
}

int  MgmtSysLogTypeWrite(char * buf, int len)
{
    int level = 0;

    level = atoi(buf);
    huaweiSetLogType(level);
    huaweiLog();

	return 0;
}

int  MgmtSysLogFtpServerRead(char * buf, int len)
{
    char temp[512] = {0};
    huaweiGetLogFTPServer(temp, 256);
    sprintf(buf, "%s", temp);
    return 0;
}

int  MgmtSysLogFtpServerWrite(char * buf, int len)
{
	huaweiSetLogFTPServer(buf);

    return 0;
}

int  MgmtSysLogServerRead(char * buf, int len)
{
    char temp[512] = {0};

    huaweiGetLogUDPServer(temp,256);
    sprintf(buf, "%s", temp);
    return 0;
}

int  MgmtSysLogServerWrite(char * buf, int len)
{
    char* port = strchr(buf, ':');
    if (!port)
        strcat(buf, ":514");
	huaweiSetLogUDPServer(buf);
    return 0;
}

int  MgmtSysLogOutPutTypeRead(char * buf, int len)
{
    sprintf(buf, "%02d", huaweiGetLogOutPutType());
    return 0;
}

int  MgmtSysLogOutPutTypeWrite(char * buf, int len)
{
    int level = 0 ;

    level = atoi(buf);
	huaweiSetLogOutPutType(level);

    return 0;
}

int  MgmtWorkModelRead(char * buf, int len)
{
    int workModel = 1;
    //workModel = sysGetWorkModel();
    switch(workModel){
    case 1:
        strncpy(buf,"set_work_mode",sizeof("set_work_mode"));
        break;
    case 2:
        strncpy(buf,"set_test_mode",sizeof("set_test_mode"));
        break;
    case 3:
        strncpy(buf,"set_autotest_mode",sizeof("set_autotest_mode"));
        break;
    case 4:
        strncpy(buf,"set_scriptrecord_mode",sizeof("set_scriptrecord_mode"));
        break;
    default:
        return -1;
    }
    return 0;
}

int  MgmtWorkModelWrite(char * buf, int len)
{
   // if(strncmp(buf,"set_work_mode",sizeof("set_work_mode")))
   //     sysSettingSetString();
    return 0;
}

int  MgmtChannelSwitchModeRead(char * buf, int len)
{
    int ret = -1;
    sysSettingGetInt("changevideomode", &ret, 0);
    sprintf(buf,"%d",ret);

    return 0;
}

int  MgmtChannelSwitchModeWrite(char * buf, int len)
{
    sysSettingSetInt("changevideomode", atoi(buf));
    settingManagerSave();
    return 0;
}

int  MgmtAntiFlickerSwitchRead(char * buf, int len)
{
    char HDVideoStandard[URL_LEN] = {0};

    sysSettingGetString("Aniflicker", HDVideoStandard, URL_LEN, 0);
    sprintf(buf, "%s", HDVideoStandard);
    return 0;
}

int  MgmtAntiFlickerSwitchWrite(char * buf, int len)
{
    sysSettingSetString("Aniflicker", buf);
    settingManagerSave();
    return 0;
}

int  MgmtTransportProtocolRead(char * buf, int len)
{
    char tpbuf[URL_LEN];

	sysSettingGetString("TransportProtocol", tpbuf, URL_LEN, 0);
    sprintf(buf, "%s", tpbuf);

	return 0;
}

int  MgmtTransportProtocolWrite(char * buf, int len)
{
    int protocols = atoi(buf);
    if(protocols >= 0 && protocols < 4) {
		sysSettingSetInt("TransportProtocol", protocols);
        mid_stream_transport(protocols);
        settingManagerSave();
        return 0;
    }
    return -1;
}


int  MgmtSSHServiceEnableRead(char * buf, int len)
{
    sprintf(buf, "%s", getSSHState());
    return 0;
}

int  MgmtSSHServiceEnableWrite(char * buf, int len)
{
//    int ret = 0;
    int state = atoi(buf);

    return setSSHState(state);
}

int  MgmtserialPortServiceEnableRead(char * buf, int len)
{
    sprintf(buf ,"%d", 0);
    return 0;
}

int  MgmtserialPortServiceEnableWrite(char * buf, int len)
{
    printf("write ok!!!\n");
    return 0;
}

int  MgmtBrowserLogSwitchRead(char * buf, int len)
{
    char tBrowserLogSwitch[8] = {0};
//wankang
  //  epgBrowserAgentgetTakinSettings(TAKIN_LOG_LEVEL, tBrowserLogSwitch, 8);
    if(buf == NULL)
        return -1;
    if(tBrowserLogSwitch[0] == '0')
        strcpy(buf, "0");
    else
        strcpy(buf, "1");
    return 0;
}

int  MgmtBrowserLogSwitchWrite(char * buf, int len)
{
    int tBrowserLogSwitch = atoi(buf);

    if(tBrowserLogSwitch)
        epgBrowserAgentSetTakinSettings(TAKIN_LOG_LEVEL, "9", 2);
    else
        epgBrowserAgentSetTakinSettings(TAKIN_LOG_LEVEL, "0", 2);
    return 0;
}

int  MgmtSTBTypeRead(char * buf, int len)
{
    if(buf == NULL)
        return -1;
    strcpy(buf, "0");
    printf("Can't support this function !\n");
    return 0;
}

int  MgmtChipIdRead(char * buf, int len)
{
    if(buf == NULL)
        return -1;
    strcpy(buf, "0");
    printf("Can't support this function !\n");
    return 0;
}

/************************************************************
*PlatformCode   系统平台代码    支持读取、写入
*************************************************************/
int MgmtPlatformCodeRead(char * buf, int len)
{
    char PlatformCode_buf[URL_LEN] = {0};

    strcpy(PlatformCode_buf, SessionGetPlatformCode());
    sprintf(buf, "%s", PlatformCode_buf);
    return 0;
}

int MgmtPlatformCodeWrite(char * buf, int len)
{
    SessionSetPlatformCode(buf);
    settingManagerSave(); // 待确定
    return 0;
}

/************************************************************
*PPVlist    PPV列表 支持读取
*************************************************************/
int MgmtPPVlistRead(char * buf, int len)
{
    char PPVlist_buf[URL_LEN] = {0};

    //ppv_array_addelem(PPVlist_buf);   //未实现
    printf("Can't support this function !\n");
    sprintf(buf, "%s", PPVlist_buf);
    return 0;
}

int MgmtParaListRead(char *buf ,int length)
{
    char temp[1024] = {0};
    char *check = temp;
    char *p = NULL;
    int len = 0;
    int ret = -1;
    p = (char *)malloc(512 * 1024); // 最多512K

    char ifname[URL_LEN] = { 0 };
    network_default_ifname(ifname, URL_LEN);

    memset(temp, 0, sizeof(temp));
    sysSettingGetString("eds", temp, sizeof(temp), 0);
    len += sprintf(p + len, "Main_HomepageUrl = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    sysSettingGetString("eds1", temp, sizeof(temp), 0);
    len += sprintf(p + len, "Secondary_HomepageUrl = %s\r\n", temp);

    len += sprintf(p + len, "stbIP = %s\r\n", network_address_get(ifname, temp, 1024));
    len += sprintf(p + len, "netmask = %s\r\n", network_netmask_get(ifname, temp, 1024));
    len += sprintf(p + len, "gateway = %s\r\n", network_gateway_get(ifname, temp, 1024));

    memset(temp, 0, sizeof(temp));
    sysSettingGetString("ntp", temp, sizeof(temp), 0);
    len += sprintf(p + len, "NTPDomain = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    sysSettingGetString("ntp1", temp, sizeof(temp), 0);
    len += sprintf(p + len, "NTPDomainBackup = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    sysSettingGetString("dns", temp, sizeof(temp), 0);
    len += sprintf(p + len, "dns = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    sysSettingGetString("dns1", temp, sizeof(temp), 0);
    len += sprintf(p + len, "dns1 = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    sysSettingGetString("netuser",temp,32,0);
    len += sprintf(p + len, "netuseraccount = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    sysSettingGetString("netAESpasswd", temp, sizeof(temp), 0);
    len += sprintf(p + len, "netuserpassword = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    sysSettingGetString("netuser", temp, 32, 0);
    len += sprintf(p + len, "ntvuseraccount = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    appSettingGetString("ntvAESpasswd", temp, sizeof(temp), 0);
    len += sprintf(p + len, "ntvuserpassword = %s\r\n", temp);

	sysSettingGetInt("connecttype", &ret, 0);
	sprintf(temp, "%d", ret);
    len += sprintf(p + len, "connecttype = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    appSettingGetString("defContAcc", temp, sizeof(temp), 0);
    len += sprintf(p + len, "defContAcc = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    appSettingGetString("defAESContpwd", temp, sizeof(temp), 0);
    len += sprintf(p + len, "defContPwd = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    int lastChannelPlay = 0;
    sysSettingGetInt("lastChannelPlay", &lastChannelPlay, 0);
    sprintf(temp, "%d", lastChannelPlay);
    len += sprintf(p + len, "directplay = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    appSettingGetString("epg", temp, sizeof(temp), 0);
    len += sprintf(p + len, "epgurl = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
	int tTimezone = 0;
    sysSettingGetInt("timezone", &tTimezone, 0);

    mid_tool_timezone2str(tTimezone, temp);
    len += sprintf(p + len, "timeZone = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    mid_tool_time2string(mid_time(), temp, 0);
    len += sprintf(p + len, "localTime = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    len += sprintf(p + len, "EPGRation = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    len += sprintf(p + len, "caid = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    len += sprintf(p + len, "ecmip = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    len += sprintf(p + len, "emmport = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    len += sprintf(p + len, "pisysip = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    len += sprintf(p + len, "pisysport = %s\r\n", temp);


    memset(temp, 0, sizeof(temp));
    strcpy(temp, StbVersionBuildtime());
    len += sprintf(p + len, "CompTime = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));

    ca_vm_get_type(temp);

    len += sprintf(p + len, "CASType = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
	int flag = 0;
    sysSettingGetInt("logSend", &flag, 0);
    sprintf(temp, "%d", flag);
    len += sprintf(p + len, "QoSLogSwitch = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    len += sprintf(p + len, "playbgmusic = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    len += sprintf(p + len, "bgmusicurl = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    sysSettingGetString("upgradeUrl",temp,URL_LEN, 0);

    len += sprintf(p + len, "UpgradeServer = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));

    TR069_STATISTIC_GET_LOGSERVERURL(temp, 256);

    len += sprintf(p + len, "LogServerUrl = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
//wankang
    sprintf(temp, "%d", tr069StatisticGetLogUploadInterval());
    len += sprintf(p + len, "LogUploadInterval = %s\r\n", temp);
    memset(temp, 0, sizeof(temp));
    sprintf(temp, "%d", tr069StatisticGetLogRecordInterval());
    len += sprintf(p + len, "LogRecordInterval = %s\r\n", temp);

#ifdef TVMS_OPEN
    memset(temp, 0, sizeof(temp));
    len += sprintf(p + len, "TVMSGWIP = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    int tvmsheartbitinterval = 0;
    tvms_conf_tvmsheartbitinterval_get(&tvmsheartbitinterval);
    sprintf(temp, "%d", tvmsheartbitinterval);
    len += sprintf(p + len, "TVMSHeartbitInterval = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    int tvmsdelaylength = 0;
    tvms_conf_tvmsdelaylength_get(&tvmsdelaylength);
    sprintf(temp, "%d", tvmsdelaylength);
    len += sprintf(p + len, "TVMSDelayLength = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    tvms_conf_tvmsheartbiturl_get(temp);
    len += sprintf(p + len, "TVMSHeartbitUrl = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    tvms_conf_tvmsvodheartbiturl_get(temp);
    len += sprintf(p + len, "TVMSVODHeartbitUrl = %s\r\n", temp);
#endif

    memset(temp, 0, sizeof(temp));
	appSettingGetString("templateName", temp, sizeof(temp), 0);
    len += sprintf(p + len, "templateName = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
	appSettingGetString("areaid", temp, sizeof(temp), 0);
    len += sprintf(p + len, "areaid = %s\r\n", temp);

//wankang
    int periodicInformEnable = 0;
    int PeriodicInformInterval = 0;

    memset(temp, 0, sizeof(temp));
    MgmtTMSURLRead(temp, 255);
    len += sprintf(p + len, "ManagementDomain = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    MgmtTMSUsernameRead(temp,255);
    len += sprintf(p + len, "TMSUsername = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    MgmtTMSPasswordRead(temp,255);
    len += sprintf(p + len, "TMSPassword = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    int tr069Enable = 0;
    sysSettingGetInt("tr069_enable", &tr069Enable, 0);
    sprintf(temp, "%u", tr069Enable);
    len += sprintf(p + len, "TMSEnable = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    MgmtTMSHeartBitRead(temp, 255);
    len += sprintf(p + len, "TMSHeartBit = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    MgmtTMSHeartBitIntervalRead(temp,255);
    len += sprintf(p + len, "TMSHeartBitInterval = %s\r\n", temp);


    //memset(temp, 0, sizeof(temp));

#if(SUPPORTE_HD)
    len += sprintf(p + len, "SupportHD = %d\r\n", 1);
#else
    len += sprintf(p + len, "SupportHD = %d\r\n", 0);
#endif

    memset(temp, 0, sizeof(temp));
    len += sprintf(p + len, "DigitalAudioMode = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    strcpy(temp, SessionGetPlatformCode());
    len += sprintf(p + len, "PlatformCode = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    strcpy(temp, (const char *)StbUpgradeModel());
    len += sprintf(p + len, "STB_model = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    get_upgrade_version(temp);
    len += sprintf(p + len, "SoftwareVersion = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
  //  strcpy(temp , HwSoftwareVersion()); wankang
    len += sprintf(p + len, "HWversion = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
//    HardwareVersion(temp, 12);  wankang
    len += sprintf(p + len, "HardWareVersion = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    strcpy(temp, StbVersionBuildtime());
    len += sprintf(p + len, "CompTime = %s\r\n", temp);

    int tBrowserVersion = 0;
    char *tBrowserBuildTime = NULL;
    char *tBrowserBuilder = NULL;

    epgBrowserAgentGetTakinVersion(&tBrowserVersion, &tBrowserBuildTime, &tBrowserBuilder);
    len += sprintf(p + len, "BrowserVersion = %d\r\n", tBrowserVersion);

    memset(temp, 0, sizeof(temp));
    len += sprintf(p + len, "BrowserTime = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    sysSettingGetString("upgradeUrl",temp,URL_LEN, 0);

    len += sprintf(p + len, "UpgradeServer = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    const char* pmac = network_ifacemac_get(ifname, temp, 1024);
    if (pmac)
        sprintf(temp, "%02x:%02x:%02x:%02x:%02x:%02x", pmac[0], pmac[1], pmac[2], pmac[3], pmac[4], pmac[5]);
    len += sprintf(p + len, "MACAddress = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    mid_sys_serial(temp);
    len += sprintf(p + len, "STBSerialNumber = %s\r\n", temp);
//wankang
    memset(temp, 0, sizeof(temp));
    tr069StatisticGetLogServerUrl(temp, 256);
    len += sprintf(p + len, "LogServerUrl = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    sprintf(temp, "%d", tr069StatisticGetLogUploadInterval());
    len += sprintf(p + len, "LogUploadInterval = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    sprintf(temp, "%d", tr069StatisticGetLogRecordInterval());
    len += sprintf(p + len, "LogRecordInterval = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    //sprintf(temp, "%d", stb_syslog_get_loglevel());
    sprintf(temp, "%d", huaweiGetLogLevel());
    len += sprintf(p + len, "LogLevel = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    //sprintf(temp, "%d", stb_syslog_get_logtype());
    sprintf(temp, "%d", huaweiGetLogType());
    len += sprintf(p + len, "LogType = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    //stb_syslog_get_logftpserver(temp);
    huaweiGetLogFTPServer(temp, sizeof(temp));
    len += sprintf(p + len, "LogFtpServer = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    //stb_syslog_get_logserver(temp);
    huaweiGetLogUDPServer(temp, sizeof(temp));
    len += sprintf(p + len, "LogServer = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    //sprintf(temp, "%d", stb_syslog_get_logoutputtype());
    sprintf(temp, "%d", huaweiGetLogOutPutType());
    len += sprintf(p + len, "LogOutPutType = %s\r\n", temp);

    //私有参数
/*    memset(temp, 0, sizeof(temp));
    sprintf(temp, "%d", g_log_app_channellist);
    len += sprintf(p + len, "DebugChan = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    sprintf(temp, "%d", g_log_app);
    len += sprintf(p + len, "DebugApp = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    sprintf(temp, "%d", g_log_app_upgrade);
    len += sprintf(p + len, "DebugUpgrade = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    sprintf(temp, "%d", g_log_browser_porting);
    len += sprintf(p + len, "DebugIpanel = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    len += sprintf(p + len, "DebugLevel = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    sprintf(temp, "%d", g_log_player);
    len += sprintf(p + len, "DebugPlayer = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    sprintf(temp, "%d", g_log_tr069);
    len += sprintf(p + len, "DebugTr069 = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    sprintf(temp, "%d", g_log_ts_tool);
    len += sprintf(p + len, "DebugSys = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    sprintf(temp, "%d", g_log_tvms);
    len += sprintf(p + len, "DebugTvms = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    sprintf(temp, "%d", g_log_message);
    len += sprintf(p + len, "DebugMessage = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    sprintf(temp, "%d", g_debug_rtsp);
    len += sprintf(p + len, "DebugRtsp = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    sprintf(temp, "%d", g_debug_dhcp);
    len += sprintf(p + len, "DebugDhcp = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    if(sys_connecttype_get() == 0)
        memcpy(temp, "pppoe", strlen("pppoe"));
    else
        memcpy(temp, "eth0", strlen("eth0"));

    len += sprintf(p + len, "NetworkType = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    len += sprintf(p + len, "DebugWifi = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    sprintf(temp, "%d", g_debug_middle);
    len += sprintf(p + len, "DebugMiddle = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    sprintf(temp, "%d", g_log_mqm);
    len += sprintf(p + len, "DebugMqm = %s\r\n", temp);
*/


#if 0 //wankang
    memset(temp, 0, sizeof(temp));
    sprintf(temp, "%d", debug_all);
    len += sprintf(p + len, "DebugAll = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    sprintf(temp, "%d", open_mqm_flag);
    len += sprintf(p + len, "OpenMqm = %s\r\n", temp);
#endif
    memset(temp, 0, sizeof(temp));
	int macrovision = 0;
    appSettingGetInt("macrovision", &macrovision, 0);
    sprintf(temp, "%d", macrovision);
    len += sprintf(p + len, "MacrovisionEnableDefault = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
	int HDCPEnableDefault = 0;
	appSettingGetInt("HDCPEnableDefault", &HDCPEnableDefault, 0);
    sprintf(temp, "%d", HDCPEnableDefault);
    len += sprintf(p + len, "HDCPEnableDefault = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
	sysSettingGetString("TransportProtocol", temp, 1024, 0);
    len += sprintf(p + len, "TransportProtocol = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
	sysSettingGetString("mqmcIp", temp, sizeof(temp), 0);
    len += sprintf(p + len, "MQMC = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    /*  if(sys_videoformat_get())
      {
          strcpy(temp, "PAL");
      }
      else
          strcpy(temp, "NTSC");
    len += sprintf(p+len,"SDVideoStandard = %s\r\n",temp);

    memset(temp,0,sizeof(temp));*/
  sysSettingGetInt("videoformat",&ret,0);

    switch(ret) {
    case 0:
        strcpy(temp, "disable");
        len += sprintf(p + len, "SDVideoStandard = NTSC\r\n");
        break;
    case 1:
    case 7:
        strcpy(temp, "1080i50Hz");
        len += sprintf(p + len, "SDVideoStandard = PAL\r\n");
        break;
    case 2:
    case 8:
        strcpy(temp, "1080i60Hz");
        len += sprintf(p + len, "SDVideoStandard = NTSC\r\n");
        break;
    case 3:
        strcpy(temp, "480i60Hz");
        len += sprintf(p + len, "SDVideoStandard = NTSC\r\n");
        break;
    case 4:
        strcpy(temp, "576i50Hz");
        len += sprintf(p + len, "SDVideoStandard = PAL\r\n");
        break;
    default:
        strcpy(temp, "disable");
        break;
    }
    len += sprintf(p + len, "HDVideoStandard = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
	sysSettingGetString("Aniflicker", temp, 1024, 0);
    len += sprintf(p + len, "antiFlickerSwitch = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
    int videomode = 0;
	sysSettingGetInt("changevideomode", &videomode, 0);
    if(videomode == 0)
        strcpy(temp, "Normal");
    if(videomode == 1)
        strcpy(temp, "last picture");
    if(videomode == 2)
        strcpy(temp, "smooth switch");
    len += sprintf(p + len, "channelSwitchMode = %s\r\n", temp);

#if (defined(brcm7405) || defined(hi3716m))
    memset(temp, 0, sizeof(temp));
    int aspectRatio = 0;
	appSettingGetInt("hd_aspect_mode", &aspectRatio, 0);
    if(aspectRatio == 0)
        aspectRatio = 0;
    else
        aspectRatio = 1;
    sprintf(temp, "%d", aspectRatio);
    len += sprintf(p + len, "AspectRatio = %s\r\n", temp);
#endif

    memset(temp, 0, sizeof(temp));
    len += sprintf(p + len, "workModel = %s\r\n", temp);

    memset(temp, 0, sizeof(temp));
	int caFlag = 0;
	appSettingGetInt("nCAFlag", &caFlag, 0);
    switch(caFlag) {
    case CA_FLAG_NOT_CA_ACCOUNT:
        strncpy(temp, "0", 1);
        break;
    case CA_FLAG_SECURE_MEDIA:
        strncpy(temp, "1", 1);
        break;
    case CA_FLAG_IRDETO_HARD:
        strncpy(temp, "2", 1);
        break;
    case CA_FLAG_IRDETO_SOFT:
        strncpy(temp, "3", 1);
        break;
    case CA_FLAG_VERIMATRIX:
        strncpy(temp, "4", 1);
        break;
    default:
        break;
    }
    len += sprintf(p + len, "hw_op_CAregister = %s\r\n", temp);

    if(check == temp) {
        printf("check array temp address OK\n");
    } else {
        printf("check array temp address ERROR, temp is %#p, check is %#p\n", temp, check);
    }
	printf("lenth = %d\n",length);
    strncpy(buf,p,(len < length? len:length));
	buf[length] = '\0';
	printf("\n**********paralist********\n%s\n",buf);
    //printf("the paralist len =%d==%p==%s\n",strlen(buf->extend),buf->extend,buf->extend);

    free(p);
    p = NULL;

    return 0;
}
 int MgmtEpgUrlRead(char * buf, int len)
{
    char epgUrl[URL_LEN] = {0};
    appSettingGetString("epg", epgUrl, sizeof(epgUrl), 0);
    snprintf(buf, len, "%s", epgUrl);
    return 0;
}

 int MgmtEpgUrlWrite(char * buf, int len)
{
    appSettingSetString("epg", buf);
    return 0;
}

int MgmtInitSSHRead(char * buf ,int len)
{
    sprintf(buf, "%d", getSSHState());
    return 0;
}
int MgmtInitSSHWrite(char * buf,int len)
{
    int ret = -1;
    int state = atoi(buf);
#if defined(DEBUG_BUILD)
    ret = setSSHState(state);
#endif
    return ret;
}

//not sure for this
int MgmtStbMonitorUserRead(char * buf ,int len)
{
    sprintf(buf, "%s", "huawei");
	return 0 ;
}
int MgmtStbMonitorUserWrite(char * buf,int len)
{
   printf("MgmtStbMonitorUserWrite\n");
   return 0;
}

int MgmtStbMonitorPwdRead(char * buf,int len)
{
    sprintf(buf, "%s", ".287aW");
    return 0 ;
}
int MgmtStbMonitorPwdWrite(char * buf,int len)
{
    printf("MgmtStbMonitorPwdWrite\n");
	return 0;
}

int MgmtHwopCAregisterRead(char * buf, int len)
{
    int caFlag = 0;
	appSettingGetInt("nCAFlag", &caFlag, 0);
    switch(caFlag) {
    case CA_FLAG_NOT_CA_ACCOUNT:
        strncpy(buf, "0", 1);
        break;
    case CA_FLAG_SECURE_MEDIA:
        strncpy(buf, "1", 1);
        break;
    case CA_FLAG_IRDETO_HARD:
        strncpy(buf, "2", 1);
        break;
    case CA_FLAG_IRDETO_SOFT:
        strncpy(buf, "3", 1);
        break;
    case CA_FLAG_VERIMATRIX:
        strncpy(buf, "4", 1);
        break;
    default:
        return -1;
    }
    return 0;
}

int MgmtHwopCAregisterWrite(char * buf, int len)
{
    if(appSettingSetInt("nCAFlag", atoi(buf)) == -1) {
        printf("sysCAFlagSet error!\n");
        return -1;
    } else {
        settingManagerSave();
        return 0;
    }
}


// 0：开始；-1：停止；1：暂停；2：上传中。
static int debugInfoStatus = -1;
int getMgmtDebugInfoStatus()
{
    return debugInfoStatus;
}

void setMgmtDebugInfoStatus(int value)
{
    debugInfoStatus = value;
    return;
}

int g_startupCapFlag = 0;
int getMgmtStartupCaptured(void)
{
    appSettingGetInt("StartupCaptured", &g_startupCapFlag, 0);
    return g_startupCapFlag;
}

void setMgmtStartupCaptured(int value)
{
    appSettingSetInt("StartupCaptured", value);
    g_startupCapFlag = value;
    return;
}

char g_startupLogAddr[100] = {0};
void getMgmtStartupUploadAddr(char *value, int size)
{
    appSettingGetString("StartupUploadAddr", value, size, 0);
    snprintf(g_startupLogAddr, 100, "%s", value);
    return;
}

void setMgmtStartupUploadAddr(char *value)
{
    appSettingSetString("StartupUploadAddr", value);
    snprintf(g_startupLogAddr, 100, "%s", value);
    return;
}

void getMgmtParasListMain(char *value, int size)
{
    printf("ParasListMain in\n");
    mid_sys_getVisualizationStreamInfo(value, size, InfoTYpe_stream_information);
    // 工具和可视化的不对应，临时加的转换
    char *tok1 = strstr(value, "CPU");
    char *tok2 = strstr(value, "MEM");
    if (tok1) {
        tok1[1] = 'p';
        tok1[2] = 'u';
    }

    if (tok2) {
        tok2[1] = 'e';
        tok2[2] = 'm';
    }
    return;
}
void getMgmtParasListPip(char *value, int size)
{
    sprintf(value, "Cpu=unknown\nMem=unknown\nVideoCodec=unknown\nVideoResolution=unknown\n\
            VideoAspect=unknown\nPictureCode=unknown\nAudioCodec=unknown\nAudioBitRate=unknown\nAudioChannels=unknown\n\
            AudioSamplingRate=unknown\nPacketLost=unknown\nPacketDisorder=unknown\n\
            StreamDF=unknown\nTransportProtocol=unknown\nContinuityError=unknown\nSynchronisationError=unknown\n\
            EcmError=unKnown\nDiffAvPlayTime=unknown\nVideoBufSize=unknown\nVideoUsedSize=unknown\nAudioBufSize=unknown\n\
            AudioUsedSize=unknown\nVideoDecoderError=unknown\nVideoDecoderDrop=unknown\nVideoDecoderUnderflow=unknown\n\
            VideoDecoderPtsError=unknown\nAudioDecoderError=unknown\nAudioDecoderDrop=unknown\nAudioDecoderUnderflow=unknown\n\
            AudioDecoderPtsError=unknown\n");
    return;
}
void getMgmtStream1ParaList(char *value, int size)
{
    mid_sys_getVisualizationStreamInfo(value, size, InfoTYpe_stream_information);
    return;
}
void getMgmtStream2ParaList(char *value, int size)
{
    sprintf(value, "Cpu=unknown\nMem=unknown\nVideoCodec=unknown\nVideoResolution=unknown\n\
            VideoAspect=unknown\nPictureCode=unknown\nAudioCodec=unknown\nAudioBitRate=unknown\nAudioChannels=unknown\n\
            AudioSamplingRate=unknown\nPacketLost=unknown\nPacketDisorder=unknown\n\
            StreamDF=unknown\nTransportProtocol=unknown\nContinuityError=unknown\nSynchronisationError=unknown\n\
            EcmError=unKnown\nDiffAvPlayTime=unknown\nVideoBufSize=unknown\nVideoUsedSize=unknown\nAudioBufSize=unknown\n\
            AudioUsedSize=unknown\nVideoDecoderError=unknown\nVideoDecoderDrop=unknown\nVideoDecoderUnderflow=unknown\n\
            VideoDecoderPtsError=unknown\nAudioDecoderError=unknown\nAudioDecoderDrop=unknown\nAudioDecoderUnderflow=unknown\n\
            AudioDecoderPtsError=unknown\n");
    return;
}


