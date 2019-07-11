/******************************************************************************

  Copyright (C), 2001-2011, Huawei Tech. Co., Ltd.

 ******************************************************************************
  File Name     : hmw_mgmtlib.h
  Version       : Initial Draft
  Author        : z00109453
  Created       : 2012/5/25
  Last Modified :
  Description   : �����ṩ�Ĺ�����ͷ�ļ�
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
���½ڵ㶨��Ϊ��ģ���Զ�����������
************************************************************************/
#define MGMT_BOOTSTRAP                  "Mgmt.BootStrap"            //bootStrapΪ0 or 1;     BOOTSTRAP�¼�
#define MGMT_ACSREBOOT                  "Mgmt.AcsReboot"            //�Ƿ�acs�·�������
#define MGMT_ACSREBOOT_COMMANDKEY      "Mgmt.AcsReboot.CommandKey" //����ǰ��commandKey
#define MGMT_TMSENABLE                  "Mgmt.TMSEnable"            //tms�Ƿ����
//����2��������Ҫ����android���м��δ����ʱʹ�ã�
#define MGMT_ACSUPGRADE_REBOOT         "Mgmt.AcsUpgrade.Reboot"    //�Ƿ�acs�·�������
#define MGMT_ACSUPGRADE_COMMANDKEY    "Mgmt.AcsUpgrade.CommandKey" //����ǰ��commandKey

/*****************************************************************************
Module Name:����ģ��
Module Description:ϵͳ����ģ��
*****************************************************************************/
typedef enum tagHMW_MgmtConfigSRC_E
{
    MGMT_CONFIG_TMS = 0,    //TMS ������ȡ
    MGMT_CONFIG_MT,         //stbmonitor ���߲�����ȡ
    MGMT_CONFIG_OTHERS,     //����������Ŀǰû���ⷽ��Ĳ�����Ԥ��		
}HMW_MgmtConfig_SRC;


