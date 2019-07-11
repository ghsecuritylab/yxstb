
#include "UpgradeUdiskSource.h"
#include "RingBuffer.h"

#include <stdlib.h>

#define BUFFER_SIZE	64*1024

namespace Hippo {

UpgradeUdiskSource::UpgradeUdiskSource()
{
    m_force = false;
    m_prePrompt = false;
    m_postPrompt = false;

    m_memory = malloc(BUFFER_SIZE);

    m_bufferType = USBT_RingBuffer;
    m_buffer.m_ringBuffer = new RingBuffer((uint8_t*)m_memory, BUFFER_SIZE, true);    
}

UpgradeUdiskSource::~UpgradeUdiskSource()
{
    delete m_buffer.m_ringBuffer;
    free(m_memory);    
}

    
    
    
}