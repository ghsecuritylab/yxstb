
#include "UpgradeAssertions.h"
#include "UpgradeBurner.h"

#include "TempFile.h"
#include "UpgradeData.h"
#include "UpgradeManager.h"
#include "Hippo_Context.h"
#include "Hippo_HString.h"
#include "OSOpenSSL.h"
#include "UpgradeReceiver.h"
#include "UpgradeSource.h"
#include "TempFile.h"
#include <stdlib.h>
#include <unistd.h>  //for sleep
#include <string.h>
#include "AppSetting.h"
#include "SysSetting.h"
#include "UpgradeCommon.h"

#include "libzebra.h"
#include <openssl/md5.h>

#define SETTING_FILE_PATH  "/var/setting.ini"

namespace Hippo {

static void* burnThreadEntry(void* arg);
static void burnCallback(int len);


UpgradeBurner::UpgradeBurner(UpgradeManager* manager, UpgradeData* data)
	: m_manager(manager)
	, m_data(data)
	, m_thread(0)
	, m_dataSize(0)
    , m_state(UBS_ERROR)
    , m_errorCode(0)
{
}

UpgradeBurner::~UpgradeBurner()
{
}

bool
UpgradeBurner::start()
{

    if (!pthread_create(&m_thread, NULL, burnThreadEntry, this))
        return true;
    else
        return false;
}

bool
UpgradeBurner::stop()
{
    return false;
}

void
UpgradeBurner::updateProgress(int size)
{
    Message* msg = obtainMessage(MC_Progress, size, 0);
    sendMessage(msg);
}

void
UpgradeBurner::updateEnd()
{
    if (m_data) {
        delete m_data;
        m_data = 0;
    }
    removeMessages(MC_Timer);
    m_manager->sendEmptyMessage(UpgradeManager::UMMC_BURN_END);
}

void
UpgradeBurner::handleMessage(Message* msg)
{
    switch (msg->what) {
        case MC_Progress:
            m_manager->sendUpgradeMessage(UpgradeManager::UMIT_PROGRESS, msg->arg1, 0, 0);
            break;
       case MC_Timer:
            burnCallback(-1);
            break;
        default:
            break;
    }
}

static UpgradeBurner* g_burner = 0;
static int g_precent = -1;
static void burnCallback(int len)
{
    if (g_burner) {
        if (len == -1)
            g_precent++;

        if (len < g_precent) {
            g_burner->sendEmptyMessageDelayed(UpgradeBurner::MC_Timer, 2000);
            g_burner->updateProgress(g_precent);
        } else {
            g_burner->removeMessages(UpgradeBurner::MC_Timer);
            g_precent = -1;
            g_burner->updateProgress(len);
        }

    }
}

int
UpgradeBurner:: upgradeSettingFile(const char* filePath)
{
    FILE* fp = NULL;
    char *p = NULL;
    char buf [512] = {0};
    char para[32];
    HString fieldName;
    HString fileldValue;

    fp = fopen(filePath, "r");
    if (!fp) {
       UpgradeLogDebug("setting file is not exist\n");
        return -1;
    }
    burnCallback(0);
    sleep(1); /* In order to show 0%. */
    while (fgets(buf, 512, fp)) {
        p = strchr(buf, '=');
        if (!p)
            continue;
        if (p - buf > 32)
            continue;
        memset(para, 0, 32);
        strncpy(para, buf, p - buf);
        for(int i = 0; i < strlen(buf); i++) {
            if(buf[i] == '\r' || buf[i] == '\n' || buf[i] == '\0') {
                buf[i] = 0;
                break;
            }
        }
        fieldName = para;
        if (strlen(buf) - 1 > p -buf)
            fileldValue = p + 1;
	    else
            fileldValue = "";
        UpgradeLogDebug("fieldName = %s, fieldValue = %s", fieldName.c_str(), fileldValue.c_str());
        HippoContext::getContextInstance()->ioctlWrite(fieldName,fileldValue);
    }
    fclose(fp);
    settingManagerSave();
    burnCallback(100);
    return 0;
}

static void* burnThreadEntry(void* arg)
{
    int ret = 0;

    if (g_burner)
        return 0;

    g_burner = (UpgradeBurner*)arg;
    UpgradeData* data = g_burner->upgradeData();
    if (data->type() == UpgradeData::UDT_TempFile ) {
        TempFile* tempFile = data->tempFile();
        tempFile->open('r');
        int fileLen = tempFile->size();
        tempFile->close();

        UpgradeLogDebug("fileLen = %d\n", fileLen);
        UpgradeSource* source = g_burner->upgradeManager()->upgradeSource();
        if (!source || fileLen <= 0) {
            UpgradeLogError("source is null or fileLen  error\n");
            g_burner->setState(UpgradeBurner::UBS_ERROR);
            g_burner->setErrorCode(UpgradeManager::UMMI_UPGRADE_VERSION_FAILED);
            g_burner->updateEnd();
            g_burner = 0;
            return 0;
        }
         UpgradeLogDebug("source->m_currentContent = %d\n", source->m_currentContent);

        if (UpgradeManager::UMUC_SETTING == source->m_currentContent) {
            ret = g_burner->upgradeSettingFile(tempFile->filePath());
            if (ret) {
                g_burner->setErrorCode(UpgradeManager::UMMI_UPGRADE_VERSION_FAILED);
                g_burner->setState(UpgradeBurner::UBS_ERROR);
                g_burner = 0;
                return 0;
            }
            g_burner->setState(UpgradeBurner::UBS_OK);
            g_burner->updateEnd();
            return 0;
        }

        if (UpgradeManager::UMUC_LOGO == source->m_currentContent
            && strcmp(source->m_logomd5.c_str(),"no md5")) {
            TempFile* tempFile1 = data->tempFile();
            tempFile1->open('r');
            int fileLen = tempFile1->size();
            char* buf = (char *)malloc(fileLen+1);
            tempFile1->read(buf, fileLen);
            tempFile1->close();
            OpenSSL::MD5Crypto picMD5;
            std::string digest = picMD5.CreateDigest(buf, fileLen+1);
            if(digest.empty() || strcmp(source->m_logomd5.c_str(), digest.c_str())) {
                g_burner->setState(UpgradeBurner::UBS_ERROR);
                UpgradeLogDebug("LOGO MD5 ERROR\n");
                g_burner->setErrorCode(UpgradeManager::UMMI_UPGRADE_VERSION_FAILED);
                g_burner->updateEnd();
                g_burner = 0;
                free(buf);
                return 0;
            }
            free(buf);
        }

#if defined(hi3560e)
//对于标清升级前操作env导致升级完成后不能切换分区，
//盒子还会从老分区启动，现象是没有升级成功
#else
        yhw_env_writeString("SYSTEMUPGRADEOK", "n");
#endif
        sleep(1); ////防止logo升级太快, 升级进度上报早于状态转换为:UPDATE_NORMAL_SYS_START
        g_burner->sendEmptyMessageDelayed(UpgradeBurner::MC_Timer, 1000);
        ret = firmware_burn_ex((char*)tempFile->filePath(), 0, burnCallback);
        if (0 > ret) {
            g_burner->setState(UpgradeBurner::UBS_ERROR);
            UpgradeLogDebug("UPGRADE_VERSION_FAILED\n");
            burnCallback(-1);
            if ((-2 == ret) && (source->Type() == UpgradeManager::UMUT_UDISK_SOFTWARE))
                g_burner->setErrorCode(UpgradeManager::UMMI_UPGRADE_VERSION_WRONG);
            else
                g_burner->setErrorCode(UpgradeManager::UMMI_UPGRADE_VERSION_FAILED);

            sleep(1);
            g_burner->updateEnd();
            return 0;
        }
#if defined(hi3560e)
//对于标清升级前操作env导致升级完成后不能切换分区，
//盒子还会从老分区启动，现象是没有升级成功
        if (UpgradeManager::UMUC_LOGO == source->m_currentContent) //firmware_burn_ex,标清3560E, 除了app外,其余不走回调函数
            burnCallback(100);
#else
        yhw_env_writeString("SYSTEMUPGRADEOK", "y");
#endif
        g_burner->upgradeSettingFile(SETTING_FILE_PATH);
        g_burner->setState(UpgradeBurner::UBS_OK);
        g_burner->updateEnd();
    }
    g_burner = 0;
    return 0;
}

} /* namespace Hippo */
