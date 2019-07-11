#ifdef HUAWEI_C20

#include "ProgramParserC20.h"
#include "ProgramAssertions.h"
#include "ChannelListHttpCall.h"
#include "HttpDataSource.h"
#include "RingBuffer.h"
#include "app_heartbit.h"

#define BUFFER_SIZE	64*1024

extern char* global_cookies;

namespace Hippo {

ChannelListHttpCall::ChannelListHttpCall()
{     PROGRAM_LOG("\n");
	m_channelList.clear();
}

ChannelListHttpCall:: ~ChannelListHttpCall()
{
    free(m_memory);
    delete m_ringBuffer;
    delete channelSource;
    PROGRAM_LOG("~ChannelListHttpCall \n");
}


bool
ChannelListHttpCall::onDataArrive()
{
    return sendEmptyMessage(MC_DataArrive);
}

bool
ChannelListHttpCall::onError()
{
    return sendEmptyMessage(MC_Error);
}

bool
ChannelListHttpCall::onEnd()
{
    return sendEmptyMessage(MC_End);
}


void
ChannelListHttpCall::handleMessage(Message * msg)
{PROGRAM_LOG("msg->what=%d\n",msg->what);
    switch (msg->what) {
    case MC_DataArrive:
        receiveData();
        break;
    case MC_End:
        receiveEnd();
        break;
    case MC_Error:
        receiveError();
        break;
    default:
        break;
    }
}

void
ChannelListHttpCall::receiveData()
{PROGRAM_LOG("\n");
    char* buf = NULL;

    while (1) {
        uint8_t* bufPointer;
        uint32_t bufLength;

         m_ringBuffer->getReadHead(&bufPointer, &bufLength);
         if (bufLength == 0)
            break;
         buf = (char*)malloc(bufLength+4);
        memset(buf, 0, bufLength+4);
        memcpy(buf, bufPointer, bufLength);
        m_channelList += buf;
        free(buf);
        m_ringBuffer->submitRead(bufPointer, bufLength);
    }

}

void
ChannelListHttpCall::receiveError()
{
    PROGRAM_LOG_ERROR("receive error\n");
    delete this;
}

void
ChannelListHttpCall::receiveEnd()
{
    PROGRAM_LOG("m_channelList.c_str()=[%s]\n", m_channelList.c_str());
    programParser().parseChannelList(NULL, m_channelList.c_str());
    delete this;
}


int
ChannelListHttpCall::requstChannelList(char *requestUrl, int type)
{PROGRAM_LOG("ChannelListHttpCall::requstChannelList url(%s)\n", requestUrl);

    if(strncmp(requestUrl, "http://", 7)){
        PROGRAM_LOG_ERROR("ChannelListHttpCall::requstChannelList url(%s)\n", requestUrl);
        return 0;
    }
    m_memory = (char*)malloc(BUFFER_SIZE);

    m_ringBuffer = new RingBuffer((uint8_t*)m_memory, BUFFER_SIZE);

    channelSource = new HttpDataSource();
    channelSource->SetRequestUrl(requestUrl);
    channelSource->setBuffer(m_ringBuffer);
    if(global_cookies){
        channelSource->setCookieInfo(global_cookies);
    }
    channelSource->attachSink(this);
    channelSource->start();

    return 0;
}


}
#else
#include <ctype.h>
#include "ProgramParserC20.h"
#include "ProgramAssertions.h"
#include "ChannelListHttpCall.h"

#include "app_heartbit.h"
#include "Assertions.h"
#include "mid/mid_tools.h"

