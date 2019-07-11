
#include "UpgradeAssertions.h"
#include "UpgradeIPSource.h"

#include "RingBuffer.h"

#include "SysSetting.h"

#include <stdlib.h>


#define BUFFER_SIZE	64*1024

namespace Hippo {

UpgradeIPSource::UpgradeIPSource()
{
    m_memory = malloc(BUFFER_SIZE);

    m_bufferType = USBT_RingBuffer;
    m_buffer.m_ringBuffer = new RingBuffer((uint8_t*)m_memory, BUFFER_SIZE);

    m_force = false;
	int upgradeForce = 0;
    sysSettingGetInt("upgradeForce", &upgradeForce, 0);
    if (upgradeForce)
        m_ignoreVersion = true;
    else 
        m_ignoreVersion = false;
	
    m_prePrompt = false;
    m_postPrompt = false;
}

UpgradeIPSource::~UpgradeIPSource()
{
    delete m_buffer.m_ringBuffer;
    free(m_memory);
}

} // namespace Hippo
