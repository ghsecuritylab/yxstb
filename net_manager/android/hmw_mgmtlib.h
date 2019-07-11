/******************************************************************************

  Copyright (C), 2001-2011, Huawei Tech. Co., Ltd.

 ******************************************************************************
  File Name     : hmw_mgmtlib.h
  Version       : Initial Draft
  Author        : z00109453
  Created       : 2012/5/25
  Last Modified :
  Description   : 对外提供的公共库头文件
  Function List :
  History       :
  1.Date        : 2012/5/25
    Author      : z00109453
    Modification: Created file

******************************************************************************/
#ifndef _HMW_MGMTLIB_H
#define _HMW_MGMTLIB_H


#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
如下节点定义为本模块自定义数据名称
************************************************************************/
#define MGMT_BOOTSTRAP                  "Mgmt.BootStrap"            //bootStrap为0 or 1;     BOOTSTRAP事件
#define MGMT_ACSREBOOT                  "Mgmt.AcsReboot"            //是否acs下发的重启
#define MGMT_ACSREBOOT_COMMANDKEY      "Mgmt.AcsReboot.CommandKey" //重启前的commandKey
#define MGMT_TMSENABLE                  "Mgmt.TMSEnable"            //tms是否可用
//下面2个升级主要用于android在中间件未起来时使用；
#define MGMT_ACSUPGRADE_REBOOT         "Mgmt.AcsUpgrade.Reboot"    //是否acs下发的重启
#define MGMT_ACSUPGRADE_COMMANDKEY    "Mgmt.AcsUpgrade.CommandKey" //升级前的commandKey

/*****************************************************************************
Module Name:基础模块
Module Description:系统基础模块
*****************************************************************************/
typedef enum tagHMW_MgmtConfigSRC_E
{
    MGMT_CONFIG_TMS = 0,    //TMS 参数获取
    MGMT_CONFIG_MT,         //stbmonitor 工具参数获取
    MGMT_CONFIG_OTHERS,     //其他参数，目前没有这方面的参数，预留		
}HMW_MgmtConfig_SRC;


