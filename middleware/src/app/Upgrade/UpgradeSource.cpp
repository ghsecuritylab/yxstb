
#include "UpgradeAssertions.h"
#include "UpgradeSource.h"
#include "UpgradeManager.h"
#include "DataSource.h"

namespace Hippo {

UpgradeSource::UpgradeSource()
	: m_version(-1)
	, m_logoVersion(-1)
	, m_settingVersion(-1)    
	, m_force(false)
	, m_prePrompt(false)
	, m_postPrompt(false)
	, m_ignoreVersion(false)
	, m_dataSource(NULL)
	, m_bufferType(USBT_Unknow)
	, m_currentContent(0)
    , m_successedConent(0)
{
    m_logomd5.clear();
}

UpgradeSource::~UpgradeSource()
{
}

TempFile* 
UpgradeSource::getTempFile()
{
    if (m_bufferType == USBT_TempFile)
        return m_buffer.m_file;
    else
        return 0;
}

TempBuffer* 
UpgradeSource::getTempBuffer()
{
    if (m_bufferType == USBT_TempBuffer)
        return m_buffer.m_tempBuffer;
    else
        return 0;
}

RingBuffer* 
UpgradeSource::getRingBuffer()
{
    if (m_bufferType >= USBT_RingBuffer)
        return m_buffer.m_ringBuffer;
    else
        return 0;
}

bool
UpgradeSource::setSourceAddress(int upgradeContent)
{
    std::string sourceAddress;
        
    if (!m_dataSource)
        return false;
    
    m_currentContent = upgradeContent;
        
    switch (upgradeContent) {
    case UpgradeManager::UMUC_SOFTWARE:
        sourceAddress = m_softwareSourceAddr;
        break;
    case UpgradeManager::UMUC_LOGO:
        sourceAddress = m_logoSourceAddr;
        break;
    case UpgradeManager::UMUC_SETTING:
        sourceAddress = m_settingSourceAddr;
        break;
    default:
        sourceAddress = m_softwareSourceAddr;
        break;   
    }
    
    m_dataSource->setDataSourceAddress(sourceAddress.c_str());
    
    return true; 
    
}

void
UpgradeSource::setSuccessedContent()
{
    m_successedConent |= m_currentContent;
    
}

} // namespace Hippo
