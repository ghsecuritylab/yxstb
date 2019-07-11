/**
 * @file DvbsDataSource.cpp
 * @brief DDB write operation about ota upgrade. Push data to ring buffer cache.
 * @author Michael
 * @version 1.0
 * @date 2012-07-27
 */
#if 0
#include "RingBuffer.h"
#include "DvbsDataSource.h"
#include "OtaSystemSearch.h"

namespace Hippo
{

/**
 * @brief 
 */
DvbsDataSource::DvbsDataSource( )
{

}

/**
 * @brief 
 */
DvbsDataSource::~DvbsDataSource( )
{

}

/**
 * @brief start receive data 
 * 
 * @return 
 */
bool DvbsDataSource::start( )
{
	OtaSystem()->startReceive(this);
    return true;
}

/**
 * @brief 
 *
 * @return 
 */
bool DvbsDataSource::stop( )
{
    return true;
} 

/**
 * @brief 
 *
 * @param section the section number of download data table when ota parsing
 * @param buf the pointer of the block data in hte section
 * @param len the length of the block data in the section
 */
void DvbsDataSource::receiveData(unsigned short section, unsigned char *buf, unsigned short len)
{
	/* Here deal with data from ts */
	unsigned char *pDataBuf = NULL;
	unsigned int dataBufLen = 0;
	unsigned int     offset = 0;
    
    unsigned short headLen = 4;
    unsigned char  headBuf[4];        /* Head: 4Byte */

    headBuf[0] = section >> 8;
    headBuf[1] = section & 0x00FF;
    headBuf[2] = len >> 8;
    headBuf[3] = len & 0x00FF;

    /* First write head */
	while(1)
    {		
        m_ringBuffer->getWriteHead(&pDataBuf, &dataBufLen);
        if(dataBufLen>=headLen)
        {
            memcpy(pDataBuf, headBuf + offset, headLen);
            m_ringBuffer->submitWrite(pDataBuf, headLen);
            break;
        } 
        memcpy(pDataBuf, headBuf + offset, dataBufLen);
        m_ringBuffer->submitWrite(pDataBuf, dataBufLen);
        offset  = offset  + dataBufLen;
        headLen = headLen - dataBufLen;
    }

    /* Then write data */
    offset = 0;
	while(1)
    {		
        m_ringBuffer->getWriteHead(&pDataBuf, &dataBufLen);
        if(dataBufLen>=len)
        {
            memcpy(pDataBuf, buf + offset, len);
            m_ringBuffer->submitWrite(pDataBuf, len);
            m_dataSink->onDataArrive();
            break;
        } 
        memcpy(pDataBuf, buf + offset, dataBufLen);
        m_ringBuffer->submitWrite(pDataBuf, dataBufLen);
        offset = offset + dataBufLen;
        len    = len    - dataBufLen;
    }
}

/**
 * @brief 
 */
void DvbsDataSource::receiveEnd()
{
    m_dataSink->onEnd();
}

/**
 * @brief 
 */
void DvbsDataSource::receiveError()
{
    m_dataSink->onError();
}

}
#endif
