
#include "DataSource.h"

#include "DataSink.h"

#include <stdio.h>


namespace android {

DataSource::DataSource()
	: m_dataSink(NULL)
	, m_ringBuffer(NULL)
{
}

DataSource::~DataSource()
{
}

bool 
DataSource::attachSink(DataSink *sink)
{
    if (m_dataSink)
        return false;
    m_dataSink = sink;
    return true;
}

bool 
DataSource::detachSink(DataSink *sink)
{
    if (m_dataSink != sink)
        return false;
    m_dataSink = NULL;
    return true;
}

bool 
DataSource::tellDataSize(uint32_t size)
{
    if (m_dataSink == 0)
        return false;
    return !m_dataSink->setDataSize(size);
}

} /* namespace android */
