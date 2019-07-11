#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "jse.h"
#include "app_include.h"
#include "mid/mid_time.h"
#include "Assertions.h"
#include "config/pathConfig.h"
#include "NetworkFunctions.h"

#include "sys_msg.h"
#include "sys_basic_macro.h"
#include "sys_key_deal.h"
#include "libzebra.h"

#include "Tr069.h"
#ifdef TVMS_OPEN
#include "tvms_setting.h"
#endif
#ifdef ENABLE_BAK_SETTING
#include "BakSetting.h"
#endif

#include "UtilityTools.h"
#include "Tr069Setting.h"
#include "mgmtModule.h"
#include "MessageTypes.h"
#include "config.h"
#include "io_xkey.h"


typedef int (*READ_FUNC)(const char *, char *, int);
typedef int (*WRITE_FUNC)(const char *);


static int CharToNumber(char* string)
{
    int	flag = 1;
    char *str = string;
    int i, num, len;

    if((*str) == '\0')
        return 0;
    if((*str) == '-') {
        flag = -1;
        str++;
    }
    if(((*str) == '0') && (((*(str + 1)) == 'x') || ((*(str + 1)) == 'X'))) {
        flag = 0;
        str += 2;
    }
    if((*str) == '\0')
        return 0;
    len = strlen(str);
    num = 0;
    if(flag) {
        for(i = 0; i < len; i++) {
            num = num * 10 + (*str) - '0';
            str++;
        }
        num = num * flag;
        return num;
    }
    if((len >= 8) && ((*str) > '7')) {
        for(i = 0; i < 8; i++) {
            int k;
            if((*str >= '0') && (*str <= '9'))
                k = *str - '0';
            else if((*str >= 'a') && (*str <= 'f'))
                k = *str - 'a' + 10;
            else if((*str >= 'A') && (*str <= 'F'))
                k = *str - 'A' + 10;
            else
                k = 0;
            k = 15 - k;
            num = num * 16 + k;
            str++;
        }
        num++;
        num = -num;
    }
    else {
        if(len > 8) len = 8;/* min(len,8) */
        for(i = (8 - len); i < 8; i++) {
            int k;
            if((*str >= '0') && (*str <= '9'))
                k = *str - '0';
            else if((*str >= 'a') && (*str <= 'f'))
                k = *str - 'a' + 10;
            else if((*str >= 'A') && (*str <= 'F'))
                k = *str - 'A' + 10;
            else
                k = 0;
            num = num * 16 + k;
            str++;
        }
    }
    return num;
}

/* jse_parse_param
 *
 * 从参数字符串分析得出参数璿d 整数
 * s 字符䰿e 结束字符䶿将剩下的字符串作为一个值返回，不考虑其中的分割号等特殊字�?命令匹配，返回获取参数个擿否冿返�?
 * 2007-1-16 21:39 create by 柳建�?* 2007-4-5 12:25 特别说明
 * 如果fmt里字符个数大于之后的可变参数个数，很可能到致严重错误
 */
int jse_parse_param(char *str, char *cmd, char in, char *fmt, ...)
{
    va_list args;
    int argc = 0;
    void *p = NULL;
    char *s, f;
    char buf[URL_MAX_LEN + 4];

    strcpy(buf, str);
    str = buf;

    s = strchr(str, in);
    if(s)
        *s = 0;

    if(strcmp(str, cmd) != 0) { // Command does not match
        // LogUserOperDebug("%s %s\n",str,cmd);
        return -1;
    }

    f = *fmt++;
    va_start(args, fmt);
    while(f != '\0') {
        if(s == NULL) {
            // LogUserOperDebug("WARN: s == NULL\n");
            va_end(args);
            return argc;
        }
        str = s + 1;
        s = strchr(str, in);
        if(s && f != 'e')
            *s = 0;

        p = va_arg(args, void *);
        if(p == NULL) {
            // LogUserOperDebug("WARN: p == NULL\n");
            va_end(args);
            return argc;
        }
        argc ++;
        switch(f) {
        case 'd':
            *((int *)p) = CharToNumber(str);
            break;
        case 's':
            strcpy((char *)p, str);
            break;
        case 'e':
            strcpy((char *)p, str);
            va_end(args);
            return argc;
        case 'c':
            *((char *)p) = *str;
            break;
        default:
            // LogUserOperDebug("WARN: %c / %s\n", f, fmt);
            break;
        }
        f = *fmt++;
    }

    va_end(args);
    return argc;
}

