#include "NativeHandlerLittleSystem.h"
#include "NativeHandlerAssertions.h"
#include "UpgradeManager.h"
#include "Message.h"
#include "MessageTypes.h"
#include "mgmtModule.h"
#include "browser_event.h"
#include "EmergencyDialog.h"
#include "UDiskDetect.h"
#include "Customer.h"

#include "BootImagesShow.h"
#include "SysSetting.h"
#include "config.h"
#include "libzebra.h"
#include "scbt_global.h"
#include "io_xkey.h"
#include "sys_msg.h"

#include <string.h>

#include<unistd.h>

#include "NetworkFunctions.h"

namespace Hippo {

NativeHandlerLittleSystem::NativeHandlerLittleSystem()
    : mUpgradeMode(UpgradeManager::UMUT_IP_SOFTWARE)
    , mIsHandleKey(false)
    , mPressOKCount(0)
    , mTr069Contral(0)
{
}

NativeHandlerLittleSystem::~NativeHandlerLittleSystem()
{

}

void
NativeHandlerLittleSystem::onActive()
{
    //DONT_SET(NMB_SHOW_ERROR_CODE);
    //DONT_SET(NMB_SHOW_BOTTOM_ICON);
    //DONT_SET(NMB_AUTO_DOMENU);
    //DONT_SET(NMB_REJOIN_WIRELESS);
}

void
NativeHandlerLittleSystem::onUnactive()
{
    DONT_ZERO();
}



bool
NativeHandlerLittleSystem::handleMessage(Message *msg)
{
    NATIVEHANDLER_LOG_VERBOSE("what = 0x%x, arg1 = 0x%x, arg2 = 0x%x\n", msg->what, msg->arg1, msg->arg2);
    if(msg->what == MessageType_KeyDown) {
        if (msg->arg1 == EIS_IRKEY_SELECT) {
            if (mIsHandleKey)
                onEmergency();
        } else {
            if (mDialog)
                mDialog->handleMessage(msg);
        }
       return true;
    }
    if (msg->what == MessageType_Network || msg->arg1 == MV_Network_ConnectOk) {
        switch (mUpgradeMode) {
        case UpgradeManager::UMUT_IP_SOFTWARE:
            if (mTr069Contral)
                upgradeManager()->touchOffUpgradeCheck(UpgradeManager::UMUT_IP_SOFTWARE, true);
            else
                upgradeManager()->touchOffUpgradeCheck(UpgradeManager::UMUT_IP_SOFTWARE, false);
            break;
        case UpgradeManager::UMUT_UDISK_SOFTWARE:
            UDiskUpgradeInEmergency(1);
            break;
        case UpgradeManager::UMUT_MONITOR_SOFTWARE:
            upgradeManager()->UpgradeGraphicsInit();
	        upgradeManager()->setUpgradeState(UpgradeManager::UMMI_UPGRADE_SHOW_STBIP);
            #ifndef ANDROID
            setMonitorState(1);
            #endif
            break;
        default:
            break;
        }
        return true;
    }

    switch (msg->arg1) {
    case LITTLE_SYSTEM_RUN:
        runSystem(msg->arg2);
        break;
    case CHECK_VERSION_VALIDITY:
        checkAppVersion();
        break;
    case LITTLE_SYSTEM_UDISK_UPGRADE_FILAID:
        if (mUpgradeMode == UpgradeManager::UMUT_UDISK_SOFTWARE)
            return NativeHandlerPublic::doRequestReboot();
        startNetWork();
        mUpgradeMode = UpgradeManager::UMUT_IP_SOFTWARE;
        mTr069Contral = 0;
        return true;
    case LITTLE_SYSTEM_SERVER_UPGRADE_FILAID:
        upgradeManager()->UpgradeGraphicsInit();
        upgradeManager()->setUpgradeState(UpgradeManager::UMMI_UPGRADE_SHOW_STBIP);
        #ifndef ANDROID
        setMonitorState(1);
        #endif
        return true;
    case EIS_IRKEY_USB_UNINSERT:
        if (upgradeManager()->getUpgradeState() == UpgradeManager::UMMI_UPGRADE_UNZIP)
            upgradeManager()->setUpgradeState(UpgradeManager::UMMI_UPGRADE_UNZIP_FAILED);
        break;
    default:
        break;
    }
    return NativeHandlerPublic::handleMessage(msg);
}

void
NativeHandlerLittleSystem::startNetWork()
{
    char ifname[URL_LEN] = { 0 };
    NetworkInterface* iface = networkManager().getInterface(network_default_ifname(ifname, URL_LEN));
    if (iface)
        iface->connect();
}

void
NativeHandlerLittleSystem::checkAppVersion()
{
#ifndef INCLUDE_EMERGENCY_START
    printf("not need verify !\n");
    exit(0);
#endif
    mIsHandleKey = false;
    int ret = 0;
    int upgradeForceFlag = 0;
    sysSettingGetInt("upgradeForce", &upgradeForceFlag, 0);
    if (1 == upgradeForceFlag) {
        printf("Force upgrade, now!\n");
        InitApp();
        runSystem(1);
        return;
    }
#ifdef HUAWEI_C20
    int tIndex = 0;
    int blockSize = 0;
    int eraseSize = 0;
    stSecureBootInfo stSignInfo;
    char *app0buf = NULL;
    char *app1buf = NULL;
    char *signbuf = NULL;

    char *flag = NULL;
    int ret0 = -1;
    ret0 = yhw_env_readString( "APPUPGRADEOK", &flag );
    if (!ret0 && !strcmp(flag, "y")){
        /*read app0 data*/
        yhw_flash_mtdblock_info ((char *)"app0" ,&tIndex, &blockSize, &eraseSize);
        app0buf = (char *)malloc(blockSize);
        memset(app0buf, 0, blockSize);
        yhw_flash_read_appblock((char *)"app0", app0buf);


        /*read app1 data*/
        yhw_flash_mtdblock_info ((char *)"app1", &tIndex, &blockSize, &eraseSize);
        app1buf = (char *)malloc(blockSize);
        memset(app1buf, 0, blockSize);
        yhw_flash_read_appblock((char *)"app1", app1buf);


        /*read sign data*/
        yhw_flash_mtdblock_info ((char *)"sign", &tIndex, &blockSize, &eraseSize);
        signbuf = (char *)malloc(blockSize);
        memset(signbuf, 0, blockSize);
        yhw_flash_read_appblock((char *)"sign", signbuf);

       /*get the last 3 block data */
       memcpy(&stSignInfo, signbuf, 304);

       ret = scbt_api_verify_app_data(app0buf, app1buf, &stSignInfo);

       free(app0buf);
       free(app1buf);
       free(signbuf);
   	}else {
   	   ret = 0;
   	}
#else
    char *flag = NULL;
    int ret0 = -1;
    ret0 = yhw_env_readString( "APPUPGRADEOK", &flag );
    if (!ret0 && !strcmp(flag, "y"))
        ret = 1;
    else
        ret = 0;
#endif
    if (ret == 1) {
        printf("verify app success\n");
        exit(0);
    } else {
        printf("app block check failed\n");
		InitApp();
        sysSettingSetInt("upgradeForce", 1);
        int upgradeOK = 0;
        int firstOK = 0;
        sysSettingGetInt("isFirstUpgradeOK", &firstOK, 0);
        sysSettingGetInt("IPUpgradeOK", &upgradeOK, 0);
        if (upgradeOK) {
            if (0 == firstOK || 1 == firstOK)
               // upgradeManager()->sendUpgradeEvent(UpgradeManager::EVENT_UPGRADE_FAIL);
            sysSettingSetInt("IPUpgradeOK", 0);
            sysSettingSetInt("isFirstUpgradeOK", 0);
        }
        UDiskUpgradeInEmergency(1);
    }
}

void
NativeHandlerLittleSystem::runSystem(int step)
{
    if (step == 1) {
        mUpgradeMode = UpgradeManager::UMUT_IP_SOFTWARE;
        startNetWork();
        return;
    }

    upgradeManagerCreate(0);
#ifndef INCLUDE_EMERGENCY_START

    bool status = upgradeManager()->readUpgradeData();
    std::string strValue;
    if (status) {
        status = upgradeManager()->getUpgradeData("upgradeType", strValue);
        if (status) {
            InitApp();
            mUpgradeMode = atoi(strValue.c_str());
            strValue.clear();
            upgradeManager()->getUpgradeData("upgradeTr069", strValue);
            mTr069Contral = atoi(strValue.c_str());
            if (mUpgradeMode == UpgradeManager::UMUT_UDISK_SOFTWARE) {
                Message *msg = NULL;
                msg = defNativeHandler().obtainMessage(MessageType_Network, NETWORK_CONNECT_OK, 0);
                defNativeHandler().sendMessage(msg);
            } else
                startNetWork();
            return;
        }
    }
#endif
    mPressOKCount = 0;
    mIsHandleKey = true;
    Message *msg = defNativeHandler().obtainMessage(MessageType_Unknow, CHECK_VERSION_VALIDITY, 0);
    defNativeHandler().sendMessageDelayed(msg, 1500);
}

void
NativeHandlerLittleSystem::onEmergency()
{
    if (mPressOKCount == 0) {
        defNativeHandler().removeMessages(MessageType_Unknow);
        Message *msg = defNativeHandler().obtainMessage(MessageType_Unknow, CHECK_VERSION_VALIDITY, 0);
        defNativeHandler().sendMessageDelayed(msg, 2000);
        mPressOKCount = 1;
        return;
    }
    mPressOKCount++;
    if (mPressOKCount == 3) {
        InitApp();
        mPressOKCount = 0;
        defNativeHandler().removeMessages(MessageType_Unknow);
        mDialog = new EmergencyDialog(1);
        mDialog->draw();
    }
}

void
NativeHandlerLittleSystem::InitApp()
{
    mIsHandleKey = false;
    io_xkey_reg(NULL);
    yhw_IRinputsystem_uninit();
    sleep(1);
    //yhw_board_init();
    io_xkey_reg((xkey_call)sys_msg_port_irkey);

    mid_task_init( );
    mid_timer_init( );
    mid_http_init( );
    mid_dns_init( );
    mid_net_init();

    GraphicsConfig();
    BootImagesShowLogoInit();
}

}
