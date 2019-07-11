#ifndef _UpgradeManager_H_
#define _UpgradeManager_H_

#ifdef __cplusplus

#include "config/pathConfig.h"
#include "MessageHandler.h"

#include <vector>
#include <string>
#include <map>

#define UPGRADE_PATH DEFAULT_RAM_DATAPATH"/upgrade_temp.bin"

namespace Hippo {

class UpgradeSource;
class UpgradeChecker;
class UpgradeReceiver;
class UpgradeBurner;
class UpgradeWidget;

class UpgradeManager : public MessageHandler {
public:
    UpgradeManager(int systemType);
    ~UpgradeManager();
    
    enum State {
        UMS_IDLE,
        UMS_CHECK,
        UMS_WAIT_RECEIVE,
        UMS_RECEIVE,
        UMS_WAIT_BURN,
        UMS_BURN
    };
    
    enum UpgradeType {
        UMUT_IP_SOFTWARE = 0,
        UMUT_DVB_SOFTWARE,
        UMUT_MONITOR_SOFTWARE,
        UMUT_UDISK_SOFTWARE,
        UMUT_IP_TEMPLATE,
        UMUT_DVB_TEMPLATE
    };
    
	enum UpgradeContent {
	    UMUC_SOFTWARE   = 0x01,
	    UMUC_LOGO       = 0x02,
	    UMUC_SETTING    = 0x04
	};
	
	enum SystemType {
	    UMST_MINI = 0,
	    UMST_NORMAL,
	    UMST_DEFAULT    
	};
	        
    enum MessageCode {
        UMMC_CHECK_START,
        UMMC_CHECK_END,
        UMMC_RECEIVE_START,
        UMMC_RECEIVE_END,
        UMMC_BURN_START,
        UMMC_BURN_END,
        UMMC_LOGO_RECEIVE_START,
        UMMC_SETTING_RECEIVE_START,
        UMMC_UPGRADE_REQUEST,
        UMMC_AUTO_CHECK_TIMER,
        UMMC_AUTO_UPGRADE_TIMER,
        UMMC_CHANNELLIST_CHECK
    };

    enum MessageInfo {
        UMMI_UPGRADE_START              = 0x9201,
        UMMI_UPGRADE_END                = 0x9202,
        UMMI_GET_SERVER_FAILED          = 0x9208,
        UMMI_GET_CONFIG_FAILED          = 0x9212,
        UMMI_PARSE_CONFIG_FAILED        = 0x9214,
        UMMI_GET_VERSION_FAILED         = 0x921A,
        UMMI_UPGRADE_VERSION_START      = 0x921B,
        UMMI_UPGRADE_VERSION_SUCCEED    = 0x921C,
        UMMI_UPGRADE_VERSION_FAILED     = 0x921D,
        UMMI_UPGRADE_SAME_VERSION       = 0x921E,
        UMMI_UPGRADE_TEMPLATE_FAILED    = 0x921F,
        UMMI_UPGRADE_UNZIP              = 0x9220,
        UMMI_UPGRADE_UDISK_VERSION      = 0x9221,
        UMMI_UPGRADE_UNZIP_FAILED       = 0x9222,
        UMMI_UPGRADE_SHOW_STBIP         = 0x9223,
        UMMI_UPGRADE_VERSION_WRONG      = 0x9224,
        UMMI_WRITE_VERSION_FAILED       = 0x9225,
        UMMI_UPGRADE_PROGRESS_START     = 0x9280
    };
    
    enum UpgradeEvent{
        UMUE_UPGRADE_START = 1,
        UMUE_UPGRADE_FINISH,
        UMUE_UPGRADE_OK,
        UMUE_UPGRADE_FAIL
    };
    
    enum InfoType {
        UMIT_PROMPT = 0,
        UMIT_INFO,
        UMIT_PROGRESS    
    };

public:      
    int upgradeSystemType() { return m_systemType;}

    void setLocalUpgradeCheckFlag (bool flag) { m_isLocalCheckUpgrade = flag; }
    void setUpgradeProvider(int provider) { m_provider = provider; }
    int upgradeProvider() { return m_provider; }	
    int getUpgradeState();
    bool setUpgradeState(int pState);
    bool readUpgradeData();
    void writeUpgradeData(const char *key, const char *value);
    bool getUpgradeData(const char *key, std::string& value);
    bool saveUpgradeData();
    void UpgradeGraphicsInit();
    
    bool touchOffUpgradeCheck(int, bool);
    void responseEvent(int, bool);    
    bool startMonitorORUdiskUpgrade(UpgradeSource* source);
    void sendUpgradeMessage(int type, int arg1, int arg2, unsigned int pDelayMillis);

    int upgradePercent() { return m_percent; }
    bool upgradeTr069Contral() { return m_isTr069Upgrade; }
    UpgradeSource* upgradeSource() { return m_source; }
	
    
protected:    
    virtual void handleMessage(Message* msg);
        
    bool addChecker(UpgradeChecker*);
    bool removeChecker(UpgradeChecker*);
    UpgradeChecker* findChecker(int);

    bool startCheck(UpgradeChecker*);
    bool stopCheck();
    
    bool startCheckTimer();
    bool stopCheckTimer();
    
    bool startReceive(UpgradeSource*);
    bool stopReceive();
    
    void onDestroy();
    void onCheckEnd();
    void onReceiveEnd();
    void onBurnStart();
    void onBurnEnd();
    void onReceiveLogoStart();
    void onReceiverSettingStart();                
    void onUpgradeRequest(UpgradeSource*);
    void onCheckChannellist();
    void burnerEnded();
    
    bool compareVersion(UpgradeSource*);
    void reportEvent(UpgradeChecker*);
    void handleUpgradeMessage(int);
    
    void sendUpgradeEvent(const int event);
    
private:
    int m_systemType;       /*mini system and normal system;*/
    int m_state;      
    int m_softwareVersion;
    int m_logoVersion;
    int m_settingVersion;
    int m_templateVersion;
    int m_updateContent;
    int m_percent;
    int m_provider;	/*0: CTC;   1: CU*/
    bool m_isTr069Upgrade;
    bool m_isLocalCheckUpgrade; // touch off upgrade on local setting page	
    UpgradeReceiver* m_receiver;
    UpgradeBurner* m_burner;
    UpgradeSource* m_source;
    UpgradeWidget *mUpgradeLayOut;
    std::vector<UpgradeChecker*> m_checkerArray;
    std::map<int, int> errMessageInfo;
    std::map<std::string, std::string> mStartInfo;
                       
};

UpgradeManager* upgradeManager();
    
}

#endif //__cplusplus

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
void upgradeManagerCreate(int systemType);
#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_UpgradeManager_H_