namespace Hippo {

ChannelListHttpCall::ChannelListHttpCall()
{
	m_channelList.clear();
	m_contentDataLen = 0;
	m_recvDatLen = 0;
	m_httpState = HttpState_eReadHead;

}

ChannelListHttpCall:: ~ChannelListHttpCall()
{
	PROGRAM_LOG("~ChannelListHttpCall \n");
}

static int str_lwr(char *str)
{
	while(*str != '\0'){
		*str = tolower(*str);
		str++;
	}
	return 0;
}

int ChannelListHttpCall::readHttpHeadData(char *buf, char *redirect)
{
	char *head_end = NULL;
	char *head_data = NULL;
	char *p = NULL;
	CURLcode res;
	size_t iolen = 0;
	int length = 0;

	res = curl_easy_recv(m_curl, buf, HEAD_MAX_LEN, &iolen);
	if(res == CURLE_AGAIN){
		m_httpState = HttpState_eReadHead;
		PROGRAM_LOG_ERROR("ChannelListHttpCall::readHttpHeadData socket is not ready, try again\n");
		return 0;
	}
	if(res != CURLE_OK ){
		m_httpState = HttpState_eReadError;
		PROGRAM_LOG_ERROR("ChannelListHttpCall::readHttpHeadData HttpState_eReadError(%s)\n", strerror(res));
		return 0;
	}
	head_end = strstr(buf,"\r\n\r\n");
	if(head_end == NULL){
		m_httpState = HttpState_eReadError;
		PROGRAM_LOG_ERROR("ChannelListHttpCall::readHttpHeadData read http head error\n");
		return 0;
	}
	head_data = (char *)malloc(head_end - buf + 4);
	if(head_data == NULL){
		PROGRAM_LOG_ERROR("ChannelListHttpCall::readHttpHeadData malloc error\n");
		return 0;
	}
	memset(head_data, 0, sizeof(head_data));
	strncpy(head_data, buf, head_end - buf);
	PROGRAM_LOG("ChannelListHttpCall::readHttpHeadData head data(%s)\n", head_data);
	str_lwr(head_data);
	PROGRAM_LOG("ChannelListHttpCall::readHttpHeadData head data(%s)\n", head_data);
	if(strncmp(head_data, "http/1.", 7)){
		m_httpState = HttpState_eReadError;
		free(head_data);
		return 0;
	}
	head_end += 4;

	if(head_data[9] == '3'){
		p = strstr(head_data, "location: ");
		if(p == NULL){
			m_httpState = HttpState_eReadError;
			PROGRAM_LOG_ERROR("ChannelListHttpCall::readHttpHeadData Location not fond!\n");
			free(head_data);
			return 0;
		}
		p += 10;
		length = mid_tool_line_len(p);
		redirect = (char*)malloc(length + 4);
		if(redirect == NULL){
			PROGRAM_LOG_ERROR("ChannelListHttpCall::readHttpHeadData malloc error\n");
			free(head_data);
			return 0;
		}
		memset(redirect, 0, sizeof(redirect));
		mid_tool_line_first(p, redirect);
		m_httpState = HttpState_eReadLocation;
		free(head_data);
		return 0;
	}
	if(strncmp(head_data + 9, "20", 2)){
		m_httpState = HttpState_eReadError;
		PROGRAM_LOG_ERROR("ChannelListHttpCall::readHttpHeadData not 200 OK\n");
		free(head_data);
		return 0;
	}
	if((p = strstr(head_data, "content-length: ")) != NULL){
		if(sscanf(p + 16, "%d", &m_contentDataLen) != 1 || m_contentDataLen < 0){
			m_httpState = HttpState_eReadError;
			PROGRAM_LOG_ERROR("ChannelListHttpCall::readHttpHeadData length(%d)\n", m_contentDataLen);
			free(head_data);
			return 0;
		}
	}
	free(head_data);
	m_recvDatLen += iolen - (head_end - buf);
	m_channelList += head_end;

	if(m_recvDatLen >= m_contentDataLen){
		m_httpState = HttpState_eReadFinish;
	} else {
		m_httpState = HttpState_eReadData;
	}
	return 0;
}

int ChannelListHttpCall::readHttpContentData(char *buf)
{
	CURLcode res;
	size_t iolen = 0;

	res = curl_easy_recv(m_curl, buf, HEAD_MAX_LEN, &iolen);
	if(res == CURLE_AGAIN){
		m_httpState = HttpState_eReadData;
		PROGRAM_LOG_ERROR("ChannelListHttpCall::readHttpContentData socket is not ready, try again\n");
		return 0;
	}
	if(res != CURLE_OK ){
		m_httpState = HttpState_eReadError;
		PROGRAM_LOG_ERROR("ChannelListHttpCall::readHttpContentData HttpState_eReadError %s\n", strerror(res));
		return 0;
	}
	m_recvDatLen += iolen;
	if(m_recvDatLen >= m_contentDataLen){
		m_httpState = HttpState_eReadFinish;
	}
	m_channelList += buf;

	return 0;

}

void
ChannelListHttpCall::handleMessage(Message * msg)
{
	struct timeval tv;
	fd_set infd, outfd, errfd;
	Message *message;
	char head_buf[HEAD_MAX_LEN];
	int sockfd = 0;
	int rc = -1;
	CURLcode res;
	size_t iolen = 0;
	if(msg->what != MessageType_Timer){
		PROGRAM_LOG_WARNING("ChannelListHttpCall::handleMessage what(%d) is not request channellist\n", msg->what);
		return;
	}

	tv.tv_sec = 0;
	tv.tv_usec= 0;

	sockfd = msg->arg2;
	FD_ZERO(&infd);
	FD_ZERO(&outfd);

	if(msg->arg1 == 0){
		FD_SET(sockfd, &outfd);
		rc = select(sockfd + 1, &infd, &outfd, NULL, &tv);
		if(rc <= 0){
			message = this->obtainMessage(MessageType_Timer, 0, sockfd);
			this->sendMessageDelayed(message, 100);
			return;
		}
		res = curl_easy_send(m_curl, m_requestHead.c_str(), strlen(m_requestHead.c_str()), &iolen);
		if(CURLE_OK != res){
			curl_easy_cleanup(m_curl);
			PROGRAM_LOG_ERROR("ChannelListHttpCall::handleMessage curl_easy_send error(%s)\n", curl_easy_strerror(res));
			return;
		}
		PROGRAM_LOG("ChannelListHttpCall::handleMessage Reading response.\n");
		message = this->obtainMessage(MessageType_Timer, 1, sockfd);
		this->sendMessageDelayed(message, 100);
	} else if(msg->arg1 == 1){
		FD_SET(sockfd, &infd);
		rc = select(sockfd + 1, &infd, &outfd, NULL, &tv);
		if(rc <= 0){
			message = this->obtainMessage(MessageType_Timer, 1, sockfd);
			this->sendMessageDelayed(message, 100);
			return;
		}
		memset(head_buf, 0, sizeof(head_buf));
		if(m_httpState == HttpState_eReadHead){
			char *redirect = NULL;
			readHttpHeadData(head_buf, redirect);
			if(m_httpState == HttpState_eReadFinish){
				curl_easy_cleanup(m_curl);
				parseReceiveData();
				delete this;
			}else if(m_httpState == HttpState_eReadError){
				PROGRAM_LOG_ERROR("ChannelListHttpCall::handleMessage HttpState_eReadHead Error\n");
				delete this;
			} else if(m_httpState == HttpState_eReadLocation){
				if(redirect != NULL){
					curl_easy_cleanup(m_curl);
					m_channelList.clear();
					requstChannelList(redirect, m_httpType);
					free(redirect);
				}
			}else {
				message = this->obtainMessage(MessageType_Timer, 1, sockfd);
				this->sendMessageDelayed(message, 100);
			}
		} else if(m_httpState == HttpState_eReadData){
			readHttpContentData(head_buf);
			if(m_httpState == HttpState_eReadFinish){
				curl_easy_cleanup(m_curl);
				parseReceiveData();
				delete this;
				return;
			}else if(m_httpState == HttpState_eReadError){
				PROGRAM_LOG_ERROR("ChannelListHttpCall::handleMessage HttpState_eReadData Error\n");
				curl_easy_cleanup(m_curl);
				delete this;
			} else {
				message = this->obtainMessage(MessageType_Timer, 1, sockfd);
				this->sendMessageDelayed(message, 100);
			}
		}

	}

	return;
}

void
ChannelListHttpCall::parseReceiveData(void)
{
	if(m_httpType == 1){
		programParser().parseChannelList(NULL, m_channelList.c_str());
	} else {
		PROGRAM_LOG("ChannelListHttpCall::parseReceiveData (%s)\n", m_channelList.c_str());
	}
}

int
ChannelListHttpCall::requstChannelList(char *requestUrl, int type)
{
	char request_head[1024] ={0};
	char server_url[64] ={0};
	char *p = NULL;
	int sockfd = 0;
	int len = 0;

	CURLcode res;
	size_t iolen;

	if(strncmp(requestUrl, "http://", 7)){
		PROGRAM_LOG_ERROR("ChannelListHttpCall::requstChannelList url(%s)\n", requestUrl);
		return 0;
	}
	p = strchr(requestUrl + 7, '/');
	if(p == NULL){
		PROGRAM_LOG_ERROR("ChannelListHttpCall::requstChannelList request path is empty\n");
		return 0;
	}
	memset(request_head, 0, sizeof(request_head));
	memset(server_url, 0, sizeof(server_url));
	snprintf(server_url, p - requestUrl + 1, requestUrl );

	len = sprintf(request_head, "GET %s HTTP/1.1\r\nHost: %s\r\n", p, server_url + 7);
	if(global_cookies){
		len += sprintf(request_head + len, "%s\r\n", global_cookies);
	}
	len += sprintf(request_head + len, "\r\n");

	m_requestHead.clear();
	m_requestHead = request_head;
	PROGRAM_LOG("ChannelListHttpCall::requstChannelList requesthead(%s)\n", m_requestHead.c_str());

	m_curl = curl_easy_init();

	if(!m_curl) {
		PROGRAM_LOG_ERROR("ChannelListHttpCall::requstChannelList curl init failed\n");
		return 0;
	}

	curl_easy_setopt(m_curl, CURLOPT_URL, server_url);
	/* Do not do the transfer - only connect to host */
	curl_easy_setopt(m_curl, CURLOPT_CONNECT_ONLY, 1L);
	res = curl_easy_perform(m_curl);

	if(CURLE_OK != res)	{
		PROGRAM_LOG_ERROR("ChannelListHttpCall::requstChannelList curl_easy_perform error(%s)\n", strerror(res));
		return 0;
	}
	/* Extract the socket from the curl handle - we'll need it for waiting */
	res = curl_easy_getinfo(m_curl, CURLINFO_LASTSOCKET, &sockfd);

	if(CURLE_OK != res)	{
		PROGRAM_LOG_ERROR("ChannelListHttpCall::requstChannelList curl_easy_getinfo error(%s)\n", curl_easy_strerror(res));
		return 0;
	}

	m_httpType = type;
	Message *message = this->obtainMessage(MessageType_Timer, 0, sockfd);
	this->sendMessageDelayed(message, 1000);

	return 0;
}


}

#endif
