#ifndef __SYS_KEY_DEAL_H__
#define __SYS_KEY_DEAL_H__

#define KEYMODE_STR_EPG "epg"
#define KEYMODE_STR_VOD "vod"
#define KEYMODE_STR_IPTV "iptv"
#define KEYMODE_STR_IPTV_NUM "iptv_num"	/*数字切台*/
#define KEYMODE_STR_DVBS "dvbs"
#define KEYMODE_STR_BOOT "boot"
#define KEYMODE_STR_ERROR "error"
#define KEYMODE_STR_TIMEOUT "timeout"
#define KEYMODE_STR_STANDBY "standby"
#define KEYMODE_STR_CONFIG "config"
#define KEYMODE_STR_FLASH "flash"
#define KEYMODE_STR_UPGRADE "upgrade"
#define KEYMODE_STR_UCONFIG "uconfig"
#define KEYMODE_STR_LOCAL "local"

typedef enum tagappmode {
    APPMODE_IPTV  = 0,
    APPMODE_DVBS  = 1,
    APPMODE_STANDBY = 2,
    APPMODE_LOCAL = 3,
    APPMODE_UCONFIG = 4,
    APPMODE_NETUNLINK = 5,
    APPMODE_DIALOG = 6,
    APPMODE_DLNA_DMR = 7, /* DLNA中的DMS与IPTV,DVBS或LOCAL(高清播放器)共存, DMR是和IPTV等互斥存在的一种模式. wangjian*/
    APPMODE_ERROR
} dual_Appmode_e;

/* add:	wangjian
*  由于DLNA的特殊性，其DMS和DMC与IPTV,DVBS或LOCAL(高清播放器)共存,
*  而其DMR是和IPTV,DVBS或LOCAL互斥存在的一种"小"模式,即存在从IPTV,
*  DVBS或LOCAL这些"大"模式跳转到DMR然后返回到原模式的需求.
*/
typedef enum enum_business_mode {
    BUSIMODE_IPTV = 0,
    BUSIMODE_DVBS,
    BUSIMODE_LOCAL
} BUSINESS_MODE;


#ifdef __cplusplus
extern "C" {
#endif


    void sys_key_mode_set(const char* keymode);

    int sys_appmode_get(void);
    void sys_appmode_set(int mode);
    int sys_network_msg(void);
    void app_set_standby(int flag);

    int sys_frontPanelKey_common(int keyvalue);
    int sys_frontPanelKey_hungary(int keyvalue);



#ifdef __cplusplus
}
#endif


#endif
