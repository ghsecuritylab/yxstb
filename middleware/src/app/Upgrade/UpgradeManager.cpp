#include "UpgradeManager.h"

#include "UpgradeAssertions.h"
#include "UpgradeSource.h"
#include "UpgradeIPSource.h"
#include "UpgradeChecker.h"
#include "UpgradeIPChecker.h"
#include "TemplateIPUpgradeChecker.h"
#include "UpgradeReceiver.h"
#include "UpgradeBurner.h"
#include "UpgradeTemplateBurner.h"
#include "UdiskUpgrade.h"
#include "UpgradeData.h"
#include "Synchronized.h"
#include "MessageTypes.h"
#include "MessageValueSystem.h"
#include "UpgradeWidget.h"
#include "StandardScreen.h"
#include "BrowserEventQueue.h"
#include "NativeHandler.h"
#include "BrowserAgent.h"
#include "SystemManager.h"
#include "config/pathConfig.h"
#include "NetworkFunctions.h"
#include "ProgramParser.h"
#include "UpgradeCommon.h"
#include "AppSetting.h"
#include "SysSetting.h"
#include "VersionSetting.h"
#include "SettingEnum.h"
#include "Business.h"
#include "Session.h"
#include "stbinfo/stbinfo.h"

#include "customer.h"
#include "Tr069.h"

#include "UtilityTools.h"
#include "config.h"

#include "sys_basic_macro.h"

#include "mid_sys.h"
#include "mid/mid_http.h"
#include "mid/mid_tools.h"
#include "dns/mid_dns.h"
#include "build_info.h"
#include "json/json.h"
#include "json/json_object.h"
#include "json/json_public.h"

#include <netinet/in.h>
#include <arpa/inet.h>

extern char* global_cookies;

