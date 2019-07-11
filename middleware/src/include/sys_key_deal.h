#ifndef __SYS_KEY_DEAL_H__
#define __SYS_KEY_DEAL_H__

#define KEYMODE_STR_EPG "epg"
#define KEYMODE_STR_VOD "vod"
#define KEYMODE_STR_IPTV "iptv"
#define KEYMODE_STR_IPTV_NUM "iptv_num"	/*������̨*/
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
    APPMODE_DLNA_DMR = 7, /* DLNA�е�DMS��IPTV,DVBS��LOCAL(���岥����)����, DMR�Ǻ�IPTV�Ȼ�����ڵ�һ��ģʽ. wangjian*/
    APPMODE_ERROR
} dual_Appmode_e;

/* add:	wangjian
*  ����DLNA�������ԣ���DMS��DMC��IPTV,DVBS��LOCAL(���岥����)����,
*  ����DMR�Ǻ�IPTV,DVBS��LOCAL������ڵ�һ��"С"ģʽ,�����ڴ�IPTV,
*  DVBS��LOCAL��Щ"��"ģʽ��ת��DMRȻ�󷵻ص�ԭģʽ������.
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