/*定义统一管理模块通知业务模块的消息类型
    这些消息类型除了CPE的播控、上传、下载是异步的，其他的都为同步消息；
    每个消息都调用int (*fnMgmtNotifyCallback)(HMW_MgmtMsgType eMsgType, unsigned int argc, void *argv[]);
    此回调进行发送，argc 表示后面argv的个数；
    
    当CPEAgent   往外发送信息并携带信息给中间件时:
    a, int类型的都是(void *)itemp,强转, 收到为int类型时，直接(int)itemp就可以获取
    反之,需要中间件提供int类型参数时，则传递(void  *)&itemp 让中间件填写,*(int)itemp = 1
    就可以将值返回；

    b, char类型或者结构体的，cpeAgent模块都是申请空间，发送指针格式；    
*/
typedef enum tagHMW_MgmtMsgType_E
{
    MGMT_MESSAGE_NONE           = 0x000,    //NONE MSG

/*以下皆为ACS端下发的命令*/
    MGMT_CPE_REGISTER_ACS_OK    = 0x101,    //向ACS注册成功:argc = 0;
    
    MGMT_CPE_ACS_REQ_REBOOT     = 0x102,    //ACS通知STB重启:argc = 0;
    
    MGMT_CPE_CONFIG_DOWNLOAD    = 0x103,    //ACS下发升级地址或者下载配置文件用于恢复的config.dat
                                            //异步: argc = 1; argv[0] = (Mgmt_DownloadInfo *指向的一段内存空间)
        
    MGMT_CPE_CONFIG_FACTORYRESET    = 0x104,    //ACS下发出厂设置 :argc = 0; 不需要返回值，或者异步反馈重启
    
    MGMT_CPE_CONFIG_UPLOAD          = 0x105,    //ACS下发中间件上传日志或者config.dat
                                                //异步: argc=1; argv[0]=(Mgmt_UpLoadInfo*指向的一段内存空间)
    
    MGMT_CPE_PLAY_DIAGNOSTICS       = 0x106,    //ACS下发的播控诊断异步:argc=3; 
                                                //argv[0]:DiagnosticsState[64], argv[1]:PlayURL[1024]; argv[2]:PlayState

    MGMT_CPE_CALL_VALUECHANG        = 0x107,    //CPEAgent向中间件进行值更改;目前该接口预留，无实际处理
    
    MGMT_LOG_OUTPUT_CHANNEL_SET     = 0x108,    //CPE需要的log输出方式，具体看enum:hmwMgmtLogChn
                                                //argc = 2; 
                                                
    MGMT_CPE_GET_PLAYSTATE          = 0x109,   //获取当前的播放状态url \ state；                                  

/*以下皆为stbMonitor_pc端下发的命令*/
    MGMT_MT_PLAYER_BY_CHANNO    = 0x201,    //下发播放某个频道号
                                            //argc = 2; argv[0]:下发的频道号;argv[1] 中间件反馈的结果
                                            //argv[1]为int型；-1表示播放失败，1表示播放成功；即*argv[1] = 1;

    MGMT_MT_PLAYER_BY_URL       = 0x202,    //下发播放某个URL
                                            //argc = 2; argv[0]:下发的URL;argv[1] 中间件反馈的结果
                                            //argv[1]为int型；-1表示播放失败，1表示播放成功；即*argv[1] = 1;

    MGMT_MT_PLAYER_STOP         = 0x203,    //下发停止管理工具下发的播控操作;argc=1;
                                            //argv[0]由中间件填写播控结果,1表示成功；其他为失败
    
    
    MGMT_MT_PLAYER_MPCTRL       = 0x204,    //下发快进,后退,暂停/开始的控制，
                                            //分别为:"KEY_PAUSE_PLAY""KEY_FAST_FORWARD""KEY_FAST_BACK"，
                                            //argc =0;

    MGMT_MT_TOOL_REBOOT         = 0x205,    //通知盒子重启;argc =0;    
    
    MGMT_MT_ENTER_DEBUG         = 0x206,    //下发进入debug ;argc =0;
    
    MGMT_MT_EXIT_DEBUG          = 0x207,    //下发退出debug ;argc =0;
    
    MGMT_MT_GET_CHANNELNUM_TOTAL    = 0x208,    //获取频道总个数
                                                //argc = 1; argv[0] 由中间件填写频道总个数，用于后面获取频道信息
    
    MGMT_MT_GET_CHANNELINFO_I       = 0x209,    //根据上面获取的频道总数，依次读取频道i的信息
                                                //argc = 2; argv[0]:表示频道号, argv[1]由中间件填写频道信息
    
    MGMT_MT_GET_COLLECT_FILEPATH    = 0x210,    //获取一键定位文件的完整文件路径；argc = 2; 
                                                //argv[0]由中间件填写文件是否存在int:1,存在,2 收集中;其他表示收集失败
                                                //argv[1],当文件存在时填写文件路径；

/*以下是管理工具进行升级的消息*/
    MGMT_MT_UPGRADE_GET_WORKSTAT        = 0x301,    //获取能否升级argc =1,  argv[0] ,int 型:1 表示可以升级,其他异常
    
    MGMT_MT_UPGRADE_SET_LENGTH          = 0x302,    //传送pc端升级文件(*.bin)的长度argc = 1; argv[0]文件长度信息
    
    MGMT_MT_UPGRADE_SET_FORCE           = 0x303,    //是否强制升级的标识;argc = 1; argv[0] = 1,表示要强制升级
    
    MGMT_MT_UPGRADE_SET_UPHEADER        = 0x304,    //发送文件头信息，获取能否升级
                                                    //argc = 2 ; argv[0]:升级文件头传送给stb, 
                                                    //argv[1]由中间件填写非强制升级下是否进行升级(1:是要升级)；
    
    MGMT_MT_UPGRADE_GET_DOWNHANDLE      = 0x305,    //获取升级文件保存位置的句柄
                                                    //argc = 1;  argv[0]:由中间件填写(FILE * fptr)这个句柄给机顶盒；
                                                    //即memcpy(argv[0], &fptr, sizeof(FILE *));
    
    MGMT_MT_UPGRADE_SET_CLOSEWORK       = 0x306,    //开始下载，通知关闭播放、按键状态，即让盒子进入升级下载页面
                                                    //argc = 0;

    MGMT_MT_UPGRADE_SET_DOWNLOAD_PER    = 0x307,    //下载过程中隔一段时间，循环将下载进度发送给盒子
                                                    //argc = 1; argv[0] ,int 型,为具体值;
    
    MGMT_MT_UPGRADE_GET_BURN_PROCESS    = 0x308,    //中间件进行烧录升级过程时，stbmonitor定时获取烧写进度
                                                    //argc = 1; argv[0],由中间件反馈
    
    MGMT_MT_UPGRADE_SET_BURN_START      = 0x309,    //升级文件下载完成后，告知中间件此事件发生在0x308之前
                                                    //argc =0;

    MGMT_NT_UPGRADE_NETWORK_DISCONNECT  = 0x310,    //CPEAgent下载过程中网络断开连接，通知中间件
                                                    //argc =0;

    MGMT_STB_NETWORK_DISCONNECT         = 0x400,    //由外部发给cpe的事件；
    MGMT_STB_AWAIT_DISCONNECT           = 0x401,    //stb断开与pc端的连接，但不停止服务端；
    MGMT_MESSAGE_END,
}HMW_MgmtMsgType;



