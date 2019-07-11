
#include "ParseCmd.h"
#include "ParseXML.h"
#include "MonitorTool.h"
#include "MonitorDef.h"
#include "MonitorAgent.h"
#include "MonitorManager.h"

//#include <openssl/bn.h>
//#include <openssl/rsa.h>
//#include <openssl/aes.h>
#include <string.h>


void monitorSetUDPLogAddress(char *address);
void monitorSetFTPLogAddress(char *address);
void MonitorLog();

/***************************************
*ParseWrite Class
*
****************************************/
ParseWrite::ParseWrite()
{
}

ParseWrite::~ParseWrite()
{
}


int ParseWrite::Exec(moni_buf_t pBuf)
{
	int ret = 0;
	int response_ret = 0;
	int len = 0;
	int capFlag = 0;
	//
	do{
		if (strlen(m_szCmdInfo[1]) == 0 || strlen(m_szCmdInfo[1]) > 63){
			response_ret = MSG_COMMAND_FAIL_ERROR;
			printf("This commond(%s) is error !\n", pBuf->buf);
			break;
		}

		if (strlen(m_szCmdInfo[3]) == 0){
			response_ret = MSG_COMMAND_FAIL_ERROR;
			printf("the write command is no data\n");
			break;
		}

		//ÅäºÏ³¯¸è¹¤¾ß
		len = IDENTIFY_CODE_LEN + SESSIONID_LEN + strlen(m_szCmdInfo[0]) + 1 + strlen(m_szCmdInfo[1]) + 1;
		len += strlen(m_szCmdInfo[2]) - 4;

#ifdef ChongQing_HD
    	if(!strcmp(m_szCmdInfo[1], "reboot_time")) {
        	if(atoi(pBuf->buf + len + 4) < -1 || atoi(buf->buf + len + 4) > 24) {
            	response_ret =  MSG_PARAMETER_ERROR;
            	break;
        	}
    	}
#endif
		printf("para = %s info = %s\n", m_szCmdInfo[1], m_szCmdInfo[3]);


		std::string strMT(m_szCmdInfo[1]);

		std::string strSTB = MonitorManager::cmdNameXML()->GetSTBPara(strMT);

		printf("MT = %s strSTB = %s\n", strMT.c_str(), strSTB.c_str());


        //if (strMT == "stbIP" && strSTB == "Network.op.NetIP") {
        if (strMT == "stbIP" || strMT == "gateway" || strMT == "netmask" || strMT == "dns") {
            // printf("test:static %s !!\n", strMT.c_str());
            char stbIPcmp[READ_BUFFER] = {0};
			ret = MonitorAgent::GetInstance()->ReadAgent(MGMT_CONFIG_MT, strSTB.c_str(), stbIPcmp, READ_BUFFER);
            if (!strcmp(stbIPcmp, m_szCmdInfo[3])) {
                printf("%s no change!\n", strMT.c_str());
        	    response_ret =  MSG_CONNECT_OK;
                break;
            }
        }

        #ifdef ANDROID
        if (strMT == "LogServer") {
            monitorSetUDPLogAddress(m_szCmdInfo[3]);
            MonitorLog();
            MonitorAgent::GetInstance()->WriteAgent(MGMT_CONFIG_MT, "SetLogServer", m_szCmdInfo[3], READ_BUFFER);
            response_ret =  MSG_CONNECT_OK;
            break;


        } else if (strMT == "LogFtpServer") {
            printf("ftp server is %s\n", m_szCmdInfo[3]);
            monitorSetFTPLogAddress(m_szCmdInfo[3]);
            MonitorLog();
            MonitorAgent::GetInstance()->WriteAgent(MGMT_CONFIG_MT, "SetLogFtpServer", m_szCmdInfo[3], READ_BUFFER);
            response_ret =  MSG_CONNECT_OK;
            break;

        }

        #endif

		if (strSTB == "")
			ret = MonitorAgent::GetInstance()->WriteAgent(MGMT_CONFIG_MT, strMT.c_str(), m_szCmdInfo[3], READ_BUFFER);
		else
			ret = MonitorAgent::GetInstance()->WriteAgent(MGMT_CONFIG_MT, strSTB.c_str(), m_szCmdInfo[3], READ_BUFFER);

    	if(ret == 0)
        	response_ret =  MSG_CONNECT_OK;
    	else
        	response_ret =  MSG_PARAMETER_ERROR;

	}while(0);

	sprintf(pBuf->buf, "%dwrite^%s", response_ret, m_szCmdInfo[1]);
    pBuf->len = 3 + strlen("write") + 1 + strlen(m_szCmdInfo[1]);

    if(pBuf->extend != NULL) {
        sprintf(pBuf->buf + pBuf->len, "^");
        pBuf->len += 1;
    }

    printf("Command response %s\n", pBuf->buf);
	return 0;

}


