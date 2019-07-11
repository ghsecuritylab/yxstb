#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "DLNARegister.h"
#include "DLNANat.h"
#include "DlnaAssertions.h"
#include "KeyDispatcher.h"
#include "browser/browser_event.h"
#include "AppSetting.h"

#include "app_heartbit.h"
#include "Assertions.h"
#include "mid/mid_tools.h"
#include "mid_sys.h"
#include "ipanel_event.h"
#include "json/json.h"
#include "sys_basic_macro.h"
#include "NetworkFunctions.h"

extern "C" {
    int UpnpGetServerPort();
    char *UpnpGetServerIpAddress();
}

namespace Hippo {

DLNARegister::DLNARegister()
{
	m_contentDataLen = 0;
	m_recvDatLen = 0;
	m_httpState = HttpState_eReadHead;

}

DLNARegister::~DLNARegister()
{
	DLNA_LOG("~DLNARegister \n");
}

static int str_lwr(char *str)
{
	while(*str != '\0'){
		*str = tolower(*str);
		str++;
	}
	return 0;
}

int DLNARegister::readHttpHeadData(char *buf, char *redirect)
{
	char *head_end = NULL;
	char *head_data = NULL;
	char *p = NULL;
	CURLcode res;
	size_t iolen = 0;
	int length = 0;
	m_httpState = HttpState_eReadFinish;
	// return 0;

	res = curl_easy_recv(m_curl, buf, HEAD_MAX_LEN, &iolen);
	if(res == CURLE_AGAIN){
		m_httpState = HttpState_eReadHead;
		DLNA_LOG_ERROR("DLNARegister::readHttpHeadData socket is not ready, try again\n");
		return 0;
	}
	if(res != CURLE_OK ){
		m_httpState = HttpState_eReadError;
		DLNA_LOG_ERROR("DLNARegister::readHttpHeadData HttpState_eReadError(%s)\n", strerror(res));
		return 0;
	}
	head_end = strstr(buf,"\r\n\r\n");
	if(head_end == NULL){
		m_httpState = HttpState_eReadError;
		DLNA_LOG_ERROR("DLNARegister::readHttpHeadData read http head error\n");
		return 0;
	}
	head_data = (char *)malloc(head_end - buf + 4);
	if(head_data == NULL){
		DLNA_LOG_ERROR("DLNARegister::readHttpHeadData malloc error\n");
		return 0;
	}
	memset(head_data, 0, sizeof(head_data));
	strncpy(head_data, buf, head_end - buf);
	DLNA_LOG_VERBOSE("DLNARegister::readHttpHeadData head data(%s)\n", head_data);
	str_lwr(head_data);
	DLNA_LOG_VERBOSE("DLNARegister::readHttpHeadData head data(%s)\n", head_data);
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
			DLNA_LOG_ERROR("DLNARegister::readHttpHeadData Location not fond!\n");
			free(head_data);
			return 0;
		}
		p += 10;
		length = mid_tool_line_len(p);
		redirect = (char*)malloc(length + 4);
		if(redirect == NULL){
			DLNA_LOG_ERROR("DLNARegister::readHttpHeadData malloc error\n");
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
		DLNA_LOG_WARNING("DLNARegister::readHttpHeadData not 200 OK\n");
		free(head_data);
		return 0;
	}
	if((p = strstr(head_data, "content-length: ")) != NULL){
		if(sscanf(p + 16, "%d", &m_contentDataLen) != 1 || m_contentDataLen < 0){
			m_httpState = HttpState_eReadError;
			DLNA_LOG_WARNING("DLNARegister::readHttpHeadData length(%d)\n", m_contentDataLen);
			free(head_data);
			return 0;
		}
	}
	free(head_data);
	m_recvDatLen += iolen - (head_end - buf);
	m_RetInfo += head_end;

	if(m_recvDatLen >= m_contentDataLen){
		m_httpState = HttpState_eReadFinish;
	} else {
		m_httpState = HttpState_eReadData;
	}
	return 0;
}

int DLNARegister::readHttpContentData(char *buf)
{
	CURLcode res;
	size_t iolen = 0;

	res = curl_easy_recv(m_curl, buf, HEAD_MAX_LEN, &iolen);
	if(res == CURLE_AGAIN){
		m_httpState = HttpState_eReadData;
		DLNA_LOG_ERROR("DLNARegister::readHttpContentData socket is not ready, try again\n");
		return 0;
	}
	if(res != CURLE_OK ){
		m_httpState = HttpState_eReadError;
		DLNA_LOG_ERROR("DLNARegister::readHttpContentData HttpState_eReadError %s\n", strerror(res));
		return 0;
	}
	m_recvDatLen += iolen;
	if(m_recvDatLen >= m_contentDataLen){
		m_httpState = HttpState_eReadFinish;
	}
	m_RetInfo += buf;

	return 0;

}

