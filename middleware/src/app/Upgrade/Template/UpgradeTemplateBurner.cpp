#include "UpgradeTemplateBurner.h"
#include "UpgradeAssertions.h"
#include "TempFile.h"
#include "UpgradeData.h"
#include "UpgradeManager.h"

#include "config/pathConfig.h"

#ifdef VERIFY_UPGRADE_FILE
#include "scbt_global.h"
#endif

#include <string.h>

#define TEMPLATEPATH DEFAULT_TEMPLATE_DATAPTH"/template.tar.gz"

namespace Hippo {

static void* burnThreadEntry(void* arg);

UpgradeTemplateBurner::UpgradeTemplateBurner(UpgradeManager* manager, UpgradeData* data)
	: UpgradeBurner(manager, data)
{
}

UpgradeTemplateBurner::~UpgradeTemplateBurner()
{
}

bool
UpgradeTemplateBurner::start()
{
    if (!pthread_create(&m_thread, NULL, burnThreadEntry, this))
        return true;
    else
        return false;
}

bool
UpgradeTemplateBurner::stop()
{
    return false;
}

static UpgradeBurner* g_templateBurner = 0;

static void* burnThreadEntry(void* arg)
{
    int fileLength = 0;
    int retLength = 0;
    int preProgress = 0;
    int size = 0;
    int ret = 0;
    char buf[1024] = {0};

    g_templateBurner = (UpgradeTemplateBurner*)arg;
    UpgradeLogDebug("start copy template data\n");
    UpgradeData* downLoadData = g_templateBurner->upgradeData();
    if (!downLoadData)
    	return NULL;

    /*aes_decrypt*/
#if defined(VERIFY_UPGRADE_FILE)
    ret = scbt_api_decrypt_without_upgrade_header_file((char*)downLoadData->tempFile()->filePath());
    UpgradeLogDebug("ret = %d\n", ret);
    if (ret != 0 ) {
    	g_templateBurner->setState(UpgradeBurner::UBS_ERROR);
        g_templateBurner->setErrorCode(UpgradeManager::UMMI_UPGRADE_TEMPLATE_FAILED);
        g_templateBurner->updateEnd();
        return NULL;
    }
    /*rsa_verif*/
    ret = scbt_api_verify_only_without_header_file((char*)downLoadData->tempFile()->filePath());
    UpgradeLogDebug("ret = %d\n", ret);
    if (ret != 1) {
    	g_templateBurner->setState(UpgradeBurner::UBS_ERROR);
        g_templateBurner->setErrorCode(UpgradeManager::UMMI_UPGRADE_TEMPLATE_FAILED);
        g_templateBurner->updateEnd();
        return NULL;
    }
#endif
    UpgradeData* templateData = new UpgradeData(TEMPLATEPATH);

    if (downLoadData->open('r') == -1 || templateData->open('w') == -1) {
    	g_templateBurner->setState(UpgradeBurner::UBS_ERROR);
        g_templateBurner->setErrorCode(UpgradeManager::UMMI_UPGRADE_TEMPLATE_FAILED);
        g_templateBurner->updateEnd();
        delete templateData;
        return NULL;
    }
    size = downLoadData->size();
    UpgradeLogDebug("size = %d\n", size);
    while (fileLength < size) {
        retLength = downLoadData->read(buf, 1024);
        if (!retLength) {
        	UpgradeLogDebug("read finish\n");
            break;
        }
        templateData->write(buf, retLength);
        fileLength += retLength;
        int progress = (fileLength /1000)  / (size / 100000);
        if (progress > preProgress + 3) {
            g_templateBurner->updateProgress(progress);
	        preProgress = progress;
	    }
    }
    downLoadData->close();
    downLoadData->unlink();
    templateData->close();
    g_templateBurner->setState(UpgradeBurner::UBS_OK);
    g_templateBurner->updateEnd();
    delete templateData;
    //delete downLoadData;
    return NULL;
}

}
