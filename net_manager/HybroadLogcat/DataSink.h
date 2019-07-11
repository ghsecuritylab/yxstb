#ifndef _DataSink_H_
#define _DataSink_H_

#include <stdint.h>

#ifdef __cplusplus

namespace android {

class DataFilter;
class DataSource;
class RingBuffer;

class DataSink {
public:
    DataSink();
    virtual ~DataSink();

    int setBuffer(RingBuffer* buffer) { m_ringBuffer = buffer; return 0; }
    int setDataSize(uint32_t size)  { m_dataSize = size; return 0; }
    virtual void receiveData() = 0;
protected:
    RingBuffer* m_ringBuffer;
    uint32_t m_dataSize;

private:
};

} /* namespace android */

#endif /* __cplusplus */

#endif /* _DataSink_H_ */
