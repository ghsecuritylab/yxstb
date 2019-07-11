#ifndef __CONFIG_FILE_ALTERNATION_H__
#define __CONFIG_FILE_ALTERNATION_H__

#include "config/pathConfig.h"

#define GENERAL_ERROR -1
#define OPERATION_SUCCESS 0

namespace Hippo{

#define CONSTANT_FILE_BUF_MAX 2048
#define REGULAR_FILE_BUF_MAX 1024
#define FILE_BUF_MAX CONSTANT_FILE_BUF_MAX
#define BLOCK_LEN (128*1024)
#define NET_LEN 16

#define CONSTANTCONFIGFILEPATH	CONFIG_FILE_DIR"/yx_config_constant.ini"
#define REGULARCONFIGFILEPATH	CONFIG_FILE_DIR"/yx_config_regular.ini"
#define CONSTANTCONFIGFILEPATHBAK "/dev/mtd/2"

#define EPG_SERVER				"http://58.223.251.139:8298/auth"
#define BAK_EPG_SERVER			"http://58.223.251.139:8298/auth"
#define UPGRADE_SERVER			"http://58.223.80.17:8083/EDS/jsp/upgrade.jsp"

typedef enum {
    PPPOE = 1,
    DHCP = 2,
    staticIP = 3
} YX_NETTYPE;

typedef enum {
    ADAPTIVE = 0,
    UDPNORTP = 1,
    TCPNORTP = 2,
    UDPRTSP = 3,
    TCPRTSP = 4
} YX_RATIO;

typedef enum {
    FTCP = 0,
    FUDP = 1,
    FTCPRTP = 2,
    FUDPRTP = 3,
} YX_FRATIO;

typedef enum {
    ENGLISH = 0,
    CHINESE = 1
} YX_LANGUAGE;

typedef enum {
    DOUBLE_STACK_OFF = 0,
    DOUBLE_STACK_ON = 1
} DOUBLE_STACK;

typedef enum {
    PAL = 0,
    NTSC = 1
} YX_TVMAODE;

typedef enum {
    AUTO = 0,
    AR4TO3 = 1,
    AR16TO9 = 2
}YX_ASPECTRATIO;

typedef enum {
    BOOL_OFF = 0,
    BOOL_ON = 1
}BOOL;

typedef struct tagCONFIG {
    int nettype;      /* 0, is ethernet; 1, is wireless. */
    int connectType;
    int ratioType;
    int fratioType;
    char ip[16];
    char subMask[16];
    char gateWay[16];
    char Dns[512];
    char Dns1[512];
    char Name[65];
    char Pwd[16];
    char Ntp[512];
    char templateName[65];
    char areaid[129];

    int wireless_connecttype;
    char wireless_ip[NET_LEN];
    char wireless_netmask[NET_LEN];
    char wireless_gateway[NET_LEN];
    char wireless_dns[NET_LEN];
    char wireless_dns1[NET_LEN];

    char upgServer[512];
    int upgrgflag;
    int plch;
    char userId[65];
    char userPwd[32];
    int Lang;
    int logLevel;
    int doublestack;
    int Palorntsc;

    int Time_zone;
    char epgServer[512];
    char Epg[512];
    char Net[512];
    char devCom[16];
    char devId[16];
    char stbId[32];
    char icKey[32];
    char Usb1[16];
    char Usb2[16];
    char Mac[20];
    char Kernel[16];
    char Boot[16];
    char hardwareId[16];
    char Loader[16];
    char App[16];
    char Cpu[8];
    char Memory[8];
    char Flash[8];
    char Cur_Version[16];
    char BootFailVersion[16];
    char settingsVer[16];
    char logoVer[16];
    char lastChanel[16];
    char playurl[2048];
    int channel_id;
    int AspectRatio;
    char TVMSURL[128];
    int TVMSDelayTime;
    int TVMSHeartTime;
    int iPanellog_show;
    int debug_show;
    int csvCycle;
    char csvAddr[256];
    int mute;
    int volume;
    int isBootFail;
    char pppoeSession[20];
    int pppoe_errstatus;
    int isViModify1;
    int isViModify2;

    BOOL flagHW;
    BOOL flagZTE;
    BOOL flagUT;
    BOOL flagRTSP;
    BOOL flagMODE;

    char EPG_url_from_EDS[512];
    int tr069_enable;
    int tr069_upgrades;
    int tr069_debug;
    char lockValueArray[32];
}CONFIG;

using namespace std;

class ConfigFileAlternation{
public:
    ConfigFileAlternation();
    ~ConfigFileAlternation();

    int MoveToNew(void); // move the old item to the new one
    int BackToOld(void); // rollback the new item to the old config file

private:
    int PrepareMoveToNew(void);
    int PrepareBackToOld(void);

    void SetDefaultSetting(void);
    void PrintSettingInfo(void);
    int ConstantSettingRead(void);
    int RegularSettingRead(void);
    int ConstantSettingWrite(int mode);
    int RegularSettingWrite(void);
    int GetSettingChecksum(char *filepathname, int *checksumInFile);
    int SettingToFile(char *filename, char *dataBuf, int length);
    int SettingToFlash(char *mtdDev, char *dataBuf);
    char* CopyRightValue(char * strDest, const char* strSrc, int num);
    int ParseSettingFile(char *filename);
    int ParseBakSettingFile(const char *filename);

private:
    CONFIG para_config; // this member is named by old codes, in order to use the old codes directly
}; // class ConfigFileAlternation{
} // namespace Hippo{

#endif // __CONFIG_FILE_ALTERNATION_H__
