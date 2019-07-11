
#include "DataSink.h"

#include <stdio.h>


namespace android {

DataSink::DataSink()
    : m_ringBuffer(NULL)
    , m_dataSize(0)
{
}

DataSink::~DataSink()
{
}

} /* namespace android */
