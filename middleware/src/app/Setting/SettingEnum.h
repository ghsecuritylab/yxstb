#ifndef SettingEnum_h
#define SettingEnum_h

#ifdef VIETTEL_HD
typedef enum//lang=1为英语，lang=2为越南语。
{
	LANGUAGE_ENGLISH  = 1,
	LANGUAGE_LOCAL
} YX_LANGUAGE;
#else
typedef enum
{
	LANGUAGE_LOCAL	= 1,
	LANGUAGE_ENGLISH
} YX_LANGUAGE;
#endif

typedef enum{
	PCM = 0,
	PASS_THROUGH,
	OUT_CLOSE,
}SPDIF_AUDIO_FORMAT;

typedef enum {
    SOFTWARE_VERSION = 1,
    LOGO_VERSION,
    SETTING_VERSION,
    TEMPLATE_VERSION = 8,
    SOFTWARE_VERSION_BAK,
    UPGRADE_FAILURE_NUM
}UpgradeVersionType;

typedef enum{
    IP_UPGRADE_MODE = 1,
    DVB_UPGRADE_MODE,
    MIX_UPGRADE_MODE
}UPGRADE_MODE;

typedef enum {
    NET_ETH = 0,
    NET_WIRELESS
}NET_TYPE;

enum {
    SWITCH_OFF = 0,
    SWITCH_ON,
    SWITCH_UNKNOW,
};

#endif //SettingEnum_h