//TMS/Stbmonitor下发的日志级别/类型，根据各自日志的实现进行转化
typedef enum 
{
    HMW_MGMT_LOG_DISABLE = 0,
    HMW_MGMT_LOG_ENABLE  = 2
}HMW_MGMT_TMSLOG_SWITCH;

typedef enum 
{
    HMW_MGMT_LOG_LEVEL_ALL  = 0,
    HMW_MGMT_LOG_LEVEL_ERROR = 3,
    HMW_MGMT_LOG_LEVEL_INFO  = 6,
    HMW_MGMT_LOG_LEVEL_DEBUG = 7
}hmwMgmtLogLevel;

typedef enum 
{
    HMW_MGMT_LOG_TYPE_ALL       = 0,
    HMW_MGMT_LOG_TYPE_OPERATE   = 16,
    HMW_MGMT_LOG_TYPE_RUN       = 17,
    HMW_MGMT_LOG_TYPE_SECURTIY  = 19,
    HMW_MGMT_LOG_TYPE_USER      = 20
}hmwMgmtLogType;

// 日志输出类型(ftp /file/ 等输出通道)
typedef enum
{
    HMW_MGMT_ALL_LOG_CLOSE    = 0,  //日志关闭
    HMW_MGMT_ONLY_FTPLOG_OPEN = 1,      //ftp日志上传
    HMW_MGMT_ONLY_SYSLOG_OPEN = 2,      //实时日志上传
    HMW_MGMT_ALL_LOG_OPEN     = 3,          //ftp/sys日志功能都开，这是cpe/stb需求的日志输出方式；
    HMW_MGMT_LOG_OUTPUT_CNT
}hmwMgmtLogChn;

/*****************************************************************************
     cpe升级用到的结构体
*****************************************************************************/
//    传输完成中命令类型
typedef enum
{
    MGMT_CMD_DOWNLOAD =  1,  /* 下载 */
    MGMT_CMD_UPLOAD   =  2,  /* 上传 */
    MGMT_CMD_DOWNLOAD_CONFIG = 3
}MgmtCmdType;


/*定义download结构体*/
typedef struct _MGMT_DownloadInfo_
{
    char       szCommandKey[64];
    char       szFileType[64];       /*下载文件类型1 Firmware Upgrade  Image,3 Vendor Configuration File*/
    unsigned int        uiFileSize;        /*文件大小*/
    char       szURL[256];           /*更新文件的URL*/
    char       szPassword[256];      /*登录更新服务器密码*/
    char       szUserName[256];      /*登录更新服务器用户*/
    char       szTargetFileName[256];/*下载的文件名*/
    int        uiDelaySeconds;        /*机顶盒延辞下载时间*/
    char       szSuccessURL[256];    /*下载成功后返回的URL*/
    char       szFailureURL[256];    /*下载失败后返回的URL*/
}Mgmt_DownloadInfo;

/*定义UpLoadInfo结构体*/
typedef struct _MGMT_UPLAOD_INFO_
{
    char       szCommandKey[64];
    char       szFileType[64];       /*上传文件类型1 Firmware Upgrade  Image,3 Vendor Configuration File*/
    char       szURL[256];           /*更新文件的URL*/
    char       szPassword[256];      /*登录更新服务器密码*/
    char       szUserName[256];      /*登录更新服务器用户*/
    int        uiDelaySeconds;        /*机顶盒延迟上传时间*/
}Mgmt_UpLoadInfo;
/*****************************************************************************
 Prototype    : fnMgmtIoCallback
 Description  : 参数读写接口回调函数
                  包括cpe tr069\stbmonitor\config.dat里面会用到的多种参数
 Input Param  : const char * szParm
                char * pBuf
                int iLen
                ;
 Output       : None
 Return Value : None
 Calls        : None
 Called By    : None
 
  History        :
  1.Date         : 2012/5/25
    Author       : z00109453
    Modification : Created function

*****************************************************************************/
typedef int (*fnMgmtIoCallback)(HMW_MgmtConfig_SRC eSrcType, const char * szParm, char * pBuf, int iLen);