/*����ͳһ����ģ��֪ͨҵ��ģ�����Ϣ����
    ��Щ��Ϣ���ͳ���CPE�Ĳ��ء��ϴ����������첽�ģ������Ķ�Ϊͬ����Ϣ��
    ÿ����Ϣ������int (*fnMgmtNotifyCallback)(HMW_MgmtMsgType eMsgType, unsigned int argc, void *argv[]);
    �˻ص����з��ͣ�argc ��ʾ����argv�ĸ�����
    
    ��CPEAgent   ���ⷢ����Ϣ��Я����Ϣ���м��ʱ:
    a, int���͵Ķ���(void *)itemp,ǿת, �յ�Ϊint����ʱ��ֱ��(int)itemp�Ϳ��Ի�ȡ
    ��֮,��Ҫ�м���ṩint���Ͳ���ʱ���򴫵�(void  *)&itemp ���м����д,*(int)itemp = 1
    �Ϳ��Խ�ֵ���أ�

    b, char���ͻ��߽ṹ��ģ�cpeAgentģ�鶼������ռ䣬����ָ���ʽ��    
*/
typedef enum tagHMW_MgmtMsgType_E
{
    MGMT_MESSAGE_NONE           = 0x000,    //NONE MSG

/*���½�ΪACS���·�������*/
    MGMT_CPE_REGISTER_ACS_OK    = 0x101,    //��ACSע��ɹ�:argc = 0;
    
    MGMT_CPE_ACS_REQ_REBOOT     = 0x102,    //ACS֪ͨSTB����:argc = 0;
    
    MGMT_CPE_CONFIG_DOWNLOAD    = 0x103,    //ACS�·�������ַ�������������ļ����ڻָ���config.dat
                                            //�첽: argc = 1; argv[0] = (Mgmt_DownloadInfo *ָ���һ���ڴ�ռ�)
        
    MGMT_CPE_CONFIG_FACTORYRESET    = 0x104,    //ACS�·��������� :argc = 0; ����Ҫ����ֵ�������첽��������
    
    MGMT_CPE_CONFIG_UPLOAD          = 0x105,    //ACS�·��м���ϴ���־����config.dat
                                                //�첽: argc=1; argv[0]=(Mgmt_UpLoadInfo*ָ���һ���ڴ�ռ�)
    
    MGMT_CPE_PLAY_DIAGNOSTICS       = 0x106,    //ACS�·��Ĳ�������첽:argc=3; 
                                                //argv[0]:DiagnosticsState[64], argv[1]:PlayURL[1024]; argv[2]:PlayState

    MGMT_CPE_CALL_VALUECHANG        = 0x107,    //CPEAgent���м������ֵ����;Ŀǰ�ýӿ�Ԥ������ʵ�ʴ���
    
    MGMT_LOG_OUTPUT_CHANNEL_SET     = 0x108,    //CPE��Ҫ��log�����ʽ�����忴enum:hmwMgmtLogChn
                                                //argc = 2; 
                                                
    MGMT_CPE_GET_PLAYSTATE          = 0x109,   //��ȡ��ǰ�Ĳ���״̬url \ state��                                  

/*���½�ΪstbMonitor_pc���·�������*/
    MGMT_MT_PLAYER_BY_CHANNO    = 0x201,    //�·�����ĳ��Ƶ����
                                            //argc = 2; argv[0]:�·���Ƶ����;argv[1] �м�������Ľ��
                                            //argv[1]Ϊint�ͣ�-1��ʾ����ʧ�ܣ�1��ʾ���ųɹ�����*argv[1] = 1;

    MGMT_MT_PLAYER_BY_URL       = 0x202,    //�·�����ĳ��URL
                                            //argc = 2; argv[0]:�·���URL;argv[1] �м�������Ľ��
                                            //argv[1]Ϊint�ͣ�-1��ʾ����ʧ�ܣ�1��ʾ���ųɹ�����*argv[1] = 1;

    MGMT_MT_PLAYER_STOP         = 0x203,    //�·�ֹͣ�������·��Ĳ��ز���;argc=1;
                                            //argv[0]���м����д���ؽ��,1��ʾ�ɹ�������Ϊʧ��
    
    
    MGMT_MT_PLAYER_MPCTRL       = 0x204,    //�·����,����,��ͣ/��ʼ�Ŀ��ƣ�
                                            //�ֱ�Ϊ:"KEY_PAUSE_PLAY""KEY_FAST_FORWARD""KEY_FAST_BACK"��
                                            //argc =0;

    MGMT_MT_TOOL_REBOOT         = 0x205,    //֪ͨ��������;argc =0;    
    
    MGMT_MT_ENTER_DEBUG         = 0x206,    //�·�����debug ;argc =0;
    
    MGMT_MT_EXIT_DEBUG          = 0x207,    //�·��˳�debug ;argc =0;
    
    MGMT_MT_GET_CHANNELNUM_TOTAL    = 0x208,    //��ȡƵ���ܸ���
                                                //argc = 1; argv[0] ���м����дƵ���ܸ��������ں����ȡƵ����Ϣ
    
    MGMT_MT_GET_CHANNELINFO_I       = 0x209,    //���������ȡ��Ƶ�����������ζ�ȡƵ��i����Ϣ
                                                //argc = 2; argv[0]:��ʾƵ����, argv[1]���м����дƵ����Ϣ
    
    MGMT_MT_GET_COLLECT_FILEPATH    = 0x210,    //��ȡһ����λ�ļ��������ļ�·����argc = 2; 
                                                //argv[0]���м����д�ļ��Ƿ����int:1,����,2 �ռ���;������ʾ�ռ�ʧ��
                                                //argv[1],���ļ�����ʱ��д�ļ�·����

/*�����ǹ����߽�����������Ϣ*/
    MGMT_MT_UPGRADE_GET_WORKSTAT        = 0x301,    //��ȡ�ܷ�����argc =1,  argv[0] ,int ��:1 ��ʾ��������,�����쳣
    
    MGMT_MT_UPGRADE_SET_LENGTH          = 0x302,    //����pc�������ļ�(*.bin)�ĳ���argc = 1; argv[0]�ļ�������Ϣ
    
    MGMT_MT_UPGRADE_SET_FORCE           = 0x303,    //�Ƿ�ǿ�������ı�ʶ;argc = 1; argv[0] = 1,��ʾҪǿ������
    
    MGMT_MT_UPGRADE_SET_UPHEADER        = 0x304,    //�����ļ�ͷ��Ϣ����ȡ�ܷ�����
                                                    //argc = 2 ; argv[0]:�����ļ�ͷ���͸�stb, 
                                                    //argv[1]���м����д��ǿ���������Ƿ��������(1:��Ҫ����)��
    
    MGMT_MT_UPGRADE_GET_DOWNHANDLE      = 0x305,    //��ȡ�����ļ�����λ�õľ��
                                                    //argc = 1;  argv[0]:���м����д(FILE * fptr)�������������У�
                                                    //��memcpy(argv[0], &fptr, sizeof(FILE *));
    
    MGMT_MT_UPGRADE_SET_CLOSEWORK       = 0x306,    //��ʼ���أ�֪ͨ�رղ��š�����״̬�����ú��ӽ�����������ҳ��
                                                    //argc = 0;

    MGMT_MT_UPGRADE_SET_DOWNLOAD_PER    = 0x307,    //���ع����и�һ��ʱ�䣬ѭ�������ؽ��ȷ��͸�����
                                                    //argc = 1; argv[0] ,int ��,Ϊ����ֵ;
    
    MGMT_MT_UPGRADE_GET_BURN_PROCESS    = 0x308,    //�м��������¼��������ʱ��stbmonitor��ʱ��ȡ��д����
                                                    //argc = 1; argv[0],���м������
    
    MGMT_MT_UPGRADE_SET_BURN_START      = 0x309,    //�����ļ�������ɺ󣬸�֪�м�����¼�������0x308֮ǰ
                                                    //argc =0;

    MGMT_NT_UPGRADE_NETWORK_DISCONNECT  = 0x310,    //CPEAgent���ع���������Ͽ����ӣ�֪ͨ�м��
                                                    //argc =0;

    MGMT_STB_NETWORK_DISCONNECT         = 0x400,    //���ⲿ����cpe���¼���
    MGMT_STB_AWAIT_DISCONNECT           = 0x401,    //stb�Ͽ���pc�˵����ӣ�����ֹͣ����ˣ�
    MGMT_MESSAGE_END,
}HMW_MgmtMsgType;



