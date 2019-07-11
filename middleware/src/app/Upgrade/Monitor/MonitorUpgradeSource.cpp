
#include "MonitorAssertions.h"
#include "MonitorUpgradeSource.h"

#include "DataStream/RingBuffer.h"

#include <stdlib.h>


#define BUFFER_SIZE	64*1024

namespace Hippo {

MonitorUpgradeSource::MonitorUpgradeSource()
{
    m_force = true;
    m_prePrompt = true;
    m_postPrompt = false;

    m_memory = malloc(BUFFER_SIZE);

    m_bufferType = USBT_RingBuffer;
    m_buffer.m_ringBuffer = new RingBuffer((uint8_t*)m_memory, BUFFER_SIZE, true);
}

MonitorUpgradeSource::~MonitorUpgradeSource()
{
    delete m_buffer.m_ringBuffer;
    free(m_memory);
}

} // namespace Hippo