/*****************************************************************************
 Prototype    : fnMgmtNotifyCallback
 Description  : Mgmt模块通知应用模块的回调函数
 Input Param  : HMW_MgmtMsgType eMsgType
                unsigned int argc
                void *argv[]
                ;
 Output       : None
 Return Value : None
 Calls        : None
 Called By    : None
 
  History        :
  1.Date         : 2012/7/2
    Author       : z00109453
    Modification : Created function

*****************************************************************************/
typedef int (*fnMgmtNotifyCallback)(HMW_MgmtMsgType eMsgType, unsigned int argc, void *argv[]);




/*****************************************************************************
 Prototype    : hmw_mgmtRegReadCallback
 Description  : 注册读参数回调接口
 Input Param  : fnMgmtIoCallback pstCallback
                ;
 Output       : None
 Return Value : None
 Calls        : None
 Called By    : None
 
  History        :
  1.Date         : 2012/5/25
    Author       : z00109453
    Modification : Created function

*****************************************************************************/
int hmw_mgmtRegReadCallback(fnMgmtIoCallback pfnCallback);


/*****************************************************************************
 Prototype    : hmw_mgmtRegWriteCallback
 Description  : 注册写参数回调接口
 Input Param  : fnMgmtIoCallback pstCallback
                ;
 Output       : None
 Return Value : None
 Calls        : None
 Called By    : None
 
  History        :
  1.Date         : 2012/5/25
    Author       : z00109453
    Modification : Created function

*****************************************************************************/
int hmw_mgmtRegWriteCallback(fnMgmtIoCallback pfnCallback);











/*****************************************************************************
 Prototype    : hmw_mgmtNotify
 Description  : 向业务模块通知消息
 Input Param  : HMW_MgmtNotifyMsg stMsg
                ;
 Output       : None
 Return Value : None
 Calls        : None
 Called By    : None
 
  History        :
  1.Date         : 2012/5/28
    Author       : z00109453
    Modification : Created function

*****************************************************************************/
int hmw_mgmtRegNotifyCallback(fnMgmtNotifyCallback pfnCallback);




/*****************************************************************************
 Prototype    : hmw_mgmtRead
 Description  : 从Mgmt模块读取参数，预留接口，目前不支持
 Input Param  : const char * szParm
                char * pBuf
                int iLen
                ;
 Output       : None
 Return Value : None
 Calls        : None
 Called By    : None
 
  History        :
  1.Date         : 2012/5/28
    Author       : z00109453
    Modification : Created function

*****************************************************************************/
int hmw_mgmtRead(const char * szParm, char * pBuf, int iLen);



/*****************************************************************************
 Prototype    : hmw_mgmtWrite
 Description  : 向Mgmt模块写入参数，预留接口，目前不支持
 Input Param  : const char * szParm
                char * pBuf
                int iLen
                ;
 Output       : None
 Return Value : None
 Calls        : None
 Called By    : None
 
  History        :
  1.Date         : 2012/5/28
    Author       : z00109453
    Modification : Created function

*****************************************************************************/
int hmw_mgmtWrite(const char * szParm, char * pBuf, int iLen);



/*****************************************************************************
 Prototype    : hmw_mgmtSendMsgToCPEAgent
 Description  : 向Mgmt模块通知消息
 Input Param  : 
 
 Output       : None
 Return Value : None
 Calls        : None
 Called By    : None
 
  History        :
  1.Date         : 2012/1/20
    Author       : skf74590
    Modification : Created function
	根据HMW_MgmtMsgType里面的不同消息传递不同的argc, *argv[]
*****************************************************************************/
int hmw_mgmtSendMsgToCPEAgent(HMW_MgmtMsgType eMsgType, int argc, void *argv[]);




