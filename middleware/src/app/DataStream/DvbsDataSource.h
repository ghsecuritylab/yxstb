#ifndef  _TRUCK_EC2108_C27_IPSTB_SRC_APP_DATASTREAM_DVBSDATASOURCE_H_
#define  _TRUCK_EC2108_C27_IPSTB_SRC_APP_DATASTREAM_DVBSDATASOURCE_H_

#if 0
#include "DataSource.h"

namespace Hippo {
const unsigned short kBufHeadSize = 4;
class DataSink;
class DvbsDataSource : public DataSource
{
    public:
        DvbsDataSource();

        ~DvbsDataSource();

        virtual bool start();
        virtual bool stop();

        void receiveData(unsigned short section, unsigned char *buf, unsigned short len);
        void receiveEnd();
        void receiveError();

        DataSink *DataReceiver() { return m_dataSink; }

    private:
};

} 

#endif
#endif