//TMS/Stbmonitor�·�����־����/���ͣ����ݸ�����־��ʵ�ֽ���ת��
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

// ��־�������(ftp /file/ �����ͨ��)
typedef enum
{
    HMW_MGMT_ALL_LOG_CLOSE    = 0,  //��־�ر�
    HMW_MGMT_ONLY_FTPLOG_OPEN = 1,      //ftp��־�ϴ�
    HMW_MGMT_ONLY_SYSLOG_OPEN = 2,      //ʵʱ��־�ϴ�
    HMW_MGMT_ALL_LOG_OPEN     = 3,          //ftp/sys��־���ܶ���������cpe/stb�������־�����ʽ��
    HMW_MGMT_LOG_OUTPUT_CNT
}hmwMgmtLogChn;

/*****************************************************************************
     cpe�����õ��Ľṹ��
*****************************************************************************/
//    �����������������
typedef enum
{
    MGMT_CMD_DOWNLOAD =  1,  /* ���� */
    MGMT_CMD_UPLOAD   =  2,  /* �ϴ� */
    MGMT_CMD_DOWNLOAD_CONFIG = 3
}MgmtCmdType;


/*����download�ṹ��*/
typedef struct _MGMT_DownloadInfo_
{
    char       szCommandKey[64];
    char       szFileType[64];       /*�����ļ�����1 Firmware Upgrade  Image,3 Vendor Configuration File*/
    unsigned int        uiFileSize;        /*�ļ���С*/
    char       szURL[256];           /*�����ļ���URL*/
    char       szPassword[256];      /*��¼���·���������*/
    char       szUserName[256];      /*��¼���·������û�*/
    char       szTargetFileName[256];/*���ص��ļ���*/
    int        uiDelaySeconds;        /*�������Ӵ�����ʱ��*/
    char       szSuccessURL[256];    /*���سɹ��󷵻ص�URL*/
    char       szFailureURL[256];    /*����ʧ�ܺ󷵻ص�URL*/
}Mgmt_DownloadInfo;

/*����UpLoadInfo�ṹ��*/
typedef struct _MGMT_UPLAOD_INFO_
{
    char       szCommandKey[64];
    char       szFileType[64];       /*�ϴ��ļ�����1 Firmware Upgrade  Image,3 Vendor Configuration File*/
    char       szURL[256];           /*�����ļ���URL*/
    char       szPassword[256];      /*��¼���·���������*/
    char       szUserName[256];      /*��¼���·������û�*/
    int        uiDelaySeconds;        /*�������ӳ��ϴ�ʱ��*/
}Mgmt_UpLoadInfo;
/*****************************************************************************
 Prototype    : fnMgmtIoCallback
 Description  : ������д�ӿڻص�����
                  ����cpe tr069\stbmonitor\config.dat������õ��Ķ��ֲ���
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
 Description  : Mgmtģ��֪ͨӦ��ģ��Ļص�����
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
 Description  : ע��������ص��ӿ�
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
 Description  : ע��д�����ص��ӿ�
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
 Description  : ��ҵ��ģ��֪ͨ��Ϣ
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
 Description  : ��Mgmtģ���ȡ������Ԥ���ӿڣ�Ŀǰ��֧��
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
 Description  : ��Mgmtģ��д�������Ԥ���ӿڣ�Ŀǰ��֧��
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
 Description  : ��Mgmtģ��֪ͨ��Ϣ
 Input Param  : 
 
 Output       : None
 Return Value : None
 Calls        : None
 Called By    : None
 
  History        :
  1.Date         : 2012/1/20
    Author       : skf74590
    Modification : Created function
	����HMW_MgmtMsgType����Ĳ�ͬ��Ϣ���ݲ�ͬ��argc, *argv[]
*****************************************************************************/
int hmw_mgmtSendMsgToCPEAgent(HMW_MgmtMsgType eMsgType, int argc, void *argv[]);