static int JseWrite_BrowserControl(const char *func, const char *param, char *value, int len)
{
    char s1[URL_MAX_LEN + 4] = {0};

    if(jse_parse_param(value, "open", ':', "e", s1) == 1) { // "open:url"
        if(strcmp(s1, "menu") == 0)
            NativeHandlerInputMessage(MessageType_System, EIS_IRKEY_URL_MENU, 0, NULL);
        else if(strcmp(s1, "config") == 0)
            NativeHandlerInputMessage(MessageType_System, EIS_IRKEY_PAGE_CONFIG, 0, NULL);
        else if(strcmp(s1, "standby") == 0)
            NativeHandlerInputMessage(MessageType_System, EIS_IRKEY_PAGE_STANDBY, 0, NULL);
        else if(strcmp(s1, "error") == 0)
            NativeHandlerInputMessage(MessageType_System, EIS_IRKEY_PAGE_ERROR, 0, NULL);
        else if(strcmp(s1, "timeout") == 0)
            NativeHandlerInputMessage(MessageType_System, EIS_IRKEY_PAGE_TIMEOUT, 0, NULL);
        else if(strcmp(s1, "boot") == 0)
            NativeHandlerInputMessage(MessageType_System, EIS_IRKEY_PAGE_BOOT, 0, NULL);
        else if(strcmp(s1, "dvbs") == 0)
            NativeHandlerInputMessage(MessageType_System, EIS_IRKEY_PAGE_DVBS, 0, NULL);
    } else if(jse_parse_param(value, "funkey", ':', "s", s1) == 1) { /* "funkey:flag" */
        sys_key_mode_set(s1);
#ifdef INCLUDE_PVR
    } else if(!strcmp(value, "pvr_list_read")) {
        LogUserOperDebug("call pvr_list_read, currently not supported\n");
#endif
    } else {
        LogUserOperError("Can't find this key(%s)\n", value);
        return -1;
    }
    return 0;
}

static int JseWrite_SysControl(const char *func, const char *param, char *value, int len)
{
    int i1 = 0;
    char s1[URL_MAX_LEN + 4] = {0};

    LogUserOperDebug("Input string [%s]\n", value);
    if(!strncmp(value, "sys_", 4)) {
        jse_sys_sys_control(value);
    } else if(jse_parse_param(value, "config", ':', "") == 0) { // config
       ; //ԭ���Ĵ���Ϊע����ʱ��ʱ���������µ�JS�ӿڳ�����ڴ���һ����ע�ᣬ����Ͳ���ע��
    } else if(jse_parse_param(value, "stanbyexit", ':', "") == 0) { // exit stanbyexit
        sendMessageToKeyDispatcher(MessageType_Unknow, 0x9380 + 95, 0, 0);
        LogUserOperDebug("stanby out\n");
    } else if(jse_parse_param(value, "standby", ':', "d", &i1) == 1) { //c10 standby
        unsigned int hardwareVer = 0;
        yhw_board_getHWVersion(&hardwareVer);

        int flag = 0;
        extern int Hybroad_Inconsistent_getIRtype();
        if (Hybroad_Inconsistent_getIRtype() == 1) {
            if (hardwareVer >= 0x300)
                flag = 1;
        }

        if(flag)
        {
            LogUserOperDebug("I will realStandby!  i1=[%d]\n", i1);
            if(i1) {
                mid_fpanel_poweroffdeep();
            }
        } else {
            LogUserOperDebug("I will standby!\n");
            mid_fpanel_standby_set(i1);
        }
#if defined(HUAWEI_C20)
    } else if(jse_parse_param(value, "netConnect", ':', "") == 0) {
        sendMessageToNativeHandler(MessageType_System, NET_CONNECT, 0, 0);
#else
    } else if (jse_parse_param(value, "staticIP", ':', "") == 0
        || jse_parse_param(value, "dhcp", ':', "") == 0
        || jse_parse_param(value, "dhcp_encrypt", ':', "") == 0
        || jse_parse_param(value, "dhcp_dblvlan", ':', "") == 0
        || jse_parse_param(value, "pppoe", ':', "") == 0) {
        char ifname[URL_LEN] = { 0 };
        network_connect(network_default_ifname(ifname, URL_LEN));
#endif // HUAWEI_C20
    } else if(jse_parse_param(value, "pppoeexit", ':', "") == 0) { //pppoeexit
        char ifname[URL_LEN] = { 0 };
        network_default_ifname(ifname, URL_LEN);
        network_disconnect(ifname);
    } else if(jse_parse_param(value, "ntpsync", ':', "") == 0) { //"ntpsync"
#if defined(SHANGHAI_HD) || defined(SHANGHAI_SD)
#else
        mid_ntp_time_sync();
#endif
    } else if(jse_parse_param(value, "cfg_reset", ':', "") == 0) { /* cfg_reset */
#ifdef Huawei_v5
        int ret = -1;
        yos_systemcall_runSystemCMD(" find /root/* -maxdepth 1 | grep -v cpvr.s3db | xargs rm -rf",  &ret); // huawei_v5 delete all root files Temporary; we will be modify later!
        LogUserOperDebug("ret : %d\n", ret);
#else
        removeFile("/root/passwd");
        removeFile("/root/monitorpasswd");
#ifdef TVMS_OPEN
        tvms_config_load(1);
#endif
        int upgradeOK = 0;
        sysSettingGetInt("IPUpgradeOK", &upgradeOK, 0);
        settingManagerLoad(1); /*customer system config load*/
        if(!upgradeOK){
           	sysSettingSetInt("isFirstUpgradeOK", -1);
            sysSettingSetInt("IPUpgradeOK", 0);
        }
        settingManagerSave();
#if defined(ENABLE_BAK_SETTING) && ((defined(VIETTEL_HD) && defined(brcm7405)) || defined(Sichuan))
		bakSetting().remove();
#endif

        TR069_STATISTIC_CONFIG_RESET();
        TR069_STATISTIC_CONFIG_SAVE();

        TR069_API_SETVALUE("Config.Reset", "", 1);
#if defined(Sichuan) || defined(Jiangsu) // 四川 江苏有零配置, 默认不清空网管地址
#else
        TR069_API_SETVALUE("Device.ManagementServer.URL", "", 0);
        TR069_API_SETVALUE("Device.ManagementServer.URLBackup", "", 0);
#endif
        TR069_API_SETVALUE("Config.Save", "", 1);

#if defined(BLUETOOTH)
        int ret = -1;
        yos_systemcall_runSystemCMD("rm "DEFAULT_MODULE_BT_DATAPATH"/hcid.modify.conf -rf",  &ret);
        LogUserOperDebug("ret : %d\n", ret);
#endif // defined(BLUETOOTH)

#endif // Huawei_v5
    } else if(jse_parse_param(value, "logoshow", ':', "d", &i1) == 1) { //logoshow
        LogUserOperDebug("Logo show %d\n", i1);
        BootImagesShowBootLogo(i1);
    } else if(jse_parse_param(value, "reboot", ':', "") == 0) { //reboot
        yhw_board_setRunlevel(1);
        mid_fpanel_reboot();
    } else if(jse_parse_param(value, "delay", ':', "d", &i1) == 1) //delay
        mid_task_delay(i1);
    else if(jse_parse_param(value, "serial", ':', "s", s1) == 1) //serial set
        mid_sys_serial_set(s1);
    else if(jse_parse_param(value, "mseclog", ':', "e", s1) == 1) //mseclog
        LogUserOperDebug(" [%s] msec [%lld] !\n", s1, mid_clock());
    else if(jse_parse_param(value, "itms", ':', "") == 0) {
        LogUserOperDebug("[DBG ITMS]\n");
#ifdef INCLUDE_TR069
				itms_start();
#elif defined(INCLUDE_HMWMGMT)
				mgmt_init();
#endif


    }else {
        LogUserOperError("I dont know the cmd [%s]\n", value);
        return -1;
    }
    return 0;
}

