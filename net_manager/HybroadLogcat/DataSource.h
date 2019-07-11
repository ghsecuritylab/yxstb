#ifndef _DataSource_H_
#define _DataSource_H_

#include <stdint.h>

#ifdef __cplusplus

namespace android {

class DataSink;
class RingBuffer;

class DataSource {
public:
    DataSource();
    virtual ~DataSource();

    int setBuffer(RingBuffer* buffer) { m_ringBuffer = buffer; return 0; }

    virtual bool attachSink(DataSink *sink);
    virtual bool detachSink(DataSink *sink);

    bool tellDataSize(uint32_t size);

protected:
    DataSink* m_dataSink;
    RingBuffer* m_ringBuffer;

private:
};

} /* namespace android */

#endif /* __cplusplus */

#endif /* _DataSource_H_ */
