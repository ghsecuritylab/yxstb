#ifndef _FileStreamDataSource_H_
#define _FileStreamDataSource_H_

#ifdef __cplusplus

#include "DataSource.h"

namespace Hippo {

class FileStreamDataSource : public DataSource {
public:
    FileStreamDataSource();
    ~FileStreamDataSource();
    
    virtual bool start();
    
    int getWriteHead(uint8_t **bufPointer, uint32_t *bufLength);
    int submitWrite(uint8_t *bufPointer, uint32_t bufLength);
    void receiveError();
};    
    
} // namespace Hippo

#endif //__cplusplus

#endif // _FileStreamDataSource_H_