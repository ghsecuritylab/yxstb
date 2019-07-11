#ifndef __TRUNK_EC2108_C27_IPSTB_SRC_APP_PROGRAM_C20_ChannelListHttpCall_H_
#define __TRUNK_EC2108_C27_IPSTB_SRC_APP_PROGRAM_C20_ChannelListHttpCall_H_

#include <string.h>
#include <string>
#include <curl/curl.h>
#include <curl/easy.h>

#include "Message.h"
#include "MessageTypes.h"
#include "MessageHandler.h"

#define HEAD_MAX_LEN 4*1024
#include "DataSink.h"
#include <iostream>

#ifdef __cplusplus

namespace Hippo {
#ifdef HUAWEI_C20
class HttpDataSource;
class ChannelListHttpCall : public DataSink,  MessageHandler {
public:
	ChannelListHttpCall();
	~ChannelListHttpCall();

    enum MessageCode {
    	MC_DataArrive,
    	MC_End,
    	MC_Error
    };

    virtual bool onDataArrive();
    virtual bool onError();
    virtual bool onEnd();

    virtual void receiveData();
    virtual void receiveError();
    virtual void receiveEnd();

    virtual void handleMessage(Message *msg);
    int requstChannelList(char *requestUrl, int type);

private:
	std::string m_channelList;
	char* m_memory;
	HttpDataSource* channelSource;

};

#else
class ChannelListHttpCall : public MessageHandler {
	typedef enum{
		HttpState_eReadHead,
		HttpState_eReadData,
		HttpState_eReadLocation,
		HttpState_eReadFinish,
		HttpState_eReadError,
	}http_state_e;
public:
	ChannelListHttpCall();
	~ChannelListHttpCall();

	virtual void handleMessage(Message *msg);
	int requstChannelList(char *requestUrl, int type);
	int readHttpHeadData(char *buf, char *redirect);
	int readHttpContentData(char *buf);
	void parseReceiveData(void);

private:
	std::string m_channelList;
	std::string m_requestHead;
	CURL *m_curl;
	http_state_e m_httpState;
	int	m_contentDataLen;
	int	m_recvDatLen;
	int m_httpType;
};
#endif
} // namespace Hippo

#endif // __cplusplus


#endif // __TRUNK_EC2108_C27_IPSTB_SRC_APP_PROGRAM_C20_ChannelListHttpCall_H_
