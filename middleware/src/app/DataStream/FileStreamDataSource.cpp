
#include "FileStreamDataSource.h"
#include "RingBuffer.h"
#include "DataSink.h"
#include "TempFile.h"

#include <string>
#include <pthread.h>

static std::string s_path;
    
namespace Hippo {

FileStreamDataSource::FileStreamDataSource()
{
    
}    

FileStreamDataSource::~FileStreamDataSource()
{
    
}        

static void* readFileStream(void* arg)
{
    int size = 0;
    uint32_t fileLength = 0;
    uint32_t retLength = 0;
    uint32_t readLength = 0;
    unsigned char* bufferHead = 0;
    
    Hippo::FileStreamDataSource *reader = (Hippo::FileStreamDataSource *)arg;
            
    TempFile* tempData = new TempFile(s_path.c_str());
    if (tempData->open('r') == -1)
        return NULL;    
        
    size = tempData->size();
    while (1) {
        if (reader->getWriteHead(&bufferHead, &readLength) < 0)
            break;        
        
        retLength = tempData->read(bufferHead, readLength);
        if (!retLength) {
            reader->stop();
            break;
        }
        if (retLength < 0) {
            reader->receiveError();
            break;
        }
        reader->submitWrite(bufferHead, retLength);        
        fileLength += retLength;
    }
    
    tempData->close();
    return NULL;         
    
}

int 
FileStreamDataSource::getWriteHead(uint8_t **bufPointer, uint32_t *bufLength)
{
    if (m_ringBuffer == 0)
        return -1;
    return m_ringBuffer->getWriteHead(bufPointer, bufLength);
}

int 
FileStreamDataSource::submitWrite(uint8_t *bufPointer, uint32_t bufLength)
{
    if (m_ringBuffer == 0 || m_dataSink == 0)
        return -1;
    m_ringBuffer->submitWrite(bufPointer, bufLength);
    m_dataSink->onDataArrive();
    return 0;
}

void
FileStreamDataSource::receiveError()
{
    if (m_dataSink)
       m_dataSink->onError();
}

bool
FileStreamDataSource::start()
{
    if (m_sourceAddress.empty())
        return true;
    
    //TempFile* tempData = new TempFile(m_sourceAddress.c_str());    
    //tellDataSize(tempData->size());
    //delete tempData;
    
    s_path.clear();
    s_path = m_sourceAddress;        
    
    pthread_t pthread;    
    pthread_create(&pthread, NULL, readFileStream, this);
    
    return true;    
}
    
}