static int JseWrite_SysRead(const char *func, const char *param, char *value, int len)
{
    int i1 = 0;
    char s1[URL_MAX_LEN + 4] = {0};

    if(!strncmp(param, "sys_", 4)) {
        jse_sys_sys_read(param, value, len);
    } else if(jse_parse_param(param, "slice", ':', "de", &i1, s1) == 2) {
        mid_tool_string_cutcpy(value, s1, i1);
    } else if(!strcmp(param, "curtime")) {
        sprintf(value, "%d" , mid_time());
    } else if(!strcmp(param, "curdate")) {
        mid_tool_time2string(mid_time(), value, 0);
    } else if(!strcmp(param, "pppoeerr")) {
        sprintf(value, "%d" , 1);
    } else if(!strcmp(param, "ntpstatus")) {
        int status;
        /*ն¼ԁ̒»זntp״̬:δ½�¼卬²½£¬µ«Ϊ¶Ս⾓¿ڲ»±䣬
          θҳ²»Ѩضў¸ģ¬ɔºΔ­4һҹֻʏ±¨}ז״̬:ʧ°ܻ�? ³ɹ¦¡£wangjian 20100419   */
        if((status = mid_ntp_status()) < 0)
            status = 0;
        sprintf(value, "%d" , status);
    } else if(!strcmp(param, "strtime")) {
        int sec = 0;

        if(NULL != value) {
            sec = mid_time();
            if(0 != MID_UTC_SUPPORT) {
                sec += mid_get_times_sec();
            }
            mid_tool_time2string(sec, value, ':');
        }
    } else if(!strcmp(param, "linkstate")) {
        char devname[USER_LEN] = { 0 };
        network_default_devname(devname, USER_LEN);
        sprintf(value, "%d", network_device_link_state(devname));
    }
    else {
        LogUserOperError("Can't find this key(%s)\n", param);
        return -1;
    }
    return 0;
}


int jse_init()
{
    a_Hippo_API_JseRegister("JseBrowserControl", NULL, JseWrite_BrowserControl, IoctlContextType_eHWBase);
    a_Hippo_API_JseRegister("JseSysControl", NULL, JseWrite_SysControl, IoctlContextType_eHWBase);
    a_Hippo_API_JseRegister("JseSysRead", JseWrite_SysRead, NULL, IoctlContextType_eHWBase);

    return 0;
}