/*****************************************************************************
 Prototype    : hmw_mgmtValueChange
 Description  : 如有Mgmt模块参数(包括tr069 和stbmonitor管理工具的参数)更改向其写入
 Input Param  : const char * szParm
                char * pBuf
                int iLen
                ;
 Output       : None
 Return Value : None
 Calls        : None
 Called By    : None
 
  History        :
  1.Date         : 2012/5/28
    Author       : z00109453
    Modification : Created function

*****************************************************************************/
int hmw_mgmtValueChange(const char * szParm, char * pBuf, int iLen);


/*****************************************************************************
 Prototype    : fnMgmtNotifyCallback
 Description  : Mgmt模块日志输出的回调函数
 Input Param  : char
 Output       : None
 Return Value : None
 Calls        : None
 Called By    : None
 
  History        :
  1.Date         : 2012/7/2
    Author       : z00109453
    Modification : Created function

*****************************************************************************/
typedef void (*fnMgmtLogExportCallback)(const char* pszFile, int lLine, const char* pszFunc, int lThreadId, int enLogType, int enLogLevel, const char* pszModule, const char* format, ...);


/*****************************************************************************
 Prototype    : hmw_mgmtRegLogCallback
 Description  :  注册写日志回调接口
 Output       : None
 Return Value : None
 Calls        : None
 Called By    : None
 
  History        :
  1.Date         : 2013/1/11
    Author       : zkf74589
    Modification : Created function

*****************************************************************************/
int hmw_mgmtRegLogCallback(fnMgmtLogExportCallback  fnCallback);


/*****************************************************************************
Module Name:Alarm模块
Module Description:实现系统统一错误码功能模块
*****************************************************************************/

typedef enum 
{
    ALARM_LEVEL_CRITICAL       = 1, /* 紧急 */
    ALARM_LEVEL_MAJOR          = 2, /* 重要 */
    ALARM_LEVEL_MINOR          = 3, /* 次要 */
    ALARM_LEVEL_WARNING        = 4, /* 提示 */
    ALARM_LEVEL_INDETERMINATE  = 5, /* 不确定 */
    ALARM_LEVEL_CLEARED        = 6  /* 已清除 */
}HMW_CPEAlarmLevel_E;

/******************************************************
   这里用定义来设置告警ID，
   外部告警的ID的定义需要加进来
*******************************************************/
#define    MGMT_ALARM_DISK_OPEREATE         "100102"        //  磁盘读写失败
#define    MGMT_ALARM_CPU_USAGE             "100103"        // CPU使用率超过阀值
#define    MGMT_ALARM_DISK_USAGE            "100104"        //磁盘使用率超过阀值
#define    MGMT_ALARM_MEM_USAGE             "100105"        // 内存使用百分比超过阀值
#define    MGMT_ALARM_PACKET_LOST           "100106"        //丢包率超过阈值告警 
#define    MGMT_ALARM_DECODE_FAIL           "100108"        //机顶盒解码错误告警
#define    MGMT_ALARM_UPGRADE_FAIL          "300101"        //升级失败
#define    MGMT_ALARM_AUTH_FAIL             "300103"        //IPTV业务认证失败告警.
#define    MGMT_ALARM_JOIN_CHANNEL          "300104"        //加入频道失败告警


int  hmw_mgmtAlarmInit();
/*****************************************************************************
 Prototype    : hmw_mgmtAlarmReport
 Description  : 告警上报函数
 Input Param  : 告警ID，如上定义MGMT_ALARM_JOIN_CHANNEL "300104"根据机器定义错误来
 		          const char* pAlarmID,
 		         告警类型1表示告警上报，0表示告警恢复
 		         int iAlarmType,
 		         告警级别HMW_CPEAlarmLevel_E，有1-6多个级别
 		         int iLevel
 		         告警附加消息,由调用者来 填写
 		         const char *pszInfo
 Output       : None
 Return Value : None
 Calls        : None
 Called By    : None
 
  History        :
  1.Date         : 2012/8/15
    Author       : zkf74589
    Modification : Created function

*****************************************************************************/
int hmw_mgmtAlarmReport(const char* pAlarmID, int iAlarmType, int iLevel, const char *pszInfo);



/*****************************************************************************
Module Name:CLI命令行模块
Module Description:实现系统命令行功能模块
*****************************************************************************/
//定义同时登陆客户端个数
#define MGMT_CLI_MAX_CONN 1

//定义注册命令行时必须携带的参数个数
#define MGMT_CLI_PARM_NUM_REG 5

//定义去注册命令行时必须携带的参数个数
#define MGMT_CLI_PARM_NUM_UNREG 1


/*****************************************************************************
 Prototype    : hmwMgmtCliCallback
 Description  : 命令行回调函数
 Output       : None
 Return Value : None
 Calls        : None
 Called By    : None
 
  History        :
  1.Date         : 2012/5/28
    Author       : z00109453
    Modification : Created function

*****************************************************************************/
typedef int (*hmwMgmtCliCallback)(struct cli_def *pstCliMng,  char *command, char *argv[], int argc);


/*****************************************************************************
 Prototype    : hmw_mgmtCliInit
 Description  : 命令行模块初始化函数
 Output       : None
 Return Value : None
 Calls        : None
 Called By    : None
 
  History        :
  1.Date         : 2012/5/28
    Author       : z00109453
    Modification : Created function

*****************************************************************************/
int hmw_mgmtCliInit();


/*****************************************************************************
 Prototype    : hmw_mgmtCliDeInit
 Description  : 命令行模块去初始化函数
 Output       : None
 Return Value : None
 Calls        : None
 Called By    : None
 
  History        :
  1.Date         : 2012/5/28
    Author       : z00109453
    Modification : Created function

*****************************************************************************/
int hmw_mgmtCliDeInit();



/*****************************************************************************
 Prototype    : hmw_mgmtCliRegCmd
 Description  : 命令行模块注册命令函数
 Input          :unsigned int argc, void *argv[]
 Output       : None
 Return Value : None
 Calls        : None
 Called By    : None
 
  History        :
  1.Date         : 2012/5/28
    Author       : z00109453
    Modification : Created function
    注意argc必须写成固定的为MGMT_CLI_PARM_NUM_REG(5个)
    argv[0]:struct cli_command *类型，表示该命令关键字的父关键字，可以取值为NULL，表示该关键字为第一个关键字
    argv[1]:char *，表示要注册的命令关键字
    argv[2]:hmwMgmtCliCallback，表示该命令的回调处理函数
    argv[3]:char *，表示要注册的命令帮助信息
    argv[4]:struct cli_command *类型，输出参数，表示该命令关键字对应的内部数据结构
*****************************************************************************/
int hmw_mgmtCliRegCmd(unsigned int argc, void *argv[]);






#ifdef _ANDROID_
/*****************************************************************************
Module Name:CPE模块
Module Description:对外接口
*****************************************************************************/
/*****************************************************************************
 Prototype    : hmw_cpeConnectServer_init()
 Description  : Android的系统环境下，cpe模块支持跨进程通讯，此为Socket_Server端
  				的初始化,需在Client之前初始化；
  				
 Input Param  : ;
 Output       : None
 Return Value : None
 Calls        : None
 Called By    : None
 
  History        :
  1.Date         : 2013/05/14
    Author       : skf74590
    Modification : Created function

*****************************************************************************/
int hmw_cpeConnectServer_init();




/*****************************************************************************
 Prototype    : hmw_cpeConnectClient_init
 Description  : 用于Android的Socket_Client端的初始化，支持在其它进程模块调用；
 				但必须保证该函数的调用在hmw_cpeConnectServer_init()之后；

 				另外，需先调用该函数，然后才注册回调；
 Input Param  : ;
 Output       : None
 Return Value : None
 Calls        : None
 Called By    : None
 
  History        :
  1.Date         : 2013/05/14
    Author       : skf74590
    Modification : Created function

*****************************************************************************/
int hmw_cpeConnectClient_init();
#endif


/*****************************************************************************
Module Name:CPE模块
Module Description:实现TR069 Agent功能模块
*****************************************************************************/
/*****************************************************************************
 Prototype    : hmw_mgmtInit
 Description  : 统一管理模块初始化函数，注意必须在设置回调函数之后调用
 Input Param  : ;
 Output       : None
 Return Value : None
 Calls        : None
 Called By    : None
 
  History        :
  1.Date         : 2012/5/29
    Author       : z00109453
    Modification : Created function

*****************************************************************************/
int hmw_mgmtCpeInit(char *dataModel);








/*****************************************************************************
Module Name:Qos日志管理模块
Module Description:实现机顶盒QoS数据统计，并上报服务器
*****************************************************************************/
//定义业务报告类型
typedef enum
{
    HMW_BUSINESS_AUTH   = 1,        /* 鉴权 */
    HMW_BUSINESS_MUTIL  = 2,        /* 多播 */
    HMW_BUSINESS_VOD    = 3,        /* 单播 */
    HMW_BUSINESS_HTTP   = 4,        /* HTTP */
    HMW_BUSINESS_VOD_ABEND,         /* 单播异常断流 */
    HMW_BUSINESS_MULTI_ABEND,       /* 组播异常断流 */
    HMW_BUSINESS_PALY_ERR,          /* 视频播放过程中出错（包括组播和单播），导致无法再播放的次数*/
} HMW_Business_E;

//定义媒体采样周期报告数据类型
typedef struct _HMW_MEDIA_STAT_
{
    HMW_Business_E     eBusiType;         /* 业务类型 2:多播 3:单播 */
    int                iPacketsLostPer;   /* 丢包率 以0.01%为单位 99 表示 0.99% */
    int                iSecPackLost;      /* Multi 时表示经过FEC的,VOD时表示经过ARQ的*/
    int                iBitRate;          /* 实时媒体速率 以100K为单位 13 表示 1.3M */
    unsigned int       uiPacketsLost;     /* 采样周期内丢包数 */
    unsigned int       uiJitter;          /* 采样周期内抖动值 单位 ms */
    unsigned short     usFrameLoss;       /* 采样周期内丢帧数 */
    unsigned char      ucFrameRate;       /* 当前播放帧率 */
}HMW_Media_Stat_S;


/*****************************************************************************
 Prototype    : hmw_mgmtQosInit
 Description  : QoS统计模块初始化
 Input Param  : ;
 Output       : None
 Return Value : int
 Calls        : None
 Called By    : None
 
  History        :
  1.Date         : 2012/08/08
    Author       : skf74590
    Modification : Created function

*****************************************************************************/
int hmw_mgmtQosInit();


/*****************************************************************************
 Prototype    : hmw_mgmtQosReportService
 Description  : QoS统计模块，用于统计HMW_Business_E类型业务接口
 Input Param  : 枚举业务类型
                HMW_Business_E  eType
                该业务失败或者成功，取值1为失败,取值0为成功
                int iRet 
                附加为失败的信息
                const char *pszInfo
 Output       : None
 Return Value : void
 Calls        : None
 Called By    : None
 
  History        :
  1.Date         : 2012/08/08
    Author       : skf74590
    Modification : Created function

*****************************************************************************/
void hmw_mgmtQosReportService(HMW_Business_E eType, int iStat, const char *pErrUrl, int iErrCode);

/*****************************************************************************
 Prototype    : hmw_mgmtQosReportMedia
 Description  : 媒体采样周期报告的数据
 Input Param  :                 
                媒体采样周期内报告的数据类型
                const HMW_Media_Stat_S *pStat
 Output       : None
 Return Value : void
 Calls        : None
 Called By    : None
 
  History        :
  1.Date         : 2012/08/08
    Author       : skf74590
    Modification : Created function

*****************************************************************************/
void hmw_mgmtQosReportMedia(const HMW_Media_Stat_S *pStat);








/*****************************************************************************
Module Name:Stbmonitor Agent

Module Description:与PC端stbmonitor的交互
*****************************************************************************/

/*****************************************************************************
 Prototype    : hmw_mgmtToolInit
 Description  :  启动管理工具stbMonitor_Server端
 Input Param  : 管理工具与机顶盒之间参数、
 				命令等的映射文件(dataModel.xml)路径
 			char  *pstrategyPath
 Output       : None
 Return Value : int
 Calls        : None
 Called By    : None
 
  History        :
  1.Date         : 2012/08/08
    Author       : skf74590
    Modification : Created function

*****************************************************************************/
int hmw_mgmtToolsInit(char *pstrategyPath);


/*****************************************************************************
 Prototype    : hmw_mgmtToolStop
 Description  :  关闭管理工具stbMonitor_Server端
 Input Param  : None
 
 Output       : None
 Return Value : int
 Calls        : None
 Called By    : None
 
  History        :
  1.Date         : 2012/08/08
    Author       : skf74590
    Modification : Created function

*****************************************************************************/
int hmw_mgmtToolsStop();

#ifdef __cplusplus
}
#endif


#endif
