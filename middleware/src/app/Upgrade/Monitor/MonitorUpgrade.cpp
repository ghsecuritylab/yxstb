
#include "MonitorUpgrade.h"
#include "MonitorAssertions.h"
#include "MonitorUpgradeSource.h"
#include "UpgradeManager.h"
#include "FileStreamDataSource.h"

static Hippo::MonitorUpgradeSource* g_upgradeSource = 0;
static Hippo::FileStreamDataSource g_dataSource;
static int g_upgradeFileLength = 0;
static int g_upgradeFlags = 0;

extern "C" 
void monitorUpgradeReset()
{
    g_upgradeFileLength = 0;
    g_upgradeFlags = 0;
}

extern "C" 
void monitorUpgradeSetLength(int length)
{
    g_upgradeFileLength = length;
}

extern "C" 
int monitorUpgradeGetLength()
{
    return g_upgradeFileLength;
}

extern "C" 
void monitorUpgradeSetFlag(int flag)
{
    g_upgradeFlags = flag;
}

extern "C" 
int monitorUpgradeGetFlag()
{
    return g_upgradeFlags;
}

extern "C" 
int monitorUpgradeDownloadStart()
{
    MONITOR_LOG("\n");

    if (!g_upgradeSource)
        g_upgradeSource = new Hippo::MonitorUpgradeSource();

    g_upgradeSource->m_version = -1; /* 此时版本号未知。*/
    g_upgradeSource->m_force = true;
    g_upgradeSource->m_prePrompt = false;
    g_upgradeSource->m_postPrompt = false;
    g_upgradeSource->m_ignoreVersion = g_upgradeFlags;
    g_upgradeSource->m_dataSource = &g_dataSource;
    g_dataSource.setBuffer(g_upgradeSource->getRingBuffer());

    if (g_upgradeFileLength > 15 * 1024 * 1024)
		g_upgradeSource->setSourceAddress(Hippo::UpgradeManager::UMUC_SOFTWARE);
    else if (g_upgradeFileLength > 100 * 1024)
		g_upgradeSource->setSourceAddress(Hippo::UpgradeManager::UMUC_LOGO);
    else
        g_upgradeSource->setSourceAddress(Hippo::UpgradeManager::UMUC_SETTING);

    Hippo::upgradeManager()->startMonitorORUdiskUpgrade(g_upgradeSource);

    g_dataSource.tellDataSize(g_upgradeFileLength);
	

    return 0;
}

extern "C" 
int monitorUpgradeGetBuffer(unsigned char** buffer, int* length)
{
    return g_dataSource.getWriteHead(buffer, (uint32_t*)length);
}

extern "C" 
int monitorUpgradeSubmitBuffer(unsigned char* buffer, int length)
{
    return g_dataSource.submitWrite(buffer, (uint32_t)length);
}

extern "C"
void monitorUpgradeReceiveError()
{
    g_dataSource.receiveError();
}

extern "C" 
int monitorUpgradeBurnStart()
{
    g_dataSource.stop();
    return -1;
}

extern "C"
int monitorUpgradeSetProgress(int precent)
{
    Hippo::upgradeManager()->sendUpgradeMessage(Hippo::UpgradeManager::UMIT_PROGRESS, precent, 0, 0);
    return 0;
}


extern "C" 
int monitorUpgradeGetProgress()
{
    return Hippo::upgradeManager()->upgradePercent();
}

extern "C"
int monitorUpgradeModeSet()
{
    char upgradeMode[5] = {0};
    sprintf(upgradeMode, "%d", Hippo::UpgradeManager::UMUT_MONITOR_SOFTWARE);
    Hippo::upgradeManager()->writeUpgradeData("upgradeType", upgradeMode);
    Hippo::upgradeManager()->saveUpgradeData();
    return 0;
}

extern "C"
int monitorSystemTypeGet()
{   
    return Hippo::upgradeManager()->upgradeSystemType();
}