/*****************************************************************************
 Prototype    : hmw_mgmtValueChange
 Description  : ����Mgmtģ�����(����tr069 ��stbmonitor�����ߵĲ���)��������д��
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
 Description  : Mgmtģ����־����Ļص�����
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
 Description  :  ע��д��־�ص��ӿ�
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
Module Name:Alarmģ��
Module Description:ʵ��ϵͳͳһ�����빦��ģ��
*****************************************************************************/

typedef enum 
{
    ALARM_LEVEL_CRITICAL       = 1, /* ���� */
    ALARM_LEVEL_MAJOR          = 2, /* ��Ҫ */
    ALARM_LEVEL_MINOR          = 3, /* ��Ҫ */
    ALARM_LEVEL_WARNING        = 4, /* ��ʾ */
    ALARM_LEVEL_INDETERMINATE  = 5, /* ��ȷ�� */
    ALARM_LEVEL_CLEARED        = 6  /* ����� */
}HMW_CPEAlarmLevel_E;

/******************************************************
   �����ö��������ø澯ID��
   �ⲿ�澯��ID�Ķ�����Ҫ�ӽ���
*******************************************************/
#define    MGMT_ALARM_DISK_OPEREATE         "100102"        //  ���̶�дʧ��
#define    MGMT_ALARM_CPU_USAGE             "100103"        // CPUʹ���ʳ�����ֵ
#define    MGMT_ALARM_DISK_USAGE            "100104"        //����ʹ���ʳ�����ֵ
#define    MGMT_ALARM_MEM_USAGE             "100105"        // �ڴ�ʹ�ðٷֱȳ�����ֵ
#define    MGMT_ALARM_PACKET_LOST           "100106"        //�����ʳ�����ֵ�澯 
#define    MGMT_ALARM_DECODE_FAIL           "100108"        //�����н������澯
#define    MGMT_ALARM_UPGRADE_FAIL          "300101"        //����ʧ��
#define    MGMT_ALARM_AUTH_FAIL             "300103"        //IPTVҵ����֤ʧ�ܸ澯.
#define    MGMT_ALARM_JOIN_CHANNEL          "300104"        //����Ƶ��ʧ�ܸ澯


int  hmw_mgmtAlarmInit();
/*****************************************************************************
 Prototype    : hmw_mgmtAlarmReport
 Description  : �澯�ϱ�����
 Input Param  : �澯ID�����϶���MGMT_ALARM_JOIN_CHANNEL "300104"���ݻ������������
 		          const char* pAlarmID,
 		         �澯����1��ʾ�澯�ϱ���0��ʾ�澯�ָ�
 		         int iAlarmType,
 		         �澯����HMW_CPEAlarmLevel_E����1-6�������
 		         int iLevel
 		         �澯������Ϣ,�ɵ������� ��д
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
Module Name:CLI������ģ��
Module Description:ʵ��ϵͳ�����й���ģ��
*****************************************************************************/
//����ͬʱ��½�ͻ��˸���
#define MGMT_CLI_MAX_CONN 1

//����ע��������ʱ����Я���Ĳ�������
#define MGMT_CLI_PARM_NUM_REG 5

//����ȥע��������ʱ����Я���Ĳ�������
#define MGMT_CLI_PARM_NUM_UNREG 1