int DLNARegister::handleRegisterResult(void)
{
    if (m_RetInfo.empty()) {
        DLNA_LOG_ERROR("DLNA handleRegisterResult error: no content\n");
        return -1;
    }

    printf("handleRegisterResult: %s\n", m_RetInfo.c_str());
    struct json_object * obj = NULL, *po = NULL;
    obj = json_tokener_parse(m_RetInfo.c_str());
    if (!obj) {
        DLNA_LOG_ERROR("DLNA handleRegisterResult parse(m_RetInfo) error.\n");
        return -1;
    }
    po = json_object_object_get(obj, "errorcode");
    if (!po) {
        DLNA_LOG_ERROR("DLNA handleRegisterResult error: no errorcode defined. suppose to success.\n");
        return -1;
    }
    m_Result.errorcode = atoi(json_object_get_string(po));

    po = json_object_object_get(obj, "hbInterval");
    if (po)
        m_Result.hbInterval = atoi(json_object_get_string(po));
    else
        m_Result.hbInterval = 60;

    po = json_object_object_get(obj, "hbip");
    if (po) {
        m_Result.hbip = json_object_get_string(po);
    } else {
        if (m_Result.errorcode == 100) {
            DLNA_LOG_ERROR("DLNA handleRegisterResult error: errorcode = 100 but no hbip defined.\n");
            return -1;
        }
    }

    po = json_object_object_get(obj, "hbport");
    if (po) {
        m_Result.hbport = atoi(json_object_get_string(po));
    } else {
        if (m_Result.errorcode == 100) {
            DLNA_LOG_ERROR("DLNA handleRegisterResult error: errorcode = 100 but no hbport defined.\n");
            return -1;
        }
    }

    po = json_object_object_get(obj, "msg");
    if (po) {
        m_Result.msg = json_object_get_string(po);
        DLNA_LOG("DLNA handleRegisterResult: message: %s\n", m_Result.msg.c_str());
    }

    DLNANat * nat = DLNANat::GetInstance();
    if (!nat) {
        DLNA_LOG_ERROR("DLNA handleRegisterResult error: DLNANat::GetInstance() failed.\n");
    }
    nat->Start(m_Result.hbip.c_str(), m_Result.hbport, m_Result.hbInterval);

    return 0;
}

