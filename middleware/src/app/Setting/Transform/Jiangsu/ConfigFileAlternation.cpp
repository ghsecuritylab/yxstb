
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include "ConfigFileAlternation.hpp"

#include "Assertions.h"

#include "AppSetting.h"
#include "SysSetting.h"
#include "FrequenceSetting.h"
#include "../SettingTransform.h"
#include "tvms_setting.h"

extern "C" {
#include "libzebra.h"
    extern void nand_init(char*, char*);
    extern void nand_getinfo(int*, int*, int*);
    extern void nand_eraseall(char*, char*, int);
    extern void nand_write(char*, int, int);
    extern void nand_write(char*, int, int);
    extern void nandUnlock(void);
};

namespace Hippo{
/***********************************************************************************************
Description: Constructor. Do nothing.
Date: 1/25/2013 3:49:04 PM
Author:
Input: none
Output: none
Return: none
Remarks: none
***********************************************************************************************/
ConfigFileAlternation::ConfigFileAlternation()
{
    //the member of this->para_config will be inited at where it is needed.
}
/***********************************************************************************************
Description: Destructor. Do nothing.
Date: 1/25/2013 3:49:12 PM
Author:
Input: none
Output: none
Return: none
Remarks: none
***********************************************************************************************/
ConfigFileAlternation::~ConfigFileAlternation()
{
}
/***********************************************************************************************
Description: Copy some items from old config file to the new config file, then save.
Date: 1/25/2013 3:48:51 PM
Author:
Input: none
Output: none
Return: none
Remarks: When we first upgrade the version from old version, use it.
***********************************************************************************************/
int ConfigFileAlternation::MoveToNew(void)
{
    if(GENERAL_ERROR == this->PrepareMoveToNew())
        return GENERAL_ERROR;

    /*the following is constant file*/
    sysSettingSetString("netuser", para_config.Name); // PPPoE account, old 65, new 34
    sysSettingSetString("netAESpasswd", para_config.Pwd); // PPPoE password 16,34
    sysSettingSetInt("nettype", para_config.nettype); // network type ,eth or wireless
    sysSettingSetInt("connecttype", para_config.connectType); // PPPoE, DHCP, Static
    //sys_TransportProtocol_set( para_config.ratioType ); // donot need, use default 3
    sysSettingSetString("ip", para_config.ip ); // when it is static IP, we will use it.
    sysSettingSetString("netmask", para_config.subMask ); // when it is static IP, we will use it.
    sysSettingSetString("gateway", para_config.gateWay ); // when it is static IP, we will use it.
    sysSettingSetString("dns", para_config.Dns); // when it is static IP, we will use it.
    sysSettingSetString("dns1", para_config.Dns1); // when it is static IP, we will use it.
    sysSettingSetString("ntp", para_config.Ntp); // one is ntp serve URL, another is ntp server IP, old 512, new 34
    sysSettingSetString("eds", para_config.epgServer);
    sysSettingSetString("eds1", para_config.Epg);
    appSettingSetString("ntvuser", para_config.userId);
    appSettingSetString("ntvAESpasswd", para_config.userPwd); // the second parameter is 1 that means save the value to flash, 32, 34
    if(0 == para_config.Palorntsc)
	    sysSettingSetInt("videoformat", 1);
    else
	    sysSettingSetInt("videoformat", 0);
    if(ENGLISH == para_config.Lang)
        sysSettingSetInt("lang", 2);
    else
        sysSettingSetInt("lang", 1); // chinese (local)
	sysSettingSetInt("enableDoubleStack", para_config.doublestack);
    sysSettingSetInt("timezone", para_config.Time_zone );
    sysSettingSetString("upgradeUrl", para_config.upgServer);// upgrade sever url
    if(2 == para_config.AspectRatio) // means 16:9
        appSettingSetInt("hd_aspect_mode", 2);
    else // 4:3 or auto
        appSettingSetInt("hd_aspect_mode", 0);
	sysSettingSetInt("tr069_upgrades", para_config.tr069_upgrades);
	sysSettingSetInt("tr069_enable", para_config.tr069_enable);
    tvms_conf_tvmsdelaylength_set( para_config.TVMSDelayTime ); // I am not sure
    tvms_conf_tvmsheartbitinterval_set( para_config.TVMSHeartTime ); // maybe it is
    //( "[sysinfo]\nCur_Version=%s\n", para_config.Cur_Version ); // do not need
    //( "BootFailVersion=%s\n", para_config.BootFailVersion ); // do not need
    //( "LogoVer=%s\n", para_config.logoVer ); // empty in old config file
    //( "SystemSettingVer=%s\n", para_config.settingsVer ); // empty in old config file
    //( "isBootFail=%d\n", para_config.isBootFail ); // 0 in old config file
    //( "logLevel=%d\n", para_config.logLevel );
    //( "tr069_debug=%d\n", para_config.tr069_debug); // delete
    //( "tvms_gw_url=%s\n", para_config.TVMSURL ); // unfound suitable item, it definitely not tvmsgwip


    /*the following is regular file*/
    appSettingSetInt("volume", para_config.volume );
    appSettingSetInt("mute", para_config.mute ); // mute flag
    sysPppoeparamSet( para_config.pppoeSession );
//    ( "[system setting]\nDEVCOM=%s\n", para_config.devCom );
//    ( "DEVID=%s\n", para_config.devId );
//    ( "STBID=%s\n", para_config.stbId );
//    ( "ICKEY=%s\n", para_config.icKey );
//    ( "MAC=%s\n", para_config.Mac );
//    ( "KERNEL=%s\n", para_config.Kernel );
//    ( "BOOT=%s\n", para_config.Boot );
//    ( "HARDWAREID=%s\n", para_config.hardwareId );
//    ( "LOADER=%s\n", para_config.Loader );
//    ( "APP=%s\n", para_config.App );
//    ( "NET=%s\n", para_config.Net );
//    ( "CPU=%s\n", para_config.Cpu );
//    ( "MEMORY=%s\n", para_config.Memory );
//    ( "FLASH=%s\n", para_config.Flash );
//    ( "USB1=%s\n", para_config.Usb1 );
//    ( "USB2=%s\n", para_config.Usb2 );
//    ( "lockValueArray=%s\n",para_config.lockValueArray);
//    ( "EPG_url_from_EDS=%s\n", para_config.EPG_url_from_EDS);
//    ( "areaid=%s\n", para_config.areaid);
//    ( "templateName=%s\n", para_config.templateName);
    //( "lastChanel=%s\n", para_config.lastChanel ); // no appropriate one
//    ( "fratioType=%d\n", para_config.fratioType ); // delete
//    ( "csvCycle=%d\n", para_config.csvCycle );
//    ( "csvAddr=%s\n", para_config.csvAddr );
//    ( "[Debug]\niPanellog_Show=%d\n", para_config.iPanellog_show ); // whether show the ipanel log

    settingManagerSave();
    tvms_config_save();
    sync();

    return OPERATION_SUCCESS;
}
/***********************************************************************************************
Description: Copy the corresponding item from new config file, the save them to the old config file.
Date: 1/25/2013 3:49:21 PM
Author:
Input: none
Output: none
Return: none
Remarks: When we need rollback the version, use it.
***********************************************************************************************/
int ConfigFileAlternation::BackToOld(void)
{
    if(OPERATION_SUCCESS != PrepareBackToOld()){
        LogSysOperError("Rollback error!\n");
        return GENERAL_ERROR;
    }

    sysSettingGetString("netuser", para_config.Name, sizeof(para_config.Name), 0); // PPPoE account
    sysSettingGetString("netAESpasswd", para_config.Pwd, sizeof(para_config.Pwd), 0); // PPPoE password
    sysSettingGetInt("nettype", &para_config.nettype, 0);  // network type ,eth or wireless
    sysSettingGetInt("connecttype", &para_config.connectType, 0); //(YX_NETTYPE)sys_connecttype_get(); // PPPoE, DHCP, Static
    //para_config.ratioType = sys_TransportProtocol_get(); //do not need, use default 0
    sysSettingGetString("ip", para_config.ip , sizeof(para_config.ip), 0); // when it is static IP, we will use it.
    sysSettingGetString("netmask", para_config.subMask, sizeof(para_config.subMask), 0); // when it is static IP, we will use it.
    sysSettingGetString("gateway", para_config.gateWay, sizeof(para_config.gateWay), 0);// when it is static IP, we will use it.
    sysSettingGetString("dns", para_config.Dns, NET_LEN, 0);
    sysSettingGetString("dns1", para_config.Dns1, NET_LEN, 0);
    sysSettingGetString("ntp", para_config.Ntp, sizeof(para_config.Ntp), 0);
    sysSettingGetString("eds", para_config.epgServer, sizeof(para_config.epgServer), 0); //
    sysSettingGetString("eds1", para_config.Epg, sizeof(para_config.Epg), 0); //
    appSettingGetString("ntvuser", para_config.userId, sizeof(para_config.userId), 0);
    appSettingGetString("ntvAESpasswd", para_config.userPwd, 32, 0);

	int videoFormat = 0;
    sysSettingGetInt("videoformat", &videoFormat, 0);
    if(!videoFormat)
        para_config.Palorntsc = (YX_TVMAODE)1;
    else
        para_config.Palorntsc = (YX_TVMAODE)0;

    int lang;
    sysSettingGetInt("lang", &lang, 0);
    if (lang == 1)
        para_config.Lang = CHINESE; //
    else
        para_config.Lang = ENGLISH;
	int doubleStack = 0;
    sysSettingGetInt("enableDoubleStack", &doubleStack, 0);
    para_config.doublestack = (DOUBLE_STACK)doubleStack;
    sysSettingGetInt("timezone", &para_config.Time_zone, 0);
    sysSettingGetString("upgradeUrl", para_config.upgServer, 512, 0); // upgrade sever url
    int aspectRatio = 0;
	appSettingGetInt("hd_aspect_mode", &aspectRatio, 0);
    if(0 == aspectRatio) // means 16:9
        para_config.AspectRatio = AR4TO3;
	else
		para_config.AspectRatio = AR16TO9;
    sysSettingGetInt("tr069_upgrades", &para_config.tr069_upgrades, 0);
	sysSettingGetInt("tr069_enable", &para_config.tr069_enable, 0);
    tvms_conf_tvmsdelaylength_get( &(para_config.TVMSDelayTime) ); // I am not sure
    tvms_conf_tvmsheartbitinterval_get( &(para_config.TVMSHeartTime) ); // maybe it is
    //( "logLevel=%d\n", para_config.logLevel );
    //( "tr069_debug=%d\n", para_config.tr069_debug); // delete
    //( "tvms_gw_url=%s\n", para_config.TVMSURL ); // unfound suitable item, it definitely not tvmsgwip
    //( "[sysinfo]\nCur_Version=%s\n", para_config.Cur_Version ); // do not need
    //( "BootFailVersion=%s\n", para_config.BootFailVersion ); // do not need
    //( "LogoVer=%s\n", para_config.logoVer ); // empty in old config file
    //( "SystemSettingVer=%s\n", para_config.settingsVer ); // empty in old config file
    //( "isBootFail=%d\n", para_config.isBootFail ); // 0 in old config file

    /*the following is regular file*/
	appSettingGetInt("volume", &para_config.volume, 0);
    appSettingGetInt("mute", &para_config.mute, 0);
    sysPppoeparamGet( para_config.pppoeSession );
//    ( "[system setting]\nDEVCOM=%s\n", para_config.devCom );
//    ( "DEVID=%s\n", para_config.devId );
//    ( "STBID=%s\n", para_config.stbId );
//    ( "ICKEY=%s\n", para_config.icKey );
//    ( "MAC=%s\n", para_config.Mac );
//    ( "KERNEL=%s\n", para_config.Kernel );
//    ( "BOOT=%s\n", para_config.Boot );
//    ( "HARDWAREID=%s\n", para_config.hardwareId );
//    ( "LOADER=%s\n", para_config.Loader );
//    ( "APP=%s\n", para_config.App );
//    ( "NET=%s\n", para_config.Net );
//    ( "CPU=%s\n", para_config.Cpu );
//    ( "MEMORY=%s\n", para_config.Memory );
//    ( "FLASH=%s\n", para_config.Flash );
//    ( "USB1=%s\n", para_config.Usb1 );
//    ( "USB2=%s\n", para_config.Usb2 );
//    ( "lockValueArray=%s\n",para_config.lockValueArray);
//    ( "EPG_url_from_EDS=%s\n", para_config.EPG_url_from_EDS);
//    ( "areaid=%s\n", para_config.areaid);
//    ( "templateName=%s\n", para_config.templateName);
    //( "lastChanel=%s\n", para_config.lastChanel );
//    ( "fratioType=%d\n", para_config.fratioType ); // we don't know , use default
//    ( "csvCycle=%d\n", para_config.csvCycle );
//    ( "csvAddr=%s\n", para_config.csvAddr );
//    ( "[Debug]\niPanellog_Show=%d\n", para_config.iPanellog_show ); // whether show the ipanel log

    int ret = this->ConstantSettingWrite(1); // save constant file
    if(0 != ret) {
        LogSafeOperDebug("Save constant file error!\n");
        ret = -1;
    }
    this->RegularSettingWrite(); // save regular file
    sync();
    return ret;
}
/***********************************************************************************************
Description: Prepare to move to new.
Date: 1/25/2013 3:52:46 PM
Author:
Input: none
Output: none
Return: none
Remarks: none
***********************************************************************************************/
int ConfigFileAlternation::PrepareMoveToNew(void)
{
    int ret;
    SetDefaultSetting();
    ret = ConstantSettingRead();
    ret += RegularSettingRead();
    if( ret != 0 ){
        fprintf( stderr, "[%s,%d]:open setting file failed\n", __func__, __LINE__ );
        return GENERAL_ERROR;
    }
    PrintSettingInfo();
    return OPERATION_SUCCESS;
}
/***********************************************************************************************
Description: Prepare to rollback to the old.
Date: 1/25/2013 3:53:15 PM
Author:
Input: none
Output: none
Return: none
Remarks: none
***********************************************************************************************/
int ConfigFileAlternation::PrepareBackToOld(void)
{
    SetDefaultSetting();
    return OPERATION_SUCCESS;
}
/***********************************************************************************************
Description: Copied from old codes.
Date:
Author:
Input: none
Output: none
Return: none
Remarks: none
***********************************************************************************************/
void ConfigFileAlternation::SetDefaultSetting(void)
{
    memset( &para_config, 0, sizeof( CONFIG ) );

    para_config.tr069_upgrades = 1;
    para_config.tr069_enable = 0;
    para_config.tr069_debug = 2;
    para_config.nettype = 0;
    para_config.connectType = DHCP;
    para_config.ratioType = ADAPTIVE;  // 0
    para_config.fratioType = FUDPRTP;
    para_config.doublestack = DOUBLE_STACK_ON;
    para_config.upgrgflag = 1;
    para_config.isViModify1 = 0;
    para_config.isViModify2 = 0;
    para_config.flagHW = BOOL_OFF;
    para_config.flagZTE = BOOL_OFF;
    para_config.flagUT = BOOL_OFF;
    para_config.flagRTSP = BOOL_OFF;
    para_config.flagMODE = BOOL_OFF;
    para_config.logLevel = 0;
    para_config.AspectRatio = AUTO;
    para_config.mute = 1;
    para_config.volume= 50;
    strncpy( para_config.ip, "10.10.10.249", 16 );
    strncpy( para_config.subMask, "255.255.255.0", 16 );
    strncpy(para_config.gateWay,"10.0.0.185",15);
    strncpy(para_config.Dns,"202.106.1.47",15);
    strncpy(para_config.Dns1,"202.106.1.47",15);
    strncpy(para_config.Ntp,"192.168.2.22",15);
    strncpy( para_config.epgServer, EPG_SERVER, 511 );
    strncpy( para_config.Epg, BAK_EPG_SERVER, 511 );
    strncpy(para_config.upgServer, UPGRADE_SERVER,511);
    strncpy(para_config.Cur_Version, "1120.10263.2604", 16);

    return;
}
/***********************************************************************************************
Description: Copied from old codes.
Date:
Author:
Input: none
Output: none
Return: none
Remarks: none
***********************************************************************************************/
void ConfigFileAlternation::PrintSettingInfo(void)
{
    printf(" para_config.Name = %s\n", para_config.Name );
    printf(" para_config.Pwd = %s\n", para_config.Pwd );
    printf(" para_config.netType = %d\n", para_config.nettype );
    printf(" para_config.connectType = %d\n", para_config.connectType );
    printf(" para_config.ratioType = %d\n", para_config.ratioType );
    printf(" para_config.fratioType = %d\n", para_config.fratioType );
    printf(" para_config.ip = %s\n", para_config.ip );
    printf(" para_config.subMask = %s\n", para_config.subMask );
    printf(" para_config.gateWay = %s\n", para_config.gateWay );
    printf(" para_config.Dns = %s\n", para_config.Dns );
    printf(" para_config.Dns1 = %s\n", para_config.Dns1 );
    printf(" para_config.Ntp = %s\n", para_config.Ntp );
    printf(" para_config.epgServer = %s\n", para_config.epgServer );
    printf(" para_config.Epg = %s\n", para_config.Epg );
    printf(" para_config.userId = %s\n", para_config.userId );
    printf(" para_config.userPwd = %s\n", para_config.userPwd );
    printf(" para_config.Palorntsc = %d\n", para_config.Palorntsc );
    printf(" para_config.Lang = %d\n", para_config.Lang );
    printf(" para_config.logLevel = %d\n", para_config.logLevel );
    printf(" para_config.doublestack = %d\n", para_config.doublestack );
    printf(" para_config.Time_zone = %d\n", para_config.Time_zone );
    printf("para_config.lockValueArray = %d\n",para_config.lockValueArray);
    printf(" para_config.upgServer = %s\n", para_config.upgServer );
    printf(" para_config.devCom = %s\n", para_config.devCom );
    printf(" para_config.devId = %s\n", para_config.devId );
    printf(" para_config.stbId = %s\n", para_config.stbId );
    printf(" para_config.icKey = %s\n", para_config.icKey );
    printf(" para_config.Mac = %s\n", para_config.Mac );
    printf(" para_config.Kernel = %s\n", para_config.Kernel );
    printf(" para_config.Boot = %s\n", para_config.Boot );
    printf(" para_config.hardwareId = %s\n", para_config.hardwareId );
    printf(" para_config.Loader = %s\n", para_config.Loader );
    printf(" para_config.App = %s\n", para_config.App );
    printf(" para_config.Net = %s\n", para_config.Net );
    printf(" para_config.Cpu = %s\n", para_config.Cpu );
    printf(" para_config.Memory = %s\n", para_config.Memory );
    printf(" para_config.Flash = %s\n", para_config.Flash );
    printf(" para_config.USB1 = %s\n", para_config.Usb1 );
    printf(" para_config.USB2 = %s\n", para_config.Usb1 );
    printf(" para_config.Cur_Version = %s\n", para_config.Cur_Version );
    printf(" para_config.logoVer = %s\n", para_config.logoVer );
    printf(" para_config.settingsVer = %s\n", para_config.settingsVer );
    printf(" para_config.iPanellog_show = %d\n", para_config.iPanellog_show );
    printf(" para_config.isViModify1 = %d\n", para_config.isViModify1 );
    printf(" para_config.isViModify2 = %d\n", para_config.isViModify2 );

    return;
}
/***********************************************************************************************
Description: Copied from old codes.
Date:
Author:
Input: none
Output: none
Return: none
Remarks: none
***********************************************************************************************/
int ConfigFileAlternation::ConstantSettingRead(void)
{
    int ret;
    int checksum = 0, checksumInFile = 1;

    if( access( CONSTANTCONFIGFILEPATH, R_OK | F_OK ) < 0 ){
        ret = ConstantSettingWrite( 1 );
        return ret;
    }

    checksum = GetSettingChecksum( CONSTANTCONFIGFILEPATH, &checksumInFile );
    printf( "[%s]:checksum = %d, checksumInFile = %d\n", __func__, checksum, checksumInFile );
    printf( "para_config.isViModify1 = %d\n", para_config.isViModify1 );

    if( checksum != checksumInFile ){
        if( 1 == para_config.isViModify1 ){
            para_config.isViModify1 = 0;
            ParseSettingFile( CONSTANTCONFIGFILEPATH );
            ret = ConstantSettingWrite( 1 );
            fprintf( stderr, "YX's enginner modify the system setting file\n" );
            return ret;
        } else {
            checksum = 0;
            fprintf( stderr, "[%s,%d]:Try to get backup file for system setting\n", __func__, __LINE__ );
            checksum = GetSettingChecksum( CONSTANTCONFIGFILEPATHBAK, &checksumInFile);
            printf( "[%s]:checksum = %d, checksumInFile = %d\n", __func__, checksum, checksumInFile );
            if( checksum != checksumInFile ){
                fprintf( stderr, "[%s,%d]:Get backup file for system setting error!\n", __func__, __LINE__ );
                return -2;
            }

            ParseBakSettingFile( CONSTANTCONFIGFILEPATHBAK );
            ret = ConstantSettingWrite( 0 );
            if( ret < 0 ) {
                fprintf( stderr, "[%s,%d]:recover system setting file fail!\n", __func__, __LINE__ );
                return -1;
            }

            fprintf( stderr, "[%s,%d]:recover system setting file successful!\n", __func__, __LINE__ );
            return 0;
        }
    }

    /*force to zero if someone change the contant in the config file but the checkesum is equal to the old value*/
    if( 1 == para_config.isViModify1 ){
        para_config.isViModify1 = 0;
        ParseSettingFile( CONSTANTCONFIGFILEPATH );
        ret = ConstantSettingWrite( 1 );
        fprintf( stderr, "YX's enginner modify the system setting file\n" );
        return ret;
    }

    ParseSettingFile( CONSTANTCONFIGFILEPATH );

    return 0;
}
/***********************************************************************************************
Description: Copied from old codes.
Date:
Author:
Input: none
Output: none
Return: none
Remarks: none
***********************************************************************************************/
int ConfigFileAlternation::RegularSettingRead(void)
{
    int ret;
    int checksum = 0, checksumInFile = 0;
    if( access( REGULARCONFIGFILEPATH, R_OK | F_OK ) < 0 ){
        ret = RegularSettingWrite();
        return ret;
    }
    checksum = GetSettingChecksum( REGULARCONFIGFILEPATH, &checksumInFile );
    printf( "[%s]:checksum = %d, checksumInFile = %d\n", __func__, checksum, checksumInFile );
    if( checksum != checksumInFile ){
        if( 1 == para_config.isViModify2 ){
            para_config.isViModify2 = 0;
            ParseSettingFile( REGULARCONFIGFILEPATH );
            ret = RegularSettingWrite();
            fprintf( stderr, "YX's enginner modify the system setting file\n" );
            return ret;
        } else {
            unlink( REGULARCONFIGFILEPATH );
            fprintf( stderr, "[%s,%d]:read regular setting file failed\n", __func__, __LINE__ );
            return -1;
        }
    }

    /*force to zero if someone change the contant in the config file but the checkesum is equal to the old value*/
    if( 1 == para_config.isViModify2){
        para_config.isViModify2= 0;
        ParseSettingFile( REGULARCONFIGFILEPATH );
        ret = RegularSettingWrite();
        fprintf( stderr, "YX's enginner modify the system setting file\n" );
        return ret;
    }

    ParseSettingFile( REGULARCONFIGFILEPATH );

    return 0;
}
/***********************************************************************************************
Description: Copied from old codes.
Date:
Author:
Input: none
Output: none
Return: none
Remarks: none
***********************************************************************************************/
int ConfigFileAlternation::ConstantSettingWrite( int mode )
{
    char *localBuf =  NULL;
    int i, ret, len = 0, checksum = 0;
    localBuf = ( char * )malloc( CONSTANT_FILE_BUF_MAX );
    if( NULL == localBuf ){
        fprintf( stderr, "[%s,%d]: can't alloc memory!\n", __func__, __LINE__ );
        return -1;
    }

    len += snprintf( localBuf + len, CONSTANT_FILE_BUF_MAX - len, "[info]\nuser=%s\n", para_config.Name );
    len += snprintf( localBuf + len, CONSTANT_FILE_BUF_MAX - len, "password=%s\n", para_config.Pwd );
    len += snprintf( localBuf + len, CONSTANT_FILE_BUF_MAX - len, "[net]\nnettype=%d\n", para_config.nettype );
    len += snprintf( localBuf + len, CONSTANT_FILE_BUF_MAX - len, "connecttype=%d\n", para_config.connectType );
    len += snprintf( localBuf + len, CONSTANT_FILE_BUF_MAX - len, "ratiotype=%d\n", para_config.ratioType );
    len += snprintf( localBuf + len, CONSTANT_FILE_BUF_MAX - len, "ip=%s\n", para_config.ip );
    len += snprintf( localBuf + len, CONSTANT_FILE_BUF_MAX - len, "mask=%s\n", para_config.subMask );
    len += snprintf( localBuf + len, CONSTANT_FILE_BUF_MAX - len, "gateway=%s\n", para_config.gateWay );
    len += snprintf( localBuf + len, CONSTANT_FILE_BUF_MAX - len, "dns=%s\n", para_config.Dns );
    len += snprintf( localBuf + len, CONSTANT_FILE_BUF_MAX - len, "dns1=%s\n", para_config.Dns1 );
    len += snprintf( localBuf + len, CONSTANT_FILE_BUF_MAX - len, "ntp=%s\n", para_config.Ntp );
    len += snprintf( localBuf + len, CONSTANT_FILE_BUF_MAX - len, "[user]\nEPGserver=%s\n", para_config.epgServer );
    len += snprintf( localBuf + len, CONSTANT_FILE_BUF_MAX - len, "EPGserver2=%s\n", para_config.Epg );
    len += snprintf( localBuf + len, CONSTANT_FILE_BUF_MAX - len, "USERID=%s\n", para_config.userId );
    len += snprintf( localBuf + len, CONSTANT_FILE_BUF_MAX - len, "USERPWD=%s\n", para_config.userPwd );
    len += snprintf( localBuf + len, CONSTANT_FILE_BUF_MAX - len, "[basic setting]\nPALORNTSC=%d\n", para_config.Palorntsc );
    len += snprintf( localBuf + len, CONSTANT_FILE_BUF_MAX - len, "LANG=%d\n", para_config.Lang );
    len += snprintf( localBuf + len, CONSTANT_FILE_BUF_MAX - len, "logLevel=%d\n", para_config.logLevel );
    len += snprintf( localBuf + len, CONSTANT_FILE_BUF_MAX - len, "doublestack=%d\n", para_config.doublestack );
    len += snprintf( localBuf + len, CONSTANT_FILE_BUF_MAX - len, "TIME_ZONE=%d\n", para_config.Time_zone );
    len += snprintf( localBuf + len, CONSTANT_FILE_BUF_MAX - len, "UPGSERVER=%s\n", para_config.upgServer );
    len += snprintf( localBuf + len, CONSTANT_FILE_BUF_MAX - len, "AspectRatio=%d\n", para_config.AspectRatio);
    len += snprintf( localBuf + len, CONSTANT_FILE_BUF_MAX - len, "tr069_upgrades=%d\n", para_config.tr069_upgrades);
    len += snprintf( localBuf + len, CONSTANT_FILE_BUF_MAX - len, "tr069_enable=%d\n", para_config.tr069_enable);
    len += snprintf( localBuf + len, CONSTANT_FILE_BUF_MAX - len, "tr069_debug=%d\n", para_config.tr069_debug);
    len += snprintf( localBuf + len, CONSTANT_FILE_BUF_MAX - len, "tvms_gw_url=%s\n", para_config.TVMSURL );
    len += snprintf( localBuf + len, CONSTANT_FILE_BUF_MAX - len, "tvms_delay_span=%d\n", para_config.TVMSDelayTime );
    len += snprintf( localBuf + len, CONSTANT_FILE_BUF_MAX - len, "tvms_heart_span=%d\n", para_config.TVMSHeartTime );
    len += snprintf( localBuf + len, CONSTANT_FILE_BUF_MAX - len, "[sysinfo]\nCur_Version=%s\n", para_config.Cur_Version );
    len += snprintf( localBuf + len, CONSTANT_FILE_BUF_MAX - len, "BootFailVersion=%s\n", para_config.BootFailVersion );
    len += snprintf( localBuf + len, CONSTANT_FILE_BUF_MAX - len, "LogoVer=%s\n", para_config.logoVer );
    len += snprintf( localBuf + len, CONSTANT_FILE_BUF_MAX - len, "SystemSettingVer=%s\n", para_config.settingsVer );
    len += snprintf( localBuf + len, CONSTANT_FILE_BUF_MAX - len, "isBootFail=%d\n", para_config.isBootFail );
    len += snprintf( localBuf + len, CONSTANT_FILE_BUF_MAX - len, "[SWITCH]\nFLAGHW=%d\nFLAGZTE=%d\nFLAGUT=%d\nFLAGRTSP=%d\nFLAGMODE=%d\n", para_config.flagHW, para_config.flagZTE, para_config.flagUT, para_config.flagRTSP, para_config.flagMODE);
    len += snprintf( localBuf + len, CONSTANT_FILE_BUF_MAX - len, "[check]\nisViModify=%d\n", para_config.isViModify1 );

    LogSafeOperDebug("[%s]: len = %d\n", __func__, len );
    for( i = 0; i < len ; i++ ){
        checksum +=  localBuf[ i ];
    }

    len += snprintf( localBuf + len, CONSTANT_FILE_BUF_MAX - len, "checksum=%d\n", checksum );

    LogSafeOperDebug( "[%s]:checksum = %d\n", __func__, checksum );

    if(CONSTANT_FILE_BUF_MAX <= len) //in fact, when the buf is not enough, the value of len will always be equal to the CONSTANT_FILE_BUF_MAX
        LogSafeOperDebug("ERROR! [%s]: len = [%d], is greater than or equal to the MAX value.\n", __func__, len );

    ret = SettingToFile( CONSTANTCONFIGFILEPATH, localBuf, len );
    if( mode == 1 ){
        ret += SettingToFlash( CONSTANTCONFIGFILEPATHBAK, localBuf );
    }
    if( NULL != localBuf ){
        free( localBuf );
        localBuf = NULL;
    }

    if( 0 != ret ){
        fprintf( stderr, "[%s,%d]: save setting file to flash error!\n", __func__, __LINE__ );
    }

    sync( );
    return ret;
}
/***********************************************************************************************
Description: Copied from old codes.
Date:
Author:
Input: none
Output: none
Return: none
Remarks: none
***********************************************************************************************/
int ConfigFileAlternation::RegularSettingWrite(void)
{
    char localBuf[ REGULAR_FILE_BUF_MAX ];
    int i, len = 0, checksum = 0;

    len += snprintf( localBuf + len, REGULAR_FILE_BUF_MAX - len, "[system setting]\nDEVCOM=%s\n", para_config.devCom );
    len += snprintf( localBuf + len, REGULAR_FILE_BUF_MAX - len, "DEVID=%s\n", para_config.devId );
    len += snprintf( localBuf + len, REGULAR_FILE_BUF_MAX - len, "STBID=%s\n", para_config.stbId );
    len += snprintf( localBuf + len, REGULAR_FILE_BUF_MAX - len, "ICKEY=%s\n", para_config.icKey );
    len += snprintf( localBuf + len, REGULAR_FILE_BUF_MAX - len, "MAC=%s\n", para_config.Mac );
    len += snprintf( localBuf + len, REGULAR_FILE_BUF_MAX - len, "KERNEL=%s\n", para_config.Kernel );
    len += snprintf( localBuf + len, REGULAR_FILE_BUF_MAX - len, "BOOT=%s\n", para_config.Boot );
    len += snprintf( localBuf + len, REGULAR_FILE_BUF_MAX - len, "HARDWAREID=%s\n", para_config.hardwareId );
    len += snprintf( localBuf + len, REGULAR_FILE_BUF_MAX - len, "LOADER=%s\n", para_config.Loader );
    len += snprintf( localBuf + len, REGULAR_FILE_BUF_MAX - len, "APP=%s\n", para_config.App );
    len += snprintf( localBuf + len, REGULAR_FILE_BUF_MAX - len, "NET=%s\n", para_config.Net );
    len += snprintf( localBuf + len, REGULAR_FILE_BUF_MAX - len, "CPU=%s\n", para_config.Cpu );
    len += snprintf( localBuf + len, REGULAR_FILE_BUF_MAX - len, "MEMORY=%s\n", para_config.Memory );
    len += snprintf( localBuf + len, REGULAR_FILE_BUF_MAX - len, "FLASH=%s\n", para_config.Flash );
    len += snprintf( localBuf + len, REGULAR_FILE_BUF_MAX - len, "USB1=%s\n", para_config.Usb1 );
    len += snprintf( localBuf + len, REGULAR_FILE_BUF_MAX - len, "USB2=%s\n", para_config.Usb2 );
    len += snprintf( localBuf + len, REGULAR_FILE_BUF_MAX - len, "volume=%d\n", para_config.volume );
    len += snprintf( localBuf + len, REGULAR_FILE_BUF_MAX - len, "mute=%d\n", para_config.mute );
    len += snprintf( localBuf + len, REGULAR_FILE_BUF_MAX - len, "lockValueArray=%s\n",para_config.lockValueArray);
    len += snprintf( localBuf + len, REGULAR_FILE_BUF_MAX - len, "EPG_url_from_EDS=%s\n", para_config.EPG_url_from_EDS);
    len += snprintf( localBuf + len, REGULAR_FILE_BUF_MAX - len, "areaid=%s\n", para_config.areaid);
    len += snprintf( localBuf + len, REGULAR_FILE_BUF_MAX - len, "templateName=%s\n", para_config.templateName);
    len += snprintf( localBuf + len, REGULAR_FILE_BUF_MAX - len, "lastChanel=%s\n", para_config.lastChanel );
    len += snprintf( localBuf + len, REGULAR_FILE_BUF_MAX - len, "fratioType=%d\n", para_config.fratioType );
    len += snprintf( localBuf + len, REGULAR_FILE_BUF_MAX - len, "csvCycle=%d\n", para_config.csvCycle );
    len += snprintf( localBuf + len, REGULAR_FILE_BUF_MAX - len, "pppoe=%s\n", para_config.pppoeSession );
    len += snprintf( localBuf + len, REGULAR_FILE_BUF_MAX - len, "csvAddr=%s\n", para_config.csvAddr );
    len += snprintf( localBuf + len, REGULAR_FILE_BUF_MAX - len, "[Debug]\niPanellog_Show=%d\n", para_config.iPanellog_show );
    len += snprintf( localBuf + len, REGULAR_FILE_BUF_MAX - len, "[check]\nisViModify=%d\n", para_config.isViModify2 );

    printf("[%s]: ret = %d\n", __func__, len );
    for( i = 0; i < len ; i++ ){
        checksum +=  localBuf[ i ];
    }

    len += snprintf( localBuf + len, REGULAR_FILE_BUF_MAX - len, "checksum=%d\n", checksum );

    printf( "[%s]:checksum = %d\n", __func__, checksum );

     if(REGULAR_FILE_BUF_MAX <= len) //in fact, when the buf is not enough, the value of len will always be equal to the BUF_MAX
        LogSafeOperDebug("ERROR! [%s]: len = [%d], is greater than or equal to the MAX value.\n", __func__, len );

    SettingToFile( REGULARCONFIGFILEPATH, localBuf, len );
    sync();
    return 0;
}
/***********************************************************************************************
Description: Copied from old codes.
Date:
Author:
Input: none
Output: none
Return: none
Remarks: none
***********************************************************************************************/
int ConfigFileAlternation::GetSettingChecksum( char *filepathname, int *checksumInFile )
{
    int fd = -1;
    int i, ret =0, isFindChecksum = 1, checksum = 0;
    char localBuf[ FILE_BUF_MAX ], tmp[2]= {0};
    char *pChar = NULL;
    char *pIsViModify = NULL;

    fd = open( filepathname, O_RDONLY );
    if( fd < 0 ) {
        LogSafeOperDebug( "open %s failed\n", filepathname );
        return -1;
    }

    memset( localBuf, 0, FILE_BUF_MAX );
    while( isFindChecksum ){
        ret = read( fd, localBuf, FILE_BUF_MAX - 1 );
        if( ret <= 0 ){
            *checksumInFile = -1;
            checksum = -1;
            break;
        }

        if( (pChar = strstr( localBuf, "checksum=")) ){
            isFindChecksum = 0;
            ret = ( pChar - localBuf );
            pChar += sizeof( "checksum=" );
            *checksumInFile = atoi( pChar -1  );
        }

        if( (pIsViModify = strstr( localBuf, "isViModify=")) ){
            pIsViModify += sizeof( "isViModify=" );
            tmp[0] = *( pIsViModify - 1 );

            if( 0 == strncmp( filepathname, CONSTANTCONFIGFILEPATH, sizeof( CONSTANTCONFIGFILEPATH ) ) )
                para_config.isViModify1 = atoi( tmp  );
            else
                para_config.isViModify2 = atoi( tmp  );
        }

        printf("[%s]: ret = %d\n", __func__, ret );
        for( i = 0; i < ret; i++ )
            checksum += localBuf[ i ];
    }

    close( fd );
    return checksum;
}
/***********************************************************************************************
Description: Copied from old codes.
Date:
Author:
Input: none
Output: none
Return: none
Remarks: none
***********************************************************************************************/
int ConfigFileAlternation::SettingToFile( char *filename, char *dataBuf, int length )
{
    FILE *pFile = NULL;

    pFile = fopen( filename, "w+" );
    if( NULL == pFile ){
        fprintf( stderr, "[%s,%d]:open setting file failed\n", __func__, __LINE__ );
        return -1;
    }

    fwrite( dataBuf, 1, length, pFile );
    fflush( pFile );
    fclose( pFile );
    return 0;
}
/***********************************************************************************************
Description: Copied from old codes.
Date:
Author:
Input: none
Output: none
Return: none
Remarks: none
***********************************************************************************************/
int ConfigFileAlternation::SettingToFlash( char *mtdDev, char *dataBuf ) // write in the backup partition
{
	char *dataBlock = NULL;
	int blocksize, oobsize, pagesize;

	dataBlock = ( char* )malloc( BLOCK_LEN );
	if( NULL == dataBlock ){
		fprintf( stderr, "[%s,%d]:can't alloc memory!\n", __func__, __LINE__ );
		return -1;
	}

	memset( dataBlock, 0, BLOCK_LEN );
	memcpy( dataBlock, dataBuf, BLOCK_LEN/128 );
	nand_init( "p\0", mtdDev );
	nand_getinfo(&pagesize, &oobsize, &blocksize);
	fprintf(stderr, "====Erase %s %d:%d...\n\n", mtdDev,blocksize,oobsize);
	nand_eraseall( mtdDev, NULL, BLOCK_LEN );
	nand_write( dataBlock, BLOCK_LEN, 0 );
	nandUnlock();

	if( NULL != dataBlock ){
		free( dataBlock );
		dataBlock = NULL;
	}

	return 0;
}
/***********************************************************************************************
Description: Copied from old codes.
Date:
Author:
Input: none
Output: none
Return: none
Remarks: none
***********************************************************************************************/
char* ConfigFileAlternation::CopyRightValue( char * strDest, const char* strSrc, int num )
{
    int i = 0;
    char *address = strDest;

    if( (strDest == NULL) || (strSrc == NULL) ){
        fprintf( stderr, "[%s,%d]:oh we got a NULL point\n", __func__, __LINE__ );
        return NULL;
    }

    strSrc++;
    if ( *strSrc == '\n' || *strSrc == '\0' || *strSrc == '\r' ) {
        *(strDest) = '\0';
        return address;
    }

    while( num ){
        *strDest++ = * strSrc++;
        if ( *strSrc == '\n' || *strSrc == '\0' || *strSrc == '\r' )
            break;

        num--;
    }

    *(strDest) = '\0';
    return address;
}
/***********************************************************************************************
Description: Copied from old codes.
Date:
Author:
Input: none
Output: none
Return: none
Remarks: none
***********************************************************************************************/
int ConfigFileAlternation::ParseSettingFile( char *filename )
{
    FILE *pFile = NULL;
    char localBuf[528];
    char *pRightValue = NULL;
    int isBlank = 0;
    fpos_t tmpPos;

    pFile = fopen( filename, "rb" );
    if( NULL == pFile ){
        LogSafeOperDebug( "open constant setting file failed\n" );
        return -1;
    }

    fgetpos( pFile, &tmpPos );
    while( fgets( localBuf, 528, pFile ) ) {
        if( 0 == strncmp("[info]", localBuf, 6 ) ) {
            while( fgets( localBuf, 528, pFile ) ) {
                isBlank = 1;
                if( localBuf[0] == '[' )
                    break;

                fgetpos( pFile, &tmpPos );
                pRightValue = strstr( localBuf, "=" );
                if( 0 == strncmp( "user", localBuf, 4 ) ) {
                    CopyRightValue( para_config.Name, pRightValue, 64 );
                    continue;
                } else if( 0 == strncmp( "password", localBuf, 8 ) ) {
                    CopyRightValue( para_config.Pwd, pRightValue, 15 );
                    continue;
                }
            }
        } else if ( 0 == strncmp("[net]", localBuf, 5 ) ) {
            while( fgets( localBuf, 528, pFile ) ) {
                isBlank = 1;
                if( localBuf[0] == '[' )
                     break;

                fgetpos( pFile, &tmpPos );
                pRightValue = strstr( localBuf, "=" );

                if( 0 == strncmp( "nettype", localBuf, 7 ) ) {
                    para_config.nettype = atoi( pRightValue + 1 );
                    continue;
                } else if( 0 == strncmp( "connecttype", localBuf, strlen("connecttype") ) ) {
                    para_config.connectType = (YX_NETTYPE)atoi( pRightValue + 1 );
                    continue;
                } else if( 0 == strncmp( "ratiotype", localBuf, 9 ) ) {
                    para_config.ratioType = (YX_RATIO)atoi( pRightValue + 1 );
                    continue;
                } else if( 0 == strncmp( "ip", localBuf, 2 ) ) {
                    CopyRightValue( para_config.ip, pRightValue, 15 );
                    continue;
                } else if( 0 == strncmp( "mask", localBuf, 4 ) ) {
                    CopyRightValue( para_config.subMask, pRightValue, 15 );
                    continue;
                } else if( 0 == strncmp( "gateway", localBuf, 7 ) ) {
                    CopyRightValue( para_config.gateWay, pRightValue, 15 );
                    continue;
                } else if( 0 == strncmp( "dns1", localBuf, 4 ) ) {
                    CopyRightValue( para_config.Dns1, pRightValue, 511 );
                    continue;
                } else if( 0 == strncmp( "dns", localBuf, 3 ) ) {
                    CopyRightValue( para_config.Dns, pRightValue, 511 );
//                    ipanel_porting_set_DNSserver1( para_config.Dns );
                    continue;
                } else if( 0 == strncmp( "ntp", localBuf, 3 ) ) {
                    CopyRightValue( para_config.Ntp, pRightValue, 511 );
                    continue;
                }
            }
        } else if ( 0 == strncmp("[user]", localBuf, 6 ) ) {
            while( fgets( localBuf, 528, pFile ) ) {
                isBlank = 1;
                if( localBuf[0] == '[' )
                     break;

                fgetpos( pFile, &tmpPos );
                pRightValue = strstr( localBuf, "=" );

                if( 0 == strncmp( "EPGserver2", localBuf, 10 ) ) {
                    CopyRightValue( para_config.Epg, pRightValue, 511 );
                    continue;
                } else if( 0 == strncmp( "EPGserver", localBuf, 9 ) ) {
                    CopyRightValue( para_config.epgServer, pRightValue, 511 );
                    continue;
                } else if( 0 == strncmp( "USERID", localBuf, 6 ) ) {
                    CopyRightValue( para_config.userId, pRightValue, 64 );
                    continue;
                } else if( 0 == strncmp( "USERPWD", localBuf, 7 ) ) {
                    CopyRightValue( para_config.userPwd, pRightValue, 19 );
                    continue;
                }
            }
        } else if ( 0 == strncmp("[basic setting]", localBuf, 15 ) ) {
            while( fgets( localBuf, 528, pFile ) ) {
                isBlank = 1;
                if( localBuf[0] == '[' )
                     break;

                fgetpos( pFile, &tmpPos );
                pRightValue = strstr( localBuf, "=" );
                if( 0 == strncmp( "PALORNTSC", localBuf, 9 ) ) {
                    para_config.Palorntsc = (YX_TVMAODE)atoi( pRightValue + 1 );
                    continue;
                } else if( 0 == strncmp( "LANG", localBuf, 4 ) ) {
                    para_config.Lang = (YX_LANGUAGE)atoi( pRightValue + 1 );
                    continue;
                 }else if( 0 == strncmp( "logLevel", localBuf, 8) ) {
                    para_config.logLevel = atoi( pRightValue + 1 );
                    continue;
                } else if( 0 == strncmp( "doublestack", localBuf, 11 ) ) {
                    para_config.doublestack = (DOUBLE_STACK)atoi( pRightValue + 1 );
                    continue;
                } else if( 0 == strncmp( "TIME_ZONE", localBuf, 9 ) ) {
                    para_config.Time_zone = atoi( pRightValue + 1 );
                    continue;
                } else if( 0 == strncmp( "UPGSERVER", localBuf, 9 ) ) {
                    CopyRightValue( para_config.upgServer, pRightValue, 511 );
                    continue;
                } else if( 0 == strncmp( "tr069_upgrades", localBuf, 14 ) ) {
                    para_config.tr069_upgrades = atoi( pRightValue + 1 );
                    continue;
                } else if( 0 == strncmp( "tr069_enable", localBuf, 12 ) ) {
                    para_config.tr069_enable = atoi( pRightValue + 1 );
                    continue;
                } else if( 0 == strncmp( "tr069_debug", localBuf, 11 ) ) {
                    para_config.tr069_debug = atoi( pRightValue + 1 );
                    continue;
                } else if( 0 == strncmp( "tvms_gw_url", localBuf, 11 ) ) {
                    CopyRightValue( para_config.TVMSURL, pRightValue, 127 );
                    continue;
                } else if( 0 == strncmp( "tvms_delay_span", localBuf, 15 ) ) {
                    para_config.TVMSDelayTime = atoi( pRightValue + 1 );
                    continue;
                } else if( 0 == strncmp( "AspectRatio", localBuf, 11 ) ) {
                    para_config.AspectRatio = (YX_ASPECTRATIO)atoi( pRightValue + 1 );
                    continue;
                } else if( 0 == strncmp( "tvms_heart_span", localBuf, 15 ) ) {
                    para_config.TVMSHeartTime = atoi( pRightValue + 1 );
                    continue;
                }
            }
        } else if ( 0 == strncmp("[sysinfo]", localBuf, 9 ) ) {
            while( fgets( localBuf, 528, pFile ) ) {
                isBlank = 1;
                if( localBuf[0] == '[' )
                    break;

                fgetpos( pFile, &tmpPos );
                pRightValue = strstr( localBuf, "=" );
                if( 0 == strncmp( "Cur_Version", localBuf, 11 ) ) {
                    CopyRightValue( para_config.Cur_Version, pRightValue, 15 );
                    continue;
                } else if( 0 == strncmp( "BootFailVersion", localBuf, 15 ) ) {
                    CopyRightValue( para_config.BootFailVersion, pRightValue, 15 );
                    continue;
                } else if( 0 == strncmp( "LogoVer", localBuf, 7 ) ) {
                    CopyRightValue( para_config.logoVer, pRightValue, 15 );
                    continue;
                } else if( 0 == strncmp( "SystemSettingVer", localBuf, 16 ) ) {
                    CopyRightValue( para_config.settingsVer, pRightValue, 15 );
                    continue;
                } else if( 0 == strncmp( "isBootFail", localBuf, 10 ) ) {
                    para_config.isBootFail = atoi( pRightValue + 1 );
                    continue;
                }
            }
        }else if( 0 == strncmp("[SWITCH]", localBuf, 8 ) ) {
            while( fgets( localBuf, 528, pFile ) ) {
                isBlank = 1;
                if( localBuf[0] == '[' )
                    break;

                fgetpos( pFile, &tmpPos );
                pRightValue = strstr( localBuf, "=" );
                if( 0 == strncmp( "FLAGHW", localBuf, 6 ) ) {
                    para_config.flagHW= (BOOL)atoi( pRightValue + 1 );
                    continue;
                } else if( 0 == strncmp( "FLAGZTE", localBuf, 7 ) ) {
                    para_config.flagZTE= (BOOL)atoi( pRightValue + 1 );
                    continue;
                } else if( 0 == strncmp( "FLAGUT", localBuf, 6 ) ) {
                    para_config.flagUT= (BOOL)atoi( pRightValue + 1 );
                    continue;
                } else if( 0 == strncmp( "FLAGRTSP", localBuf, 8 ) ) {
                    para_config.flagRTSP= (BOOL)atoi( pRightValue + 1 );
                    continue;
                } else if( 0 == strncmp( "FLAGMODE", localBuf, 8 ) ) {
                    para_config.flagMODE= (BOOL)atoi( pRightValue + 1 );
                    continue;
                }
            }
        }
        else if ( 0 == strncmp("[system setting]", localBuf, 16 ) ) {
            while( fgets( localBuf, 528, pFile ) ) {
                isBlank = 1;
                if( localBuf[0] == '[' )
                     break;

                fgetpos( pFile, &tmpPos );
                pRightValue = strstr( localBuf, "=" );
                if( 0 == strncmp( "DEVCOM", localBuf, 6 ) ) {
                    CopyRightValue( para_config.devCom, pRightValue, 15 );
                    continue;
                } else if( 0 == strncmp( "DEVID", localBuf, 5 ) ) {
                    CopyRightValue( para_config.devId, pRightValue, 15 );
                    continue;
                } else if( 0 == strncmp( "STBID", localBuf, 5 ) ) {
                    CopyRightValue( para_config.stbId, pRightValue, 31 );
                    continue;
                }else if(0 == strncmp("lockValueArray",localBuf,14)) {
                    CopyRightValue(para_config.lockValueArray,pRightValue,31);
                    continue;
                }else if( 0 == strncmp( "ICKEY", localBuf, 5 ) ) {
                    CopyRightValue( para_config.icKey, pRightValue, 31 );
                    continue;
                } else if( 0 == strncmp( "MAC", localBuf, 3 ) ) {
                    CopyRightValue( para_config.Mac, pRightValue, 19 );
                    continue;
                } else if( 0 == strncmp( "KERNEL", localBuf, 6 ) ) {
                    CopyRightValue( para_config.Kernel, pRightValue, 15 );
                    continue;
                } else if( 0 == strncmp( "BOOT", localBuf, 4 ) ) {
                    CopyRightValue( para_config.Boot, pRightValue, 15 );
                    continue;
                } else if( 0 == strncmp( "HARDWAREID", localBuf, 10 ) ) {
                    CopyRightValue( para_config.hardwareId, pRightValue, 15 );
                    continue;
                } else if( 0 == strncmp( "LOADER", localBuf, 6 ) ) {
                    CopyRightValue( para_config.Loader, pRightValue, 15 );
                    continue;
                } else if( 0 == strncmp( "APP", localBuf, 3 ) ) {
                    CopyRightValue( para_config.App, pRightValue, 15 );
                    continue;
                } else if( 0 == strncmp( "NET", localBuf, 3 ) ) {
                    CopyRightValue( para_config.Net, pRightValue, 511 );
                    continue;
                } else if( 0 == strncmp( "CPU", localBuf, 3 ) ) {
                    CopyRightValue( para_config.Cpu, pRightValue, 7 );
                    continue;
                } else if( 0 == strncmp( "MEMORY", localBuf, 6 ) ) {
                    CopyRightValue( para_config.Memory, pRightValue, 7 );
                    continue;
                } else if( 0 == strncmp( "FLASH", localBuf, 5 ) ) {
                    CopyRightValue( para_config.Flash, pRightValue, 7 );
                    continue;
                } else if( 0 == strncmp( "USB1", localBuf, 4 ) ) {
                    CopyRightValue( para_config.Usb1, pRightValue, 15 );
                    continue;
                } else if( 0 == strncmp( "USB2", localBuf, 4 ) ) {
                    CopyRightValue( para_config.Usb2, pRightValue, 15 );
                    continue;
                } else if( 0 == strncmp( "volume", localBuf, 6 ) ) {
                    para_config.volume = atoi( pRightValue + 1);
                    continue;
                } else if( 0 == strncmp( "mute", localBuf, 4 ) ) {
                    para_config.mute = atoi( pRightValue + 1 );
                    continue;
                } else if( 0 == strncmp( "lastChanel", localBuf, 10 ) ) {
                    CopyRightValue( para_config.lastChanel, pRightValue, 15 );
                    continue;
                } else if( 0 == strncmp( "fratioType", localBuf, 10 ) ) {
                    para_config.fratioType = (YX_FRATIO)atoi( pRightValue + 1 );
                    continue;
                }  else if( 0 == strncmp( "csvCycle", localBuf, 8 ) ) {
                    para_config.csvCycle = atoi( pRightValue + 1 );
                    continue;
                } else if( 0 == strncmp( "pppoe", localBuf, 5 ) ){
                    CopyRightValue( para_config.pppoeSession, pRightValue, 20 );
                } else if( 0 == strncmp( "areaid", localBuf, 6 ) ){
                    CopyRightValue( para_config.areaid, pRightValue, 128 );
                } else if( 0 == strncmp( "templateName", localBuf, 12 ) ){
                    CopyRightValue( para_config.templateName, pRightValue, 64 );
                } else if( 0 == strncmp( "csvAddr", localBuf, 7 ) ) {
                    CopyRightValue( para_config.csvAddr, pRightValue, 255 );
                    continue;
                } else if( 0 == strncmp( "EPG_url_from_EDS", localBuf, 16 ) ) {
                    CopyRightValue( para_config.EPG_url_from_EDS, pRightValue, 255 );
                    continue;
                }
            }
        } else if ( 0 == strncmp("[Debug]", localBuf, 7 ) ) {
            while( fgets( localBuf, 528, pFile ) ) {
                isBlank = 1;
                if( localBuf[0] == '[' )
                     break;

                fgetpos( pFile, &tmpPos );
                pRightValue = strstr( localBuf, "=" );
                if( 0 == strncmp( "iPanellog_Show", localBuf, 14 ) ) {
                    para_config.iPanellog_show = atoi( pRightValue + 1 );
                    continue;
                }
            }
        } else if ( 0 == strncmp("[check]", localBuf, 7 ) ) {
            while( fgets( localBuf, 528, pFile ) ) {
                isBlank = 1;
                if( localBuf[0] == '[' )
                     break;

                fgetpos( pFile, &tmpPos );
                pRightValue = strstr( localBuf, "=" );
            }
        }
        if( 1 == isBlank ){
            isBlank = 0;
            fsetpos( pFile, &tmpPos );
        }

        memset( localBuf, 0, 528 );
    }
    fclose( pFile );
    pFile = NULL;
    return 0;
}
/***********************************************************************************************
Description: Copied from old codes.
Date:
Author:
Input: none
Output: none
Return: none
Remarks: none
***********************************************************************************************/
int ConfigFileAlternation::ParseBakSettingFile(const char *filename) // read backup file to memory
{
    int fd = -1, i, ret;
    char *dataBlock = NULL;
    char *pTmp = NULL, tmpBuf[32];

    fd = open( filename, O_RDONLY );
    if( fd < 0 ){
        fprintf( stderr, "[%s,%d]:can't open reserve paritition!\n", __func__, __LINE__ );
        return -1;
    }

    dataBlock = ( char* )malloc( 1024 );
    if( NULL == dataBlock ){
        close( fd );
        fprintf( stderr, "[%s,%d]:can't alloc memory!\n", __func__, __LINE__ );
    }

    memset( dataBlock, 0, 1024 );
    ret = read( fd, ( char * )dataBlock, 1024 );
    memset( &para_config, 0, sizeof( CONFIG ) );
    //[info]
    pTmp = strstr( dataBlock, "user=" );
    pTmp += sizeof( "user" );

    if( 0x0a == *pTmp ){
        para_config.Name[0] = '\0';    /*??3¦Ì¡¤?*/
    } else {
        for( i = 0; i < 64; i++ ){    /*?¨¦?¨¹D¡ì?¨º??*/
            para_config.Name[i] = *pTmp++;
            if( 0x0a == *pTmp )
                break;
        }
    }

    pTmp = strstr( dataBlock, "password=" );
    pTmp += sizeof( "password" );

    if( 0x0a == *pTmp ){
        para_config.Pwd[0] = '\0';    /*??3¦Ì¡¤?*/
    } else {
        for( i = 0; i < 15; i++ ){    /*?¨¦?¨¹D¡ì?¨º??*/
            para_config.Pwd[i] = *pTmp++;
            if( 0x0a == *pTmp )
                break;
        }
    }

    pTmp = strstr( dataBlock, "nettype=" );
    pTmp += sizeof( "nettype" );

    if( 0x0a == *pTmp ){
        para_config.nettype = 0;    /*??3¦Ì¡¤?*/
    } else {
        memset( tmpBuf, 0, 32 );
        for( i = 0; i < 31; i++ ){        /*?¨¦?¨¹D¡ì?¨º??*/
            if( 0x0a == *( pTmp + i ) )
                break;
            tmpBuf[i] = *( pTmp + i );
        }
        para_config.nettype = atoi( tmpBuf );
    }

    pTmp = strstr( dataBlock, "connecttype=" );
    pTmp += sizeof( "connecttype" );

    if( 0x0a == *pTmp ){
        para_config.connectType = DHCP;    /*??3¦Ì¡¤?*/
    } else {
        memset( tmpBuf, 0, 32 );
        for( i = 0; i < 31; i++ ){        /*?¨¦?¨¹D¡ì?¨º??*/
            if( 0x0a == *( pTmp + i ) )
                break;
            tmpBuf[i] = *( pTmp + i );
        }
        para_config.connectType = (YX_NETTYPE)atoi( tmpBuf );
    }

    pTmp = strstr( dataBlock, "ratiotype=" );
    pTmp += sizeof( "ratiotype" );

    if( 0x0a == *pTmp ){
        para_config.ratioType = ADAPTIVE;    /*??3¦Ì¡¤?*/
    } else {
        memset( tmpBuf, 0, 32 );
        for( i = 0; i < 31; i++ ){        /*?¨¦?¨¹D¡ì?¨º??*/
            if( 0x0a == *( pTmp + i ) )
                break;
            tmpBuf[i] = *( pTmp + i );
        }
        para_config.ratioType = (YX_RATIO)atoi( tmpBuf );
    }

    pTmp = strstr( dataBlock, "ip=" );
    pTmp += sizeof( "ip" );

    if( 0x0a == *pTmp ){
        para_config.ip[0] = '\0';    /*??3¦Ì¡¤?*/
    } else {
        for( i = 0; i < 15; i++ ){    /*?¨¦?¨¹D¡ì?¨º??*/
            para_config.ip[i] = *pTmp++;
            if( 0x0a == *pTmp )
                break;
        }
    }

    pTmp = strstr( dataBlock, "mask=" );
    pTmp += sizeof( "mask" );

    if( 0x0a == *pTmp ){
        para_config.subMask[0] = '\0';    /*??3¦Ì¡¤?*/
    } else {
        for( i = 0; i < 15; i++ ){    /*?¨¦?¨¹D¡ì?¨º??*/
            para_config.subMask[i] = *pTmp++;
            if( 0x0a == *pTmp )
                break;
        }
    }

    pTmp = strstr( dataBlock, "gateway=" );
    pTmp += sizeof( "gateway" );

    if( 0x0a == *pTmp ){
        para_config.gateWay[0] = '\0';    /*??3¦Ì¡¤?*/
    } else {
        for( i = 0; i < 15; i++ ){    /*?¨¦?¨¹D¡ì?¨º??*/
            para_config.gateWay[i] = *pTmp++;
            if( 0x0a == *pTmp )
                break;
        }
    }

    pTmp = strstr( dataBlock, "dns1=" );
    pTmp += sizeof( "dns1" );

    if( 0x0a == *pTmp ){
        para_config.Dns1[0] = '\0';    /*??3¦Ì¡¤?*/
    } else {
        for( i = 0; i < 511; i++ ){    /*?¨¦?¨¹D¡ì?¨º??*/
            para_config.Dns1[i] = *pTmp++;
            if( 0x0a == *pTmp )
                break;
        }
    }

    pTmp = strstr( dataBlock, "dns=" );
    pTmp += sizeof( "dns" );

    if( 0x0a == *pTmp ){
        para_config.Dns[0] = '\0';    /*??3¦Ì¡¤?*/
    } else {
        for( i = 0; i < 511; i++ ){    /*?¨¦?¨¹D¡ì?¨º??*/
            para_config.Dns[i] = *pTmp++;
            if( 0x0a == *pTmp )
                break;
        }
    }

    pTmp = strstr( dataBlock, "ntp=" );
    pTmp += sizeof( "ntp" );

    if( 0x0a == *pTmp ){
        para_config.Ntp[0] = '\0';    /*??3¦Ì¡¤?*/
    } else {
        for( i = 0; i < 511; i++ ){    /*?¨¦?¨¹D¡ì?¨º??*/
            para_config.Ntp[i] = *pTmp++;
            if( 0x0a == *pTmp )
                break;
        }
    }

    pTmp = strstr( dataBlock, "EPGserver=" );
    pTmp += sizeof( "EPGserver" );

    if( 0x0a == *pTmp ){
        para_config.epgServer[0] = '\0';    /*??3¦Ì¡¤?*/
    } else {
        for( i = 0; i < 511; i++ ){    /*?¨¦?¨¹D¡ì?¨º??*/
            para_config.epgServer[i] = *pTmp++;
            if( 0x0a == *pTmp )
                break;
        }
    }

    pTmp = strstr( dataBlock, "EPGserver2=" );
    pTmp += sizeof( "EPGserver2" );

    if( 0x0a == *pTmp ){
        para_config.Epg[0] = '\0';    /*??3¦Ì¡¤?*/
    } else {
        for( i = 0; i < 511; i++ ){    /*?¨¦?¨¹D¡ì?¨º??*/
            para_config.Epg[i] = *pTmp++;
            if( 0x0a == *pTmp )
                break;
        }
    }

    pTmp = strstr( dataBlock, "USERID=" );
    pTmp += sizeof( "USERID" );

    if( 0x0a == *pTmp ){
        para_config.userId[0] = '\0';    /*??3¦Ì¡¤?*/
    } else {
        for( i = 0; i < 64; i++ ){    /*?¨¦?¨¹D¡ì?¨º??*/
            para_config.userId[i] = *pTmp++;
            if( 0x0a == *pTmp )
                break;
        }
    }

    pTmp = strstr( dataBlock, "USERPWD=" );
    pTmp += sizeof( "USERPWD" );

    if( 0x0a == *pTmp ){
        para_config.userPwd[0] = '\0';    /*??3¦Ì¡¤?*/
    } else {
        for( i = 0; i < 31; i++ ){    /*?¨¦?¨¹D¡ì?¨º??*/
            para_config.userPwd[i] = *pTmp++;
            if( 0x0a == *pTmp )
                break;
        }
    }

    pTmp = strstr( dataBlock, "PALORNTSC=" );
    pTmp += sizeof( "PALORNTSC" );

    if( 0x0a == *pTmp ){
        para_config.Palorntsc = PAL;    /*??3¦Ì¡¤?*/
    } else {
        memset( tmpBuf, 0, 32 );
        for( i = 0; i < 31; i++ ){        /*?¨¦?¨¹D¡ì?¨º??*/
            if( 0x0a == *( pTmp + i ) )
                break;
            tmpBuf[i] = *( pTmp + i );
        }
        para_config.Palorntsc = (YX_TVMAODE)atoi( tmpBuf );
    }

    pTmp = strstr( dataBlock, "LANG=" );
    pTmp += sizeof( "LANG" );

    if( 0x0a == *pTmp ){
        para_config.Lang = CHINESE;    /*??3¦Ì¡¤?*/
    } else {
        memset( tmpBuf, 0, 32 );
        for( i = 0; i < 31; i++ ){        /*?¨¦?¨¹D¡ì?¨º??*/
            if( 0x0a == *( pTmp + i ) )
                break;
            tmpBuf[i] = *( pTmp + i );
        }
        para_config.Lang = (YX_LANGUAGE)atoi( tmpBuf );
    }

    pTmp = strstr( dataBlock, "AspectRatio=" );
    pTmp += sizeof( "AspectRatio" );

    if( 0x0a == *pTmp ){
        para_config.AspectRatio = AUTO;    /*??3¦Ì¡¤?*/
    } else {
        memset( tmpBuf, 0, 32 );
        for( i = 0; i < 31; i++ ){        /*?¨¦?¨¹D¡ì?¨º??*/
            if( 0x0a == *( pTmp + i ) )
                break;
            tmpBuf[i] = *( pTmp + i );
        }
        para_config.AspectRatio = (YX_ASPECTRATIO)atoi( tmpBuf );
    }

    pTmp = strstr( dataBlock, "logLevel=" );
    pTmp += sizeof( "logLevel" );

    if( 0x0a == *pTmp ){
        para_config.logLevel = 0;    /*??3¦Ì¡¤?*/
    } else {
        memset( tmpBuf, 0, 32 );
        for( i = 0; i < 31; i++ ){        /*?¨¦?¨¹D¡ì?¨º??*/
            if( 0x0a == *( pTmp + i ) )
                break;
            tmpBuf[i] = *( pTmp + i );
        }
        para_config.logLevel = atoi( tmpBuf );
    }

    pTmp = strstr( dataBlock, "doublestack=" );
    pTmp += sizeof( "doublestack" );

    if( 0x0a == *pTmp ){
        para_config.doublestack = DOUBLE_STACK_ON;    /*??3¦Ì¡¤?*/
    } else {
        memset( tmpBuf, 0, 32 );
        for( i = 0; i < 31; i++ ){        /*?¨¦?¨¹D¡ì?¨º??*/
            if( 0x0a == *( pTmp + i ) )
                break;
            tmpBuf[i] = *( pTmp + i );
        }
        para_config.doublestack = (DOUBLE_STACK)atoi( tmpBuf );
    }

    pTmp = strstr( dataBlock, "TIME_ZONE=" );
    pTmp += sizeof( "TIME_ZONE" );

    if( 0x0a == *pTmp ){
        para_config.Time_zone = 0;    /*??3¦Ì¡¤?*/
    } else {
        memset( tmpBuf, 0, 32 );
        for( i = 0; i < 31; i++ ){        /*?¨¦?¨¹D¡ì?¨º??*/
            if( 0x0a == *( pTmp + i ) )
                break;
            tmpBuf[i] = *( pTmp + i );
        }
        para_config.Time_zone = atoi( tmpBuf );
    }

    pTmp = strstr( dataBlock, "UPGSERVER=" );
    pTmp += sizeof( "UPGSERVER" );

    if( 0x0a == *pTmp ){
        para_config.upgServer[0] = '\0';    /*??3¦Ì¡¤?*/
    } else {
        for( i = 0; i < 511; i++ ){    /*?¨¦?¨¹D¡ì?¨º??*/
            para_config.upgServer[i] = *pTmp++;
            if( 0x0a == *pTmp )
                break;
        }
    }

    pTmp = strstr( dataBlock, "tr069_upgrades=" );
    pTmp += sizeof( "tr069_upgrades" );

    if( 0x0a == *pTmp ){
        para_config.tr069_upgrades = 0;    /*??3¦Ì¡¤?*/
    } else {
        memset( tmpBuf, 0, 32 );
        for( i = 0; i < 31; i++ ){        /*?¨¦?¨¹D¡ì?¨º??*/
            if( 0x0a == *( pTmp + i ) )
                break;
            tmpBuf[i] = *( pTmp + i );
        }
        para_config.tr069_upgrades = atoi( tmpBuf );
    }


    pTmp = strstr( dataBlock, "tr069_enable=" );
    pTmp += sizeof( "tr069_enable" );

    if( 0x0a == *pTmp ){
        para_config.tr069_enable = 0;    /*??3¦Ì¡¤?*/
    } else {
        memset( tmpBuf, 0, 32 );
        for( i = 0; i < 31; i++ ){        /*?¨¦?¨¹D¡ì?¨º??*/
            if( 0x0a == *( pTmp + i ) )
                break;
            tmpBuf[i] = *( pTmp + i );
        }
        para_config.tr069_enable = atoi( tmpBuf );
    }

    pTmp = strstr( dataBlock, "tr069_debug=" );
    pTmp += sizeof( "tr069_debug" );

    if( 0x0a == *pTmp ){
        para_config.tr069_debug = 0;    /*??3¦Ì¡¤?*/
    } else {
        memset( tmpBuf, 0, 32 );
        for( i = 0; i < 31; i++ ){        /*?¨¦?¨¹D¡ì?¨º??*/
            if( 0x0a == *( pTmp + i ) )
                break;
            tmpBuf[i] = *( pTmp + i );
        }
        para_config.tr069_debug = atoi( tmpBuf );
    }

    pTmp = strstr( dataBlock, "tvms_gw_url=" );
    pTmp += sizeof( "tvms_gw_url" );

    if( 0x0a == *pTmp ){
        para_config.upgServer[0] = '\0';    /*??3¦Ì¡¤?*/
    } else {
        for( i = 0; i < 511; i++ ){    /*?¨¦?¨¹D¡ì?¨º??*/
            para_config.TVMSURL[i] = *pTmp++;
            if( 0x0a == *pTmp )
                break;
        }
    }

    pTmp = strstr( dataBlock, "tvms_delay_span=" );
    pTmp += sizeof( "tvms_delay_span" );

    if( 0x0a == *pTmp ){
        para_config.TVMSDelayTime = 0;    /*??3¦Ì¡¤?*/
    } else {
        memset( tmpBuf, 0, 32 );
        for( i = 0; i < 31; i++ ){        /*?¨¦?¨¹D¡ì?¨º??*/
            if( 0x0a == *( pTmp + i ) )
                break;
            tmpBuf[i] = *( pTmp + i );
        }
        para_config.TVMSDelayTime = atoi( tmpBuf );
    }

    pTmp = strstr( dataBlock, "tvms_heart_span=" );
    pTmp += sizeof( "tvms_heart_span" );

    if( 0x0a == *pTmp ){
        para_config.TVMSHeartTime = 0;    /*??3¦Ì¡¤?*/
    } else {
        memset( tmpBuf, 0, 32 );
        for( i = 0; i < 31; i++ ){        /*?¨¦?¨¹D¡ì?¨º??*/
            if( 0x0a == *( pTmp + i ) )
                break;
            tmpBuf[i] = *( pTmp + i );
        }
        para_config.TVMSHeartTime = atoi( tmpBuf );
    }

    pTmp = strstr( dataBlock, "Cur_Version=" );
    pTmp += sizeof( "Cur_Version" );

    if( 0x0a == *pTmp ){
        para_config.Cur_Version[0] = '\0';    /*??3¦Ì¡¤?*/
    } else {
        for( i = 0; i < 15; i++ ){    /*?¨¦?¨¹D¡ì?¨º??*/
            para_config.Cur_Version[i] = *pTmp++;
            if( 0x0a == *pTmp )
                break;
        }
    }

//============================================================================================
    pTmp = strstr( dataBlock, "BootFailVersion=" );
    pTmp += sizeof( "BootFailVersion" );

    if( 0x0a == *pTmp ){
        para_config.BootFailVersion[0] = '\0';    /*??3¦Ì¡¤?*/
    } else {
        for( i = 0; i < 15; i++ ){    /*?¨¦?¨¹D¡ì?¨º??*/
            para_config.BootFailVersion[i] = *pTmp++;
            if( 0x0a == *pTmp )
                break;
        }
    }
//==============================================================================================

    pTmp = strstr( dataBlock, "LogoVer=" );
    pTmp += sizeof( "LogoVer" );

    if( 0x0a == *pTmp ){
        para_config.logoVer[0] = '\0';    /*??3¦Ì¡¤?*/
    } else {
        for( i = 0; i < 15; i++ ){    /*?¨¦?¨¹D¡ì?¨º??*/
            para_config.logoVer[i] = *pTmp++;
            if( 0x0a == *pTmp )
                break;
        }
    }

    pTmp = strstr( dataBlock, "SystemSettingVer=" );
    pTmp += sizeof( "SystemSettingVer" );

    if( 0x0a == *pTmp ){
        para_config.settingsVer[0] = '\0';    /*??3¦Ì¡¤?*/
    } else {
        for( i = 0; i < 15; i++ ){    /*?¨¦?¨¹D¡ì?¨º??*/
            para_config.settingsVer[i] = *pTmp++;
            if( 0x0a == *pTmp )
                break;
        }
    }
    pTmp = strstr( dataBlock, "isBootFail=" );
    pTmp += sizeof( "isBootFail" );

    if( 0x0a == *pTmp ){
        para_config.isBootFail = 0;    /*??3¦Ì¡¤?*/
    } else {
        memset( tmpBuf, 0, 32 );
        for( i = 0; i < 31; i++ ){        /*?¨¦?¨¹D¡ì?¨º??*/
            if( 0x0a == *( pTmp + i ) )
                break;
            tmpBuf[i] = *( pTmp + i );
        }
        para_config.isBootFail = atoi( tmpBuf );
    }

    PrintSettingInfo();

    if( NULL != dataBlock ){
        free( dataBlock );
    }
    close( fd );
    return 0;
}

} // namespace Hippo{

