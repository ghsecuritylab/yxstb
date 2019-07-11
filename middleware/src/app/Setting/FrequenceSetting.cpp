
#include "FrequenceSetting.h"

#include "Assertions.h"
#include "ind_cfg.h"
#include "ind_mem.h"
#include "mid/mid_mutex.h"
#include "SettingApi.h"
#include "AppSetting.h"
#include "sys_basic_macro.h"

#ifdef ANDROID
#include "IPTVMiddleware.h"
#endif

#include <stdio.h>
#include <string.h>

struct CUS_FREQENCE {
    char lastaddress[2][STRING_LEN_64];
    char lastsrcip[2][STRING_LEN_64];
    char pppoeparam[STRING_LEN_64];
};

static struct CUS_FREQENCE cusFreqence;
static CfgTree_t g_cfgTree = NULL;
static mid_mutex_t g_mutex = NULL;

/* 没有初始化的动作 */
void frequenceSettingInit(void)
{
    g_mutex = mid_mutex_create();
    if(g_mutex == NULL) {
        LogSafeOperError("mid_mutex_create");
        return;
    }

    if(g_cfgTree)
        return;

    g_cfgTree = ind_cfg_create();
    if(g_cfgTree == NULL) {
        LogSafeOperError("tree_cfg_create\n");
        return;
    }

    ind_cfg_inset_object(g_cfgTree, "freqence");
    ind_cfg_inset_string(g_cfgTree, "freqence.lastaddress_I", cusFreqence.lastaddress[0], STRING_LEN_64);
    ind_cfg_inset_string(g_cfgTree, "freqence.lastaddress_II", cusFreqence.lastaddress[1], STRING_LEN_64);
#ifdef ENABLE_IGMPV3
    ind_cfg_inset_string(g_cfgTree, "freqence.lastsrcip_I", cusFreqence.lastsrcip[0], STRING_LEN_64);
    ind_cfg_inset_string(g_cfgTree, "freqence.lastsrcip_II", cusFreqence.lastsrcip[1], STRING_LEN_64);
#endif
    ind_cfg_inset_string(g_cfgTree, "freqence.pppoeparam", cusFreqence.pppoeparam, STRING_LEN_64);
    settingConfigRead(g_cfgTree, "freqence");
    return;
}

int sysMulticastGet(int pIndex, char *buf)
{
    if(buf == NULL) {
        LogSafeOperError("buf is NULL\n");
        return -1;
    }
    if(pIndex != 0 && pIndex != 1) {
        LogSafeOperError("player is not exist\n");
        return -1;
    }

    IND_STRCPY(buf, cusFreqence.lastaddress[pIndex]);
    return 0;
}

int sysMulticastSet(int pIndex, char* buf)
{
    if(buf == NULL) {
        LogSafeOperError("buf is NULL\n");
        return -1;
    }
    if(pIndex != 0 && pIndex != 1) {
        LogSafeOperError("player is not exist\n");
        return -1;
    }

    if(!strcmp(cusFreqence.lastaddress[pIndex], buf))
        return 0;
    IND_STRCPY(cusFreqence.lastaddress[pIndex], buf);
    settingConfigWrite(g_cfgTree, "freqence");
#ifdef ANDROID
#ifdef NEW_ANDROID_SETTING
    appSettingSetString("iptv_last_multicast_address", buf);
#else
    IPTVMiddleware_SettingSetStr("iptv_last_multicast_address", buf);
#endif
#endif
    return 0;
}

int sysPppoeparamGet(char *buf)
{
    if(buf == NULL) {
        LogSafeOperError("buf is NULL\n");
        return -1;
    }

    if(mid_mutex_lock(g_mutex))
        LogSafeOperWarn("\n");
    IND_STRCPY(buf, cusFreqence.pppoeparam);
    mid_mutex_unlock(g_mutex);
    return 0;
}

int sysPppoeparamSet(char* buf)
{
    if(buf == NULL) {
        LogSafeOperError("buf is NULL\n");
        return -1;
    }

    if(mid_mutex_lock(g_mutex))
        LogSafeOperWarn("\n");
    if(strcmp(cusFreqence.pppoeparam, buf))
        IND_STRCPY(cusFreqence.pppoeparam, buf);
    settingConfigWrite(g_cfgTree, "freqence");
    mid_mutex_unlock(g_mutex);
    return 0;
}

#ifdef ENABLE_IGMPV3

int sysSrcipGet(int pIndex, char *buf)
{
    if(buf == NULL) {
        LogSafeOperError("buf is NULL\n");
        return -1;
    }
    if(pIndex != 0 && pIndex != 1) {
        LogSafeOperError("player is not exist\n");
        return -1;
    }

    IND_STRCPY(buf, cusFreqence.lastsrcip[pIndex]);
    return 0;
}

int sysSrcipSet(int pIndex, char* buf)
{
    if(buf == NULL) {
        LogSafeOperError("buf is NULL\n");
        return -1;
    }
    if(pIndex != 0 && pIndex != 1) {
        LogSafeOperError("player is not exist\n");
        return -1;
    }

    if(strcmp(cusFreqence.lastsrcip[pIndex], buf)) {
        IND_STRCPY(cusFreqence.lastsrcip[pIndex], buf);
        settingConfigWrite(g_cfgTree, "freqence");
    }
    return 0;
}

#endif