void
DLNARegister::handleMessage(Message * msg)
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
		DLNA_LOG_ERROR("ChannelListHttpCall::handleMessage what(%d) is not request channellist\n", msg->what);
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
			DLNA_LOG_ERROR("DLNARegister::handleMessage curl_easy_send error(%s)\n", curl_easy_strerror(res));
			return;
		}
		DLNA_LOG("DLNARegister::handleMessage Reading response.\n");
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
                handleRegisterResult();
				delete this;
			}else if(m_httpState == HttpState_eReadError){
				DLNA_LOG_ERROR("DLNARegister::handleMessage HttpState_eReadHead Error\n");
				delete this;
			} else if(m_httpState == HttpState_eReadLocation){
				if(redirect != NULL){
					curl_easy_cleanup(m_curl);
					m_RetInfo.clear();
					registStb(redirect, m_type);
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
                handleRegisterResult();
				delete this;
				return;
			}else if(m_httpState == HttpState_eReadError){
				DLNA_LOG_ERROR("registStb::handleMessage HttpState_eReadData Error\n");
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

int
DLNARegister::GetRegisterJosn(std::string& regJson)
{
    char stbId[64] = {0};
    char account[64] = {0};
    const char *method = "StbRegister";
    const char *stbUa = "EC2108";

    char ifname[URL_LEN] = { 0 };
    char ifaddr[URL_LEN] = { 0 };
    network_default_ifname(ifname, URL_LEN);

    char deviceInfo[128] = {0};
    mid_sys_serial(stbId);

    sprintf(deviceInfo, "[\"LOCATION: http://%s:%d/AVRenderService.xml\"]", UpnpGetServerIpAddress(), UpnpGetServerPort());
    appSettingGetString("ntvuser", account, 64, 0);

    std::string strStbID;
    std::string strStbIP;
    std::string strAccount;
    std::string strMethod;
    std::string strStbUa;
    std::string strDeviceInfo;

    strStbID += stbId;
    strStbIP += network_address_get(ifname, ifaddr, URL_LEN);
    strAccount += account;
    strMethod += method;
    strStbUa += stbUa;
    strDeviceInfo += deviceInfo;


    regJson = "{\"stbId\":\"" + strStbID + "\",\"stbIp\":\"" + strStbIP + "\",\"stbUa\":\"" + strStbUa + "\",\"account\":\"" + strAccount + "\",\"method\":\"" + method + "\",\"deviceInfo\":" + strDeviceInfo + "}";

    return 0;
}
int
DLNARegister::GetBindJosn(std::string& regJson)
{
    char stbId[64] = {0};
    const char *method = "StbValidCodeRequest";

    char ifname[URL_LEN] = { 0 };
    char ifaddr[URL_LEN] = { 0 };
    network_default_ifname(ifname, URL_LEN);

    mid_sys_serial(stbId);

    std::string strStbID;
    std::string strStbIP;
    std::string strMethod;
    strStbID += stbId;
    strStbIP += network_address_get(ifname, ifaddr, URL_LEN);
    strMethod += method;

    regJson = "{\"stbId\":\"" + strStbID + "\",\"stbIp\":\"" + strStbIP + "\",\"method\":\"" + method + "\"}";

    return 0;
}

int
DLNARegister::registStb(char *requestUrl, int type)
{
	char request_head[1024] ={0};
	char server_url[64] ={0};
	char stbId[64] = {0};
	char *p = NULL;
	int sockfd = 0;
	int len = 0;

	CURLcode res;
	size_t iolen;

	if(strncmp(requestUrl, "http://", 7)){
		DLNA_LOG_ERROR("DLNARegister::registStb url(%s)\n", requestUrl);
		return 0;
	}
	p = strchr(requestUrl + 7, '/');
	if(p == NULL){
		DLNA_LOG_ERROR("DLNARegister::registStb request path is empty\n");
		return 0;
	}
	memset(request_head, 0, sizeof(request_head));
	memset(server_url, 0, sizeof(server_url));
	snprintf(server_url, p - requestUrl + 1, requestUrl );

	len = sprintf(request_head, "POST %s HTTP/1.1\r\nHost: %s\r\n", p, server_url + 7);
	mid_sys_serial(stbId);
	len += sprintf(request_head + len, "Ais-StbId: %s\r\n", stbId);

	std::string jsonInfo;
	if (type == 1) {
	    GetRegisterJosn(jsonInfo);
    } else {
        GetBindJosn(jsonInfo);
    }
	len += sprintf(request_head + len, "Content-Length: %d\r\n", jsonInfo.size());
	len += sprintf(request_head + len, "\r\n%s", jsonInfo.c_str());
	len += sprintf(request_head + len, "\r\n");

	m_requestHead.clear();
	m_requestHead = request_head;
	printf("DLNARegister::registStb requesthead(%s)\n", m_requestHead.c_str());

	m_curl = curl_easy_init();

	if(!m_curl) {
		DLNA_LOG_ERROR("DLNARegister::registStb curl init failed\n");
		return 0;
	}

	curl_easy_setopt(m_curl, CURLOPT_URL, server_url);
	/* Do not do the transfer - only connect to host */
	curl_easy_setopt(m_curl, CURLOPT_CONNECT_ONLY, 1L);
	res = curl_easy_perform(m_curl);

	if(CURLE_OK != res)	{
		DLNA_LOG_ERROR("DLNARegister::registStb curl_easy_perform error(%s)\n", strerror(res));
		return 0;
	}
	/* Extract the socket from the curl handle - we'll need it for waiting */
	res = curl_easy_getinfo(m_curl, CURLINFO_LASTSOCKET, &sockfd);

	if(CURLE_OK != res)	{
		DLNA_LOG_ERROR("DLNARegister::registStb curl_easy_getinfo error(%s)\n", curl_easy_strerror(res));
		return 0;
	}
	m_type = type;

	Message *message = this->obtainMessage(MessageType_Timer, 0, sockfd);
	this->sendMessageDelayed(message, 1000);

	return 0;
}

int
DLNARegister::registSTBToURG(int type)
{
    DLNA_LOG("registSTBToURG\n");
    char requestUrl[256] = {0};
    KeyDispatcherPolicy *keyPolocy = NULL;
    keyPolocy = keyDispatcher().getPolicy(EIS_IRKEY_URG_REG);
    strncpy(requestUrl, keyPolocy->mKeyUrl.c_str(), 256);
    DLNA_LOG("registSTBToURG requestUrl = %s\n", requestUrl);
    keyPolocy = keyDispatcher().getPolicy(EIS_IRKEY_DLNA_PUSH);
    DLNA_LOG("registSTBToURG url = %s\n", keyPolocy->mKeyUrl.c_str());
    registStb(requestUrl, type);
    return 0;

}

}