/*****************************************************************************
 Prototype    : hmwMgmtCliCallback
 Description  : �����лص�����
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
 Description  : ������ģ���ʼ������
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
 Description  : ������ģ��ȥ��ʼ������
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
 Description  : ������ģ��ע�������
 Input          :unsigned int argc, void *argv[]
 Output       : None
 Return Value : None
 Calls        : None
 Called By    : None
 
  History        :
  1.Date         : 2012/5/28
    Author       : z00109453
    Modification : Created function
    ע��argc����д�ɹ̶���ΪMGMT_CLI_PARM_NUM_REG(5��)
    argv[0]:struct cli_command *���ͣ���ʾ������ؼ��ֵĸ��ؼ��֣�����ȡֵΪNULL����ʾ�ùؼ���Ϊ��һ���ؼ���
    argv[1]:char *����ʾҪע�������ؼ���
    argv[2]:hmwMgmtCliCallback����ʾ������Ļص�������
    argv[3]:char *����ʾҪע������������Ϣ
    argv[4]:struct cli_command *���ͣ������������ʾ������ؼ��ֶ�Ӧ���ڲ����ݽṹ
*****************************************************************************/
int hmw_mgmtCliRegCmd(unsigned int argc, void *argv[]);






#ifdef _ANDROID_
/*****************************************************************************
Module Name:CPEģ��
Module Description:����ӿ�
*****************************************************************************/
/*****************************************************************************
 Prototype    : hmw_cpeConnectServer_init()
 Description  : Android��ϵͳ�����£�cpeģ��֧�ֿ����ͨѶ����ΪSocket_Server��
  				�ĳ�ʼ��,����Client֮ǰ��ʼ����
  				
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
 Description  : ����Android��Socket_Client�˵ĳ�ʼ����֧������������ģ����ã�
 				�����뱣֤�ú����ĵ�����hmw_cpeConnectServer_init()֮��

 				���⣬���ȵ��øú�����Ȼ���ע��ص���
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
Module Name:CPEģ��
Module Description:ʵ��TR069 Agent����ģ��
*****************************************************************************/
/*****************************************************************************
 Prototype    : hmw_mgmtInit
 Description  : ͳһ����ģ���ʼ��������ע����������ûص�����֮�����
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
Module Name:Qos��־����ģ��
Module Description:ʵ�ֻ�����QoS����ͳ�ƣ����ϱ�������
*****************************************************************************/
//����ҵ�񱨸�����
typedef enum
{
    HMW_BUSINESS_AUTH   = 1,        /* ��Ȩ */
    HMW_BUSINESS_MUTIL  = 2,        /* �ಥ */
    HMW_BUSINESS_VOD    = 3,        /* ���� */
    HMW_BUSINESS_HTTP   = 4,        /* HTTP */
    HMW_BUSINESS_VOD_ABEND,         /* �����쳣���� */
    HMW_BUSINESS_MULTI_ABEND,       /* �鲥�쳣���� */
    HMW_BUSINESS_PALY_ERR,          /* ��Ƶ���Ź����г��������鲥�͵������������޷��ٲ��ŵĴ���*/
} HMW_Business_E;

//����ý��������ڱ�����������
typedef struct _HMW_MEDIA_STAT_
{
    HMW_Business_E     eBusiType;         /* ҵ������ 2:�ಥ 3:���� */
    int                iPacketsLostPer;   /* ������ ��0.01%Ϊ��λ 99 ��ʾ 0.99% */
    int                iSecPackLost;      /* Multi ʱ��ʾ����FEC��,VODʱ��ʾ����ARQ��*/
    int                iBitRate;          /* ʵʱý������ ��100KΪ��λ 13 ��ʾ 1.3M */
    unsigned int       uiPacketsLost;     /* ���������ڶ����� */
    unsigned int       uiJitter;          /* ���������ڶ���ֵ ��λ ms */
    unsigned short     usFrameLoss;       /* ���������ڶ�֡�� */
    unsigned char      ucFrameRate;       /* ��ǰ����֡�� */
}HMW_Media_Stat_S;


/*****************************************************************************
 Prototype    : hmw_mgmtQosInit
 Description  : QoSͳ��ģ���ʼ��
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
 Description  : QoSͳ��ģ�飬����ͳ��HMW_Business_E����ҵ��ӿ�
 Input Param  : ö��ҵ������
                HMW_Business_E  eType
                ��ҵ��ʧ�ܻ��߳ɹ���ȡֵ1Ϊʧ��,ȡֵ0Ϊ�ɹ�
                int iRet 
                ����Ϊʧ�ܵ���Ϣ
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
 Description  : ý��������ڱ��������
 Input Param  :                 
                ý����������ڱ������������
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

Module Description:��PC��stbmonitor�Ľ���
*****************************************************************************/

/*****************************************************************************
 Prototype    : hmw_mgmtToolInit
 Description  :  ����������stbMonitor_Server��
 Input Param  : �������������֮�������
 				����ȵ�ӳ���ļ�(dataModel.xml)·��
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
 Description  :  �رչ�����stbMonitor_Server��
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