using namespace Hippo;
extern "C" {
/***********************************************************************************************
Description: used for c file.
Date: 1/25/2013 3:49:12 PM
Author:
Input: none
Output: none
Return: none
Remarks: none
***********************************************************************************************/
int settingTransform(void)
{
    #define CONSTANTCONFIGFILEPATH	CONFIG_FILE_DIR"/yx_config_constant.ini"

    if( access( CONSTANTCONFIGFILEPATH, R_OK | F_OK )) {
        LogRunOperDebug("Load %s faild.\n", CONSTANTCONFIGFILEPATH);
        return -1;
    }
    LogRunOperDebug("Here is load the config file.\n");
    printf("Begin to import the old items!\n");
    ConfigFileAlternation* oldItems = new ConfigFileAlternation;
    if(NULL != oldItems) {
        oldItems->MoveToNew();
        delete oldItems;
        printf("Import old items success!\n");
#if defined(hi3560e)
       system("rm  -rf "CONSTANTCONFIGFILEPATH);
       system("rm  -rf "CONFIG_FILE_DIR"/yx_config_regular.ini");
#else
       yos_systemcall_runSystemCMD("rm  -rf "CONSTANTCONFIGFILEPATH, NULL);
       yos_systemcall_runSystemCMD("rm  -rf "CONFIG_FILE_DIR"/yx_config_regular.ini", NULL);
#endif
    } else
        printf("ConfigFileAlternation init error! Import failed!\n");

    sync();
    return 0;
}
/***********************************************************************************************
Description: used for c file.
Date: 1/25/2013 3:49:12 PM
Author:
Input: none
Output: none
Return: none
Remarks: none
***********************************************************************************************/
int settingTransformBak(void)
{
    printf("Begin to export the new items!\n");
    ConfigFileAlternation* newItems = new ConfigFileAlternation;
    if(NULL != newItems) {
        newItems->BackToOld();
        delete newItems;
        printf("Export the new items success!\n");
#if defined(hi3560e)
       system("rm  -rf "CONFIG_FILE_DIR"/yx_config_system.ini");
       system("rm  -rf "CONFIG_FILE_DIR"/yx_config_customer.ini");
       system("rm  -rf "CONFIG_FILE_DIR"/yx_config_version.ini");
#else
       yos_systemcall_runSystemCMD("rm  -rf "CONFIG_FILE_DIR"/yx_config_system.ini", NULL);
       yos_systemcall_runSystemCMD("rm  -rf "CONFIG_FILE_DIR"/yx_config_customer.ini", NULL);
       yos_systemcall_runSystemCMD("rm  -rf "CONFIG_FILE_DIR"/yx_config_version.ini", NULL);
#endif
    } else
        printf("ConfigFileAlternation init error! Export failed!\n");
    sync();
    return 0;
}
};

