/*******************************************************************************
	公司：
			Yuxing software
	纪录：
			2008-1-26 21:12:26 create by Liu Jianhua
	模块：
			tr069
	简述：
			porting layer
			主要包括
			1.一个flash保存读取
			2.一个互斥
			3.一个管道
			4.一个任务
 ******************************************************************************/

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "Tr069.h"
#include "UpgradeManager.h"

#include "app_include.h"
#include "mid/mid_task.h"
#include "mid/mid_time.h"
#include "mid/mid_msgq.h"
#include "mid/mid_mutex.h"
#include "StatisticRoot.h"

#include "sys_basic_macro.h"

#include "sys_msg.h"
#ifdef TVMS_OPEN
#include "tvms.h"
#include "tvms_setting.h"
#endif
#include "ind_mem.h"
#include "ind_tmr.h"
#include "MessageTypes.h"
#include <openssl/sha.h>
#include <openssl/aes.h>
#include "cryptoFunc.h"
#include "config/pathConfig.h"
#include "stbinfo/stbinfo.h"
#include "../../app/BrowserBridge/BrowserAgent.h"
#include "VersionSetting.h"
#include "SettingEnum.h"
#include "SettingModuleNetwork.h"

extern int strm_buf_lock(char **pBuf, int *pLen);
extern void strm_buf_unlock(void);


#define FILENAME CONFIG_FILE_DIR"/yx_config_tr069.cfg"
#define DEVNAME1 "eth0:"
#define DEVNAME2 "rausb0:"

#if defined(SHANGHAI_HD) || defined(SHANGHAI_SD)
static int g_tr069upgrade_flag = 0;
#endif

static unsigned char g_TMS_aes_keys[32 * 4];



/*************************************************
Description:鑾峰彇AES 瀵嗛挜
Return:   1:鎴愬姛;0:澶辫触
*************************************************/
int app_TMS_aes_keys_set()
{
    unsigned char temp[33] ={'\0'};
    //char mac[13] ={'\0'};
    char serialOrMac[33] = {'\0'};
    char initvector[9] ="99991231";
    char tr069_passwd[128] = {'\0'} ;
    char tRandom[128] = {'\0'} ;
    int i;
    SHA256_CTX TMS_keys;
    SHA256_Init(&TMS_keys);
    //mid_net_mac_addr(serialOrMac , 0);
    //mid_sys_serial(serialOrMac);
#if defined(ANDROID)
#ifdef NEW_ANDROID_SETTING
     sysSettingGetString("SerialNumber", serialOrMac, sizeof(serialOrMac), 0);
#else
	IPTVMiddleware_SettingGetStr("SerialNumber", serialOrMac, sizeof(serialOrMac));
#endif
#else
	tr069_port_getValue("Device.DeviceInfo.SerialNumber", serialOrMac, sizeof(serialOrMac));
#endif
    strcpy(tr069_passwd, "ac5entry");
    tr069_api_getValue("Opaque", tRandom, 128);

    LogTr069Debug("app_TMS_aes_keys_set %s--%s--%s\n", serialOrMac, tr069_passwd, tRandom);
    if((strlen(serialOrMac) == 0) ||(strlen(tr069_passwd) == 0 )||strlen(tRandom) == 0){
        printf("uckey_para is error,please checking\n");
        return 1;
    }else{
        SHA256_Update(&TMS_keys,serialOrMac,strlen(serialOrMac));
        SHA256_Update(&TMS_keys,initvector,strlen(initvector));
        SHA256_Update(&TMS_keys,tr069_passwd,strlen(tr069_passwd));
        SHA256_Update(&TMS_keys, tRandom , strlen(tRandom));

        SHA256_Final(temp, &TMS_keys);
        if(strncmp(g_TMS_aes_keys, temp, 32))
            strncpy(g_TMS_aes_keys, temp, 32);
        g_TMS_aes_keys[32] = '\0';
        return 0;
    }
}

int app_TMS_aes_keys_get(unsigned char *key)
{
    if(key == NULL){
        printf("passwd is NULL\n");
        return -1;
    }
    strncpy(key,g_TMS_aes_keys, 32);
    key[32] = '\0';
    return 0;
}