namespace Hippo {

#ifdef INCLUDE_TR069

static int upgradeError[][2] = {
{UpgradeManager::UMMI_GET_SERVER_FAILED, UPGRADE_CONNECT_SERVER_FAIL},
{UpgradeManager::UMMI_GET_CONFIG_FAILED, UPGRADE_NET_DISCONNECT},
{UpgradeManager::UMMI_PARSE_CONFIG_FAILED, UPGRADE_ILLEGAL_VERSION},
{UpgradeManager::UMMI_GET_VERSION_FAILED, UPGRADE_NET_DISCONNECT},
{UpgradeManager::UMMI_UPGRADE_VERSION_FAILED,UPGRADE_ILLEGAL_VERSION},
{UpgradeManager::UMMI_UPGRADE_SAME_VERSION, UPGRADE_SAME_VERSION_AS_SERVERS},
{UpgradeManager::UMMI_UPGRADE_VERSION_SUCCEED, 1}
};

#endif

static bool g_result = false;

UpgradeManager::UpgradeManager(int systemType)
    : m_systemType(systemType)
    , m_state(UMS_IDLE)
    , m_source(0)
    , m_percent(0)
    , m_isTr069Upgrade(false)
    , m_isLocalCheckUpgrade(false)
    , m_provider(0)
    , m_receiver(0)
    , m_burner(0)
{
    char currentVersion[64] = {0};
    int version = 0;

    strcpy(currentVersion, g_make_build_version);
    char* p1 = NULL;
    p1 = strstr(currentVersion, ".");
    if (p1) {
        char* p2 = strstr(p1 + 1, ".");
        if (p2)
            p1 = p2 + 1;
    }
    if (p1)
        version = atoi(p1);

    m_softwareVersion = upgrade_version_read(SOFTWARE_VERSION);
    m_logoVersion = upgrade_version_read(LOGO_VERSION);
    m_settingVersion = upgrade_version_read(SETTING_VERSION);
    m_templateVersion = upgrade_version_read(TEMPLATE_VERSION);
    if (version != 0 && version != m_softwareVersion) {
        upgrade_version_write(SOFTWARE_VERSION, version);
        m_softwareVersion = version;
    }
    int isSendSuccessInfo;
    sysSettingGetInt("IPUpgradeOK", &isSendSuccessInfo, 0);
    if (isSendSuccessInfo)
        sendEmptyMessage(UMMC_CHANNELLIST_CHECK);

    UpgradeLogDebug("localVersion = %d, templateVersion = %d \n", m_softwareVersion, m_templateVersion);

#ifdef INCLUDE_TR069
    for(int i = 0; i < sizeof(upgradeError) / sizeof(upgradeError[0]); i++)
        errMessageInfo.insert(std::make_pair(upgradeError[i][0], upgradeError[i][1]));
#endif

#ifdef INCLUDE_LITTLESYSTEM
    if (m_systemType == UMST_MINI)
        mUpgradeLayOut = new UpgradeWidget();
#endif
}

UpgradeManager::~UpgradeManager()
{

}

bool
UpgradeManager::addChecker(UpgradeChecker* checker)
{
    int i;

    if (checker == NULL)
        return false;

    for (i = 0; i < m_checkerArray.size(); i++) {
        if (m_checkerArray[i] == checker)
            return true;
    }
    m_checkerArray.push_back(checker);
    return true;
}

bool
UpgradeManager::removeChecker(UpgradeChecker* checker)
{
    std::vector<UpgradeChecker*>::iterator it;

    for(it = m_checkerArray.begin(); it != m_checkerArray.end(); ++it) {
        if(checker == *it) {
            m_checkerArray.erase(it);
            return true;
        }
    }
    return false;
}

UpgradeChecker*
UpgradeManager::findChecker(int type)
{
    std::vector<UpgradeChecker*>::iterator it;

    for(it = m_checkerArray.begin(); it != m_checkerArray.end(); ++it) {
        if (*it != NULL) {
            if (type == (*it)->type())
                return *it;
        }
    }

    return 0;
}
bool
UpgradeManager::startCheck(UpgradeChecker* checker)
{
    std::vector<UpgradeChecker*>::iterator it;

    UpgradeLogDebug("Upgrade check start\n");
    for(it = m_checkerArray.begin(); it != m_checkerArray.end(); ++it) {
        if (checker == *it) {
            (*it)->start();
            return true;
        }
    }
    return false;
}

bool
UpgradeManager::stopCheck()
{
    std::vector<UpgradeChecker*>::iterator it;

    for(it = m_checkerArray.begin(); it != m_checkerArray.end(); ++it)
        (*it)->stop();

    return false;
}


bool
UpgradeManager::startCheckTimer()
{
    int interval = 0;
    appSettingGetInt("upcheckInterval", &interval, 0);
    removeMessages(UMMC_AUTO_CHECK_TIMER);
    if (interval >= 1) {
        sendEmptyMessageDelayed(UMMC_AUTO_CHECK_TIMER, interval * 60 * 60 * 1000);
    } else {
        sendEmptyMessageDelayed(UMMC_AUTO_CHECK_TIMER, 5 * 60 * 60 * 1000);
    }
    return true;
}

bool
UpgradeManager::stopCheckTimer()
{
    removeMessages(UMMC_AUTO_CHECK_TIMER);
    return true;
}

static void setUpgradeParam(int upgradeType, bool isTr069Contral)
{
    char upgradeValue[5] = {0};
    sprintf(upgradeValue, "%d", upgradeType);
    upgradeManager()->writeUpgradeData("upgradeType", upgradeValue);
    memset(upgradeValue, 0, sizeof(upgradeValue));
    sprintf(upgradeValue, "%d", isTr069Contral);
    upgradeManager()->writeUpgradeData("upgradeTr069", upgradeValue);
    memset(upgradeValue, 0, sizeof(upgradeValue));
    sprintf(upgradeValue, "%d", session().getPlatform());
    upgradeManager()->writeUpgradeData("upgradeUrlflag", upgradeValue);
    upgradeManager()->saveUpgradeData();

}

bool
UpgradeManager::startMonitorORUdiskUpgrade(UpgradeSource* source)
{
    if (source && source->Type() == UMUT_UDISK_SOFTWARE && source->m_currentContent == 0) {
         if (!compareVersion(source))
            return false;
#ifdef INCLUDE_LITTLESYSTEM
         if (m_systemType == UMST_NORMAL) {
            setUpgradeParam(UpgradeManager::UDISK_UPGRADE, m_isTr069Upgrade);
            exit(0);
         }
#endif
    }

    Message *msg = obtainMessage(UMMC_UPGRADE_REQUEST, source);

    UpgradeLogDebug("startUpgrade\n");
    Synchronized tSync(&g_result);
    sendMessage(msg);
    tSync.wait(&g_result);

    return true;
}

void
UpgradeManager::onUpgradeRequest(UpgradeSource* source)
{
    if(m_state > UMS_CHECK) {
        g_result = false;
        Synchronized::notify(&g_result);
        return;
    }
    if(m_state == UMS_CHECK) {
        stopCheck();
    }

    m_source = source;
    g_result = startReceive(m_source);
    if(g_result)
        m_state = UMS_RECEIVE;

    Synchronized::notify(&g_result);
}


bool
UpgradeManager::startReceive(UpgradeSource* source)
{
    if (m_state > UMS_RECEIVE)
        return false;

    if (m_receiver) {
        delete m_receiver;
        m_receiver = 0;
    }

    m_receiver = new UpgradeReceiver(this, source);

    if (source->Type() == UMUT_UDISK_SOFTWARE)
        sendUpgradeMessage(UMIT_INFO, USB_UPGRADE_OK, 0, 0);
    else
        sendUpgradeMessage(UMIT_INFO, MV_System_OpenUpgradePage, 0, 0);

    if (!m_isTr069Upgrade
        && source->Type() == UMUT_IP_SOFTWARE
        && source->currentContent() == UMUC_SOFTWARE) {
        sysSettingSetInt("upgradeNewVersion", source->m_version);
        sendUpgradeEvent(UMUE_UPGRADE_START);
    }

    UpgradeLogDebug("startReceive\n");
    return m_receiver->start();


}

void
UpgradeManager::burnerEnded()
{
    if (m_source->successedContent(UMUC_SOFTWARE)
        || m_source->successedContent(UMUC_LOGO)
        || m_source->successedContent(UMUC_SETTING)) {
        int forceFlag = 0;
        sysSettingGetInt("upgradeForce", &forceFlag, 0);
        if (forceFlag)
            sysSettingSetInt("upgradeForce", 0);

        if (m_source->successedContent(UMUC_LOGO))
            upgrade_version_write(LOGO_VERSION, m_source->m_logoVersion);
        if (m_source->successedContent(UMUC_SETTING))
            upgrade_version_write(SETTING_VERSION, m_source->m_settingVersion);
        if (m_source->successedContent(UMUC_SETTING)
            && (m_source->Type() == UMUT_IP_TEMPLATE || m_source->Type() == UMUT_DVB_TEMPLATE))
            upgrade_version_write(TEMPLATE_VERSION, m_templateVersion);

        sendUpgradeMessage(UMIT_INFO, UMMI_UPGRADE_VERSION_SUCCEED, 0, 0);
        removeFile(CONFIG_FILE_DIR"/.digest");

        if (!m_isTr069Upgrade
            && m_source->Type() == UMUT_IP_SOFTWARE
            && m_source->successedContent(UMUC_SOFTWARE)) {
            sysSettingSetInt("IPUpgradeOK", 1);
            sendUpgradeEvent(UMUE_UPGRADE_FINISH);
        }

#ifdef TRANSFORM_WIFI_CONFIG
        app_transWifiConfig_toOld();
#endif

        UpgradeSuccessed(m_source->m_version);

#ifdef SETTING_TRANSFORM
        settingManagerSave();
#if defined(Jiangsu) && !defined(EC1308H)
        UpgradeLogDebug("jiangSu: copy 56 to 2.0, new version:%d\n", m_source->m_version);
        if((1 == m_source->m_version || 2604 == m_source->m_version) && m_source->successedContent(UMUC_SOFTWARE))
#endif
        {
            settingTransformBak();
        }
#endif
    }
    onDestroy();

}

static int buildEvent(std::string pJsonType, std::string channel, std::string templateID, int version, std::string download)
{
    json_object* jsonInfo = NULL;
    jsonInfo = json_object_create_object();
    if (!jsonInfo) {
        UpgradeLogError("json info is NULL\n");
        return -1;
    }
    json_object_object_add(jsonInfo, "type", json_object_new_string(pJsonType.c_str()));
    if (pJsonType == "EVENT_NEW_VERSION") {
        json_object_object_add(jsonInfo, "path", json_object_new_string(channel.c_str()));
        json_object_object_add(jsonInfo, "downloaded", json_object_new_string(download.c_str()));
    } else {
        json_object_object_add(jsonInfo, "template_id", json_object_new_string(templateID.c_str()));
        json_object_object_add(jsonInfo, "template_version", json_object_new_int(version));
        if (channel == "IP")
            json_object_object_add(jsonInfo, "template_source", json_object_new_string("01"));
        else
            json_object_object_add(jsonInfo, "template_source", json_object_new_string("02"));
    }
    browserEventSend(json_object_to_json_string(jsonInfo), NULL);
    json_object_delete(jsonInfo);
    return 0;
}

void
UpgradeManager::sendUpgradeMessage(int type, int arg1, int arg2, unsigned int pDelayMillis)
{
    if (type == UMIT_PROMPT) {
        switch (arg1) {
        case UMUT_IP_SOFTWARE: {
#ifdef HUAWEI_C20
            buildEvent("EVENT_NEW_VERSION", "IP", "01", arg2, "0");
            sendEmptyMessageDelayed(UMMC_AUTO_UPGRADE_TIMER, 15000);
#else
            if (m_isLocalCheckUpgrade) {
                m_isLocalCheckUpgrade = false;
                buildEvent("EVENT_NEW_VERSION", "IP", "01", arg2, "0");
            } else {
                Message *msg = defNativeHandler().obtainMessage(MessageType_Upgrade, MV_System_Propmt, 0);
                defNativeHandler().sendMessage(msg);
            }
#endif
            break;
        }
        case UMUT_DVB_SOFTWARE:
            buildEvent("EVENT_NEW_VERSION", "DVB", "01", arg2, "0");
            break;
        case UMUT_IP_TEMPLATE:
            buildEvent("EVENT_EPGTEMPLATE_NEW_VERSION", "IP", "01", arg2, "0");
            break;
        case UMUT_DVB_TEMPLATE:
            buildEvent("EVENT_EPGTEMPLATE_NEW_VERSION", "DVB", "01", arg2, "0");
            break;
        default:
            break;
        }
        return;
    }

#ifdef INCLUDE_TR069
    if (type == UMIT_INFO) {
        std::map<int, int>::iterator it = errMessageInfo.find(arg1);
        if(it != errMessageInfo.end() && m_isTr069Upgrade) {
            if (it->second != 1)
                app_report_upgrade_alarm(it->second);
            tr069_api_setValue((char*)"Event.Upgraded", NULL, it->second);
            TR069_API_SETVALUE((char*)"Task.Active", "V1_CHOISE_APP", 1);
        }
    }
#endif

    if (m_systemType == UMST_MINI) {
        if (mUpgradeLayOut) {
            if (type == UMIT_INFO) {
                if (arg1 == MV_System_OpenUpgradePage ) {
                    arg1 = 0;
                    UpgradeGraphicsInit();
                } else if (arg1 == USB_UPGRADE_OK) {
                    arg1 = UpgradeManager::UMMI_UPGRADE_UDISK_VERSION;
                    UpgradeGraphicsInit();
                }
                mUpgradeLayOut->setUpgradeState(arg1);
            }
            if (type == UMIT_PROGRESS)
                mUpgradeLayOut->setProgress(arg1);
        }
    } else {
        int progress = 0;
        if (type == UMIT_PROGRESS) {
            UpgradeLogDebug("progress = %d\n", arg1);
            if (m_state == UMS_RECEIVE && (!m_source || m_source->Type() == UMUT_UDISK_SOFTWARE))
                arg1 = 0;
            if (m_state == UMS_BURN)
                m_percent = arg1;
            progress = UMMI_UPGRADE_PROGRESS_START + arg1;
        } else
            progress = arg1;

        Message *msg = defNativeHandler().obtainMessage(MessageType_Upgrade, progress, arg2);
        if(pDelayMillis > 0)
            defNativeHandler().sendMessageDelayed(msg, pDelayMillis);
        else
            defNativeHandler().sendMessage(msg);
    }

    return;
}


void
UpgradeManager::reportEvent(UpgradeChecker* checker)
{
    if (!checker && m_state >= UMS_RECEIVE)
        return;

    UpgradeSource* source = checker->upgradeSource();

    if (!compareVersion(source)) {
        /* If receiving or burning now, don't send error message for check failed */
        if (m_state <= UMS_CHECK) {
            sendUpgradeMessage(UMIT_INFO, UMMI_UPGRADE_VERSION_FAILED, source->m_version, 0);
            checker->setState(UpgradeChecker::UCS_FINISH);
            onDestroy();
        }
        return;
    }
    if (m_isLocalCheckUpgrade)
        source->m_prePrompt = true;

    if (source->m_prePrompt) {
        sendUpgradeMessage(UMIT_PROMPT, source->Type(), source->m_version, 0);
        checker->setState(UpgradeChecker::UCS_SELECTED);
        m_state = UMS_WAIT_RECEIVE;
        return;
    }
#ifdef INCLUDE_LITTLESYSTEM
    if (m_systemType == UMST_NORMAL) {
        setUpgradeParam(source->Type(), m_isTr069Upgrade);
        exit(0);
    }
#endif
    m_source = source;
    if (startReceive(source))
        m_state = UMS_RECEIVE;
    checker->setState(UpgradeChecker::UCS_FINISH);

    return;
}

void
UpgradeManager::responseEvent(int upgradeType, bool isUpgrade)
{
    std::vector<UpgradeChecker*>::iterator it;

    removeMessages(UMMC_AUTO_UPGRADE_TIMER);
    for(it = m_checkerArray.begin(); it != m_checkerArray.end(); ++it) {
        UpgradeChecker *checker = *it;
        if (checker->type() == upgradeType) {
            checker->setState(UpgradeChecker::UCS_FINISH);
            m_source = checker->upgradeSource();
            if (isUpgrade) {
#ifdef INCLUDE_LITTLESYSTEM
                if (m_systemType == UMST_NORMAL) {
                    setUpgradeParam(m_source->Type(), m_isTr069Upgrade);
                    exit(0);
                }
#endif
                startReceive(m_source);
                m_state = UMS_RECEIVE;
            } else
                onDestroy();
            break;
        }
    }
}

bool
UpgradeManager::compareVersion(UpgradeSource* source)
{
    if (!source || (source->m_version <= 0 && source->m_logoVersion <= 0 && source->m_settingVersion <= 0))
        return false;

    if (source->m_ignoreVersion) {
        if (source->Type() == UMUT_MONITOR_SOFTWARE) {
            if (source->m_version > 0)
                source->setSourceAddress(UMUC_SOFTWARE);
            else if (source->m_logoVersion > 0)
                source->setSourceAddress(UMUC_LOGO);
            else
                source->setSourceAddress(UMUC_SETTING);
        } else {
            if (!source->m_softwareSourceAddr.empty())
                source->setSourceAddress(UMUC_SOFTWARE);
            else if(!source->m_logoSourceAddr.empty())
                source->setSourceAddress(UMUC_LOGO);
            else
                source->setSourceAddress(UMUC_SETTING);
        }
        return true;
    }
    UpgradeLogDebug("softwareVersion = %d, logoVersion = %d, settingVersion = %d\n", source->m_version, source->m_logoVersion, source->m_settingVersion);
    int upgradeMode = 0;
    sysSettingGetInt("upgradeMode", &upgradeMode, 0);
    if (upgradeMode == MIX_UPGRADE_MODE
        && (source->Type() == UMUT_IP_SOFTWARE || source->Type() == UMUT_DVB_SOFTWARE)) {
        if (source->m_version > m_softwareVersion)
            return true;
        else
            return false;
    }

    if (source->Type() == UMUT_IP_TEMPLATE || source->Type() == UMUT_DVB_TEMPLATE) {
         if (source->m_version > m_templateVersion)
            return true;
         else
            return false;
    }

    if (source->m_version <= 0 || source->m_version == m_softwareVersion) {
        if ((source->m_logoVersion <= 0) || (source->m_logoVersion == m_logoVersion)) {
            if (source->m_settingVersion > 0 && source->m_settingVersion != m_settingVersion) {
                source->setSourceAddress(UMUC_SETTING);
                return true;
            } else
                return false;
        } else {
            source->setSourceAddress(UMUC_LOGO);
            return true;
        }
    } else {
        source->setSourceAddress(UMUC_SOFTWARE);
        return true;
    }

}

void
UpgradeManager::onDestroy()
{
    UpgradeLogDebug("checkerArray size(%d)\n", m_checkerArray.size());

    sendUpgradeMessage(UMIT_INFO, UMMI_UPGRADE_END, 0, 500);

    if(m_burner) {
        delete m_burner;
        m_burner = 0;
    }

    if(m_receiver) {
        delete m_receiver;
        m_receiver = 0;
    }

    std::vector<UpgradeChecker*>::iterator it;
    UpgradeLogDebug("checkerArray size(%d)\n", m_checkerArray.size());
    for(it = m_checkerArray.end() - 1; it == m_checkerArray.begin(); it--) {
        UpgradeChecker* checker = *it;
        if(checker && checker->state() == UpgradeChecker::UCS_FINISH) {
            checker->stop();
            removeChecker(checker);
            delete checker;
        }
    }

    m_state = UMS_IDLE;
    m_source = 0;
    m_isLocalCheckUpgrade = false;
    m_isTr069Upgrade = false;
    m_provider = 0;
    return;

}

void
UpgradeManager::onCheckEnd()
{
    UpgradeLogDebug("version check end\n");
    std::vector<UpgradeChecker*>::iterator it;
    UpgradeChecker* selected = 0;
    int upgradeMode = 0;

    sysSettingGetInt("upgradeMode", &upgradeMode, 0);
    if (upgradeMode == MIX_UPGRADE_MODE) {
        ; /* Wait for IP and DVB all check end, then select bigger version from all upgrade mode */
    } else {
        /* Maybe software and template upgrade check at one time */
        for (it = m_checkerArray.begin(); it != m_checkerArray.end(); ++it) {
            UpgradeChecker* checker = *it;
            if (checker->state() == UpgradeChecker::UCS_OK || checker->state() == UpgradeChecker::UCS_ERROR) {
                if (m_state >= UMS_RECEIVE)
                    checker->setState(UpgradeChecker::UCS_FINISH);
                else
                    selected = checker;
                break;
            }
        }
        if (!selected)
            return;

        if(selected->state() == UpgradeChecker::UCS_ERROR) {
            selected->setState(UpgradeChecker::UCS_FINISH);
            if (m_state <= UMS_CHECK) {
                sendUpgradeMessage(UMIT_INFO, selected->errorCode(), 0, 0);
                onDestroy();
            }
            return;
        }
    }

    reportEvent(selected);

}

void
UpgradeManager::onReceiveEnd()
{
    UpgradeLogDebug("m_state = %d\n", m_state);

    if (m_state != UMS_RECEIVE || m_receiver->state() != UpgradeReceiver::URS_OK || !m_source) {
        m_percent = -1;
        sendUpgradeMessage(UMIT_INFO, UMMI_GET_VERSION_FAILED, 0, 0);
        burnerEnded();
        return;
    }

    if (m_source->m_force && !compareVersion(m_source)) {
        m_percent = -1;
        sendUpgradeMessage(UMIT_INFO, UMMI_UPGRADE_SAME_VERSION, 0, 0);
        sendUpgradeMessage(UMIT_INFO, UMMI_UPGRADE_VERSION_FAILED, 0, 0);
        onDestroy();
        return;
    }

    UpgradeData* data = m_receiver->getUpgradeData();

    if (m_burner)
        delete m_burner;

    if (m_source->Type() == UMUT_IP_TEMPLATE || m_source->Type() == UMUT_DVB_TEMPLATE)
        m_burner = new UpgradeTemplateBurner(this, data);
    else
        m_burner = new UpgradeBurner(this, data);

    UpgradeLogDebug("burn start\n");
    m_state = UMS_BURN;
    m_burner->start();

    int delay = 0;
    if (m_source->currentContent() == UMUC_LOGO)
        delay = 2000;
    else if (m_source->currentContent() == UMUC_SETTING)
        delay = 1000;
    else
        delay = 0;

    sendUpgradeMessage(UMIT_INFO, UMMI_UPGRADE_VERSION_START, 0, delay);

}

void
UpgradeManager::onReceiveLogoStart()
{
    if (!m_source) {
        sendUpgradeMessage(UMIT_INFO, UMMI_GET_VERSION_FAILED, 0 ,0);
        burnerEnded();
        return;
    }

    if (m_source->m_logoVersion > 0
        && (m_source->m_ignoreVersion || m_source->m_logoVersion != m_logoVersion)) {
        m_source->setSourceAddress(UMUC_LOGO);
        startReceive(m_source);
    } else
        sendEmptyMessage(UMMC_SETTING_RECEIVE_START);
}

void
UpgradeManager::onReceiverSettingStart()
{
    if (!m_source) {
        sendUpgradeMessage(UMIT_INFO, UMMI_GET_VERSION_FAILED, 0, 0);
        burnerEnded();
        return;
    }

    if (m_source->m_settingVersion > 0
        && (m_source->m_ignoreVersion || m_source->m_settingVersion != m_settingVersion)) {
        m_source->setSourceAddress(UMUC_SETTING);
        startReceive(m_source);
    } else
        burnerEnded();
}


void
UpgradeManager::onBurnEnd()
{
    if (m_state != UMS_BURN) {
        sendUpgradeMessage(UMIT_INFO, UMMI_UPGRADE_VERSION_FAILED, 0, 0);
        burnerEnded();
        return;
    }

    if (m_burner->state() != UpgradeBurner::UBS_OK) {
        sendUpgradeMessage(UMIT_INFO, m_burner->errorCode(), 0, 0);
        burnerEnded();
        return;
    }

    m_source->setSuccessedContent();
    if (m_source->Type() == UMUT_IP_SOFTWARE
        || m_source->Type() == UMUT_UDISK_SOFTWARE) {
        switch(m_source->currentContent()) {
        case UMUC_SOFTWARE:
            sendEmptyMessage(UMMC_LOGO_RECEIVE_START);
            break;
        case UMUC_LOGO:
            sendEmptyMessage(UMMC_SETTING_RECEIVE_START);
            break;
        case UMUC_SETTING:
            burnerEnded();
            break;
        default:
            burnerEnded();
            break;
        }
    } else
        burnerEnded();

    return;

}

void UpgradeManager::onCheckChannellist()
{
    static int checkCount = 0;
    if (ProgramParser::m_parseChannellistOK) {
        sendUpgradeEvent(UMUE_UPGRADE_OK);
        sysSettingSetInt("IPUpgradeOK", 0);
    } else {
        checkCount++;
        if (checkCount >= 60) { // send upgrade failed after 5 mins can't connect EPG
            sendUpgradeEvent(UMUE_UPGRADE_FAIL);
            sysSettingSetInt("IPUpgradeOK", 0);
        } else
            sendEmptyMessageDelayed(UMMC_CHANNELLIST_CHECK, 5000);
    }
}

static void constructURL(const int event, char* ip, char* upgradeEventUrl)
{
    char url[URL_MAX_LEN + 4] = {0};
    char tIp[64 + 4] = {0};
    char stbID[34] = {0};
    char mac[20] = {0};
    char ntvUser[USER_LEN] = {0};
    char softwareVersion[32] = {0};
    char hardwareVersion[32] = {0};
    int newVersion = 0;
    int isBakAddr = 0;
    int port = 0;

    mid_sys_serial(stbID);
    network_tokenmac_get(mac, 20, ':');
    appSettingGetString("ntvuser", ntvUser, USER_LEN, 0);
    get_upgrade_version(softwareVersion);
    HardwareVersion(hardwareVersion, 32);
    sysSettingGetInt("upgradeNewVersion", &newVersion, 0);

    sysSettingGetInt("isUseBakAddr", &isBakAddr, 0);
    if (isBakAddr)
        sysSettingGetString("upgradeBackupUrl", url, URL_MAX_LEN + 4, 0);
    else
        sysSettingGetString("upgradeUrl", url, URL_MAX_LEN + 4, 0);

    if (memcmp(url, "http://", 7) || mid_tool_checkURL(url, tIp, &port)) {
        UpgradeLogDebug("Invalid upgrade address: %s\n", url);
        return;
    }

    sprintf(upgradeEventUrl, "http://%s:%d/EDS/jsp/upgradeEvent.jsp?TYPE=%s&STBID=%s&MAC=%s&USER=%s&VER=%d&SoftwareVersion=%s&SoftwareHWVersion=%s&HardwareVersion=%s&NEWVER=%d&EVENT=%d",
              ip, port, StbInfo::STB::UpgradeModel(), stbID, mac, ntvUser, upgrade_version_read(SOFTWARE_VERSION), softwareVersion, StbInfo::STB::Version::HWVersion(), hardwareVersion, newVersion, event);

    return;
}

static void upgradeEventResponse(int type, char* buf, int len, int arg)
{
    UpgradeLogDebug("Receive upgrade event response: \n%s\n", buf);
}

static void dnsCallBack(int arg, int dnsmsg, unsigned int hostip)
{
    char tIp[64 + 4] = {0};
    char upgradeEventUrl[URL_MAX_LEN + 4] = {0};

    if(dnsmsg) {
        UpgradeLogDebug("DNS resovle failed!\n");
        return;
    }

    mid_tool_addr2string(hostip, tIp);
    constructURL(arg, tIp, upgradeEventUrl);
    mid_http_call(upgradeEventUrl, (mid_http_f)upgradeEventResponse, 0, NULL, 0, global_cookies);
}

void
UpgradeManager::sendUpgradeEvent(const int event)
{
    char upgradeEventUrl[URL_MAX_LEN + 4] = {0};
    char url[URL_MAX_LEN + 4] = {0};
    char tIp[64 + 4] = {0};
    int port = 0;
    unsigned int ipaddr = 0;
    int isBakAddr = 0;

    sysSettingGetInt("isUseBakAddr", &isBakAddr, 0);
    if (isBakAddr)
        sysSettingGetString("upgradeBackupUrl", url, URL_MAX_LEN + 4, 0);
    else
        sysSettingGetString("upgradeUrl", url, URL_MAX_LEN + 4, 0);
    if (memcmp(url, "http://", 7) || mid_tool_checkURL(url, tIp, &port)) {
        UpgradeLogDebug("Invalid upgrade address: %s\n", url);
        return;
    }
    ipaddr = inet_addr(tIp);
    if (ipaddr == INADDR_ANY || ipaddr == INADDR_NONE) {
        mid_dns_resolve(tIp, (mid_dns_f)dnsCallBack, event, 28);
        return;
    }
    constructURL(event, tIp, upgradeEventUrl);
    mid_http_call(upgradeEventUrl, (mid_http_f)upgradeEventResponse, 0, NULL, 0, global_cookies);
}

void
UpgradeManager::handleMessage(Message* msg)
{
    switch(msg->what) {
    case UMMC_CHECK_START:
        break;
    case UMMC_CHECK_END:
        onCheckEnd();
        break;
    case UMMC_RECEIVE_END:
        onReceiveEnd();
        break;
    case UMMC_BURN_END:
        onBurnEnd();
        break;
    case UMMC_LOGO_RECEIVE_START:
        onReceiveLogoStart();
        break;
    case UMMC_SETTING_RECEIVE_START:
        onReceiverSettingStart();
        break;
    case UMMC_UPGRADE_REQUEST:
        onUpgradeRequest((UpgradeSource*)msg->obj);
        break;
    case UMMC_AUTO_CHECK_TIMER:
        touchOffUpgradeCheck(UMUT_IP_SOFTWARE, false);
        break;
    case UMMC_AUTO_UPGRADE_TIMER:
        if (m_state < UMS_RECEIVE && m_source)
            responseEvent(m_source->Type(), true);
        break;
    case UMMC_CHANNELLIST_CHECK:
        onCheckChannellist();
        break;
    default:
        break;
    }
}

bool
UpgradeManager::touchOffUpgradeCheck(int channel, bool isTr069Upgrade)
{
    if(m_state > UMS_CHECK)
        return false;

    if(findChecker(channel))
        return false;

    UpgradeLogDebug("Start by (%d)channel\n", channel);

    UpgradeChecker* upgradeChecker = NULL;
    m_isTr069Upgrade = isTr069Upgrade;
    switch (channel) {
    case UMUT_IP_SOFTWARE:
        upgradeChecker = new UpgradeIPChecker(this);
        break;
    case UMUT_DVB_SOFTWARE:
        break;
    case UMUT_IP_TEMPLATE:
        upgradeChecker = new TemplateIPUpgradeChecker(this);
        break;
    case UMUT_DVB_TEMPLATE:
        break;
    default:
        UpgradeLogDebug("param channel is unvalid!\n");
        return false;
    }

    addChecker(upgradeChecker);
    startCheck(upgradeChecker);

#ifdef HUAWEI_C20
    if (channel == UMUT_IP_SOFTWARE)
        startCheckTimer();
#endif
    if (m_state == UMS_IDLE)
        m_state = UMS_CHECK;

    return true;
}

void
UpgradeManager::UpgradeGraphicsInit()
{
    if (mUpgradeLayOut) {
        StandardScreen *layer = (StandardScreen *)Hippo::systemManager().mixer().topLayer();
        layer->attachChildToFront(mUpgradeLayOut);
        mUpgradeLayOut->setVisibleP(true);
	}
}

bool
UpgradeManager::setUpgradeState(int pState)
{
    if (mUpgradeLayOut) {
        mUpgradeLayOut->setUpgradeState(pState);
        return true;
    }

    return false;
}

int
UpgradeManager::getUpgradeState()
{
    if (mUpgradeLayOut)
        return mUpgradeLayOut->getUpgradeState();

    return 0;
}

bool
UpgradeManager::readUpgradeData()
{
    std::string valueStr;
    std::string keyStr;
    std::string str;
    int keyPos = 0;
    int valuePos = 0;
    char* ltok = NULL;
    char lBuff[4097] = "";

    FILE* infile = fopen("/var/startshell", "rb");
    if (!infile)
        return false;
    while (fgets(lBuff, 4096, infile)) {
        ltok = strrchr(lBuff, '\n');
        if (ltok)
            *ltok = 0;
        str.assign(lBuff);
        keyPos = str.find(',', 0);
        if(keyPos == -1)
            continue;

        keyStr = str.substr(0, keyPos);
        if(keyStr.compare(0, 4, "key="))
            continue;
        else
            keyStr = str.substr(4, keyPos - 4);

        valueStr = str.substr(keyPos + 1, str.length());
        if(valueStr.compare(0, 6, "value="))
            continue;
        else
            valueStr = str.substr(keyPos + 7, str.length());
        mStartInfo.insert(std::make_pair((keyStr), valueStr));
    }

    fclose(infile);

    return true;
}

void
UpgradeManager::writeUpgradeData(const char *key, const char *value)
{
    mStartInfo.insert(std::make_pair((key), value));
}

bool
UpgradeManager::saveUpgradeData()
{
    std::string lStr("");
    FILE* outfile = fopen("/var/startshell", "wb"); if (!outfile)
        return -1;
    for(std::map<std::string, std::string>::iterator it = mStartInfo.begin(); it != mStartInfo.end(); it++) {
        lStr = "key=" + it->first + ",value=" + it->second + "\n";
        fputs(lStr.c_str(), outfile);
    }
    fclose(outfile);

    return true;
}

bool
UpgradeManager::getUpgradeData(const char *key, std::string& value)
{
    std::map<std::string, std::string>::iterator it = mStartInfo.find(key);
    if (it != mStartInfo.end())
        value = it->second;
    else
        return false;
    return true;
}


static UpgradeManager* g_upgradeManager = 0;

UpgradeManager *upgradeManager()
{
    return g_upgradeManager;
}

}

extern "C"
void upgradeManagerCreate(int systemType)
{
    Hippo::g_upgradeManager = new Hippo::UpgradeManager(systemType);

    return;

}

