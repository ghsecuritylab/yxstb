#ifndef _DLNARegister_H_
#define _DLNARegister_H_

#include <string>
#include <curl/curl.h>
// #include <curl/types.h>
#include <curl/easy.h>

#include "Message.h"
#include "MessageTypes.h"
#include "MessageHandler.h"

#define HEAD_MAX_LEN 4*1024

#ifdef __cplusplus

namespace Hippo {
class DLNARegister : public MessageHandler {
	typedef enum{
		HttpState_eReadHead,
		HttpState_eReadData,
		HttpState_eReadLocation,
		HttpState_eReadFinish,
		HttpState_eReadError,
	}http_state_e;	
public:
	DLNARegister();
	~DLNARegister();
	
	virtual void handleMessage(Message *msg);	
	int registStb(char *requestUrl, int type);
	int readHttpHeadData(char *buf, char *redirect);
	int readHttpContentData(char *buf);
	int GetRegisterJosn(std::string& regJson);
    int registSTBToURG(int type);
    int GetBindJosn(std::string& regJson);    
    int handleRegisterResult(void);

private:
	std::string m_requestHead;
	std::string m_RetInfo;
	CURL *m_curl;
	http_state_e m_httpState;
	int	m_contentDataLen;
	int	m_recvDatLen;
	int m_type;
    class RegisterResult {
    public:
        RegisterResult()
            : errorcode(0) 
            , hbInterval(0)
            , hbip("")
            , hbport(0)
            , msg("") {}
        int         errorcode;
        int         hbInterval;
        std::string hbip;
        int         hbport;
        std::string msg;
    };
    RegisterResult m_Result;
};

} // namespace Hippo

#endif // __cplusplus


#endif 
