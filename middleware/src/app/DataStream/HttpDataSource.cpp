#include "HttpDataSource.h"
#include "RingBuffer.h"
#include "DataSink.h"

#include "mid/mid_http.h"

#include <string.h>
#include <stdio.h>
#include <unistd.h> //for usleep

namespace Hippo {

HttpDataSource::HttpDataSource()
{
	mcookieInfo.clear();
}

HttpDataSource::~HttpDataSource()
{
}


static int upgradeFileReceiveRun(int arg,int total, int offset, char* buf, int len)
{
	Hippo::HttpDataSource *receiver = (Hippo::HttpDataSource *)arg;

	receiver->receiveData(total, buf, len);

	return 0;

}

static void upgradeFileReceiveEnd(int result, char* buf, int len, int arg)
{
	Hippo::HttpDataSource *receiver = (Hippo::HttpDataSource *)arg;

	if(result != 0){
		receiver->receiveError();
		return;
	}
	receiver->receiveEnd();
	return;
}

static int writeFlag = 0;
void
HttpDataSource::receiveData(int total, char *buf, int len)
{
	uint8_t *bufPointer[1] = {NULL};
	uint32_t bufLength = 0;
	uint32_t offset = 0;

	if (!m_ringBuffer) {
		printf("Error: m_ringBuffer is null\n");
		return;
	}

	if(writeFlag == 0){
		printf("HttpDataSource: [%d] : total = %d\n", __LINE__, total);
		//m_dataSink->setDataSize(total);
		tellDataSize(total);
		writeFlag = 1;
	}
	while(1){
		m_ringBuffer->getWriteHead(bufPointer, &bufLength);
		//printf("HttpDataSource: bufLength = %d, len = %d\n", bufLength, len);
		if(bufLength >= len){
			memcpy(bufPointer[0], buf + offset, len);
			m_ringBuffer->submitWrite(bufPointer[0], len);
			if(m_dataSink)
			    m_dataSink->onDataArrive();
			break;
		} else {
		    usleep(500);
			memcpy(bufPointer[0], buf + offset, bufLength);
			m_ringBuffer->submitWrite(bufPointer[0], bufLength);
			offset = offset + bufLength;
			len = len - bufLength;
			if(m_dataSink)
				m_dataSink->onDataArrive();
		}
	}

}

void
HttpDataSource::receiveEnd()
{
	m_dataSink->onEnd();
}

void
HttpDataSource::receiveError()
{
	m_dataSink->onError();
}

bool
HttpDataSource::start()
{
    char* cookie = NULL;
    if (!mcookieInfo.empty()) {
	cookie = (char*)malloc(mcookieInfo.length());
        strcpy(cookie, mcookieInfo.c_str());
    }
    writeFlag = 0;
	mid_http_writecall((char *)m_sourceAddress.c_str(), (mid_http_f)upgradeFileReceiveEnd,(int)this,(mid_write_f)upgradeFileReceiveRun, cookie);
    if (!cookie)
        free(cookie);
	return true;
}

bool
HttpDataSource::stop()
{
    mid_http_break((mid_http_f)upgradeFileReceiveEnd, (int)this);
    return false;
}

} /* namespace Hippo */
