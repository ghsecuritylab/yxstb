
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


static int mid_tool_timezone2str(int pTimezone, char* buf);
char* monitorGetUDPLogAddress();
char* monitorGetFTPLogAddress();
int monitorGetLogLevel();
int monitorGetLogType();
int monitorGetLogOutPutType();

/*******************************************
*ParseRead Class
*
********************************************/
ParseRead::ParseRead()
{
}

ParseRead::~ParseRead()
{
}


int ParseRead::Exec(moni_buf_t pBuf)
{
	int ret = 0;
	int response_ret = 0;
	int channel_num  = 0;
	int channel_id   = 0;
	int argc = 0;
	void *argv[2] = {NULL};
	int capFlag = 0;
	//char *channel_info = NULL;

	if (pBuf == NULL)
		return -1;
	//
	pBuf->extend = (char *)malloc(sizeof(char) * READ_BUFFER);
	bzero(pBuf->extend, READ_BUFFER);

	//
	printf("read para = %s \n", m_szCmdInfo[1]);

	std::string strMT(m_szCmdInfo[1]);

	std::string strSTB = MonitorManager::cmdNameXML()->GetSTBPara(strMT);

	printf("MT = %s strSTB = %s\n", strMT.c_str(), strSTB.c_str());

    do {
	    if (strcmp(m_szCmdInfo[1], "StartCap") == 0) {
	        //old MonitorTool::GetInstance()->readCapFlag(&capFlag);
			sprintf(pBuf->extend, "-1", capFlag);
			response_ret =	MSG_CONNECT_OK;
			break;
		}

	    //get channel list
	    if (strSTB == "Channellist" || strMT == "Channellist") {
			argc = 1;
			argv[0] = &channel_num;
		    MonitorAgent::GetInstance()->NotifyAgent(MGMT_MT_GET_CHANNELNUM_TOTAL, argc, argv);
	        printf("channel_num = %d \n", channel_num);

	        //channel_info = (char *)malloc(channel_num * 1024 * 1024);
			//bzero(channel_info, channel_num * 1024 * 1024);

	        //for (channel_id = 1; channel_id <= channel_num; channel_id++) {
				argc = 2;
			    channel_id = 1;
				argv[0] = &channel_id;
				argv[1] = pBuf->extend;
				MonitorAgent::GetInstance()->NotifyAgent(MGMT_MT_GET_CHANNELINFO_I, argc, argv);
	            printf("channelInfo_len = %d \n", strlen(pBuf->extend));
			//}

            // printf("channel_info = %s \n", channel_info);
			response_ret =	MSG_CONNECT_OK;
		}

		else {
            #ifdef ANDROID
            if (strMT == "LogServer") {
                strcpy(pBuf->extend, monitorGetUDPLogAddress());
                response_ret =	MSG_CONNECT_OK;
                break;

            } else if (strMT == "LogFtpServer") {
                strcpy(pBuf->extend, monitorGetFTPLogAddress());
                response_ret =	MSG_CONNECT_OK;
                break;

            } else if (strMT == "LogType") {
                sprintf(pBuf->extend, "%d", monitorGetLogType());
                response_ret =	MSG_CONNECT_OK;
                break;

            } else if (strMT == "LogOutPutType") {
                sprintf(pBuf->extend, "%d", monitorGetLogOutPutType());
                response_ret =	MSG_CONNECT_OK;
                break;

            } else if (strMT == "LogLevel") {
                sprintf(pBuf->extend, "%d", monitorGetLogLevel());
                printf("%d", monitorGetLogLevel());
                response_ret =	MSG_CONNECT_OK;
                break;
            }
            #endif



			if (strSTB == "")
				ret = MonitorAgent::GetInstance()->ReadAgent(MGMT_CONFIG_MT, strMT.c_str(), pBuf->extend, READ_BUFFER);
			else
				ret = MonitorAgent::GetInstance()->ReadAgent(MGMT_CONFIG_MT, strSTB.c_str(), pBuf->extend, READ_BUFFER);

			printf("extend = %s .....\n", pBuf->extend);

	        if (strlen(pBuf->extend) == 0)
				strcpy(pBuf->extend, "null");

			//if(ret == -1)
			//	response_ret = MSG_TIMEOUT_ERROR;
			//else
				response_ret =	MSG_CONNECT_OK;
		}



		//暂时对应
		if ((strcmp(m_szCmdInfo[1], "connecttype") == 0)
			&& (strcmp(pBuf->extend, "null") != 0)) {
			if (strcmp(pBuf->extend, "PPPoE") == 0)
				strcpy(pBuf->extend, "1");
			else if (strcmp(pBuf->extend, "DHCP") == 0)
				strcpy(pBuf->extend, "2");
			else if (strcmp(pBuf->extend, "Static") == 0)
				strcpy(pBuf->extend, "3");

		}else if (strcmp(m_szCmdInfo[1], "timeZone") == 0) {
			mid_tool_timezone2str(atoi(pBuf->extend), pBuf->extend);

		}else if (strcmp(m_szCmdInfo[1], "HDVideoStandard") == 0) {
		    if (strcmp(pBuf->extend, "0") == 0)
    			strcpy(pBuf->extend, "disable");
    		//else if ((strcmp(pBuf->extend, "1") == 0) || (strcmp(pBuf->extend, "7") == 0))
            else if (strcmp(pBuf->extend, "1") == 0)
    			strcpy(pBuf->extend, "1080i50Hz");
    		//else if ((strcmp(pBuf->extend, "2") == 0) || (strcmp(pBuf->extend, "8") == 0))
            else if (strcmp(pBuf->extend, "2") == 0)
    			strcpy(pBuf->extend, "1080i60Hz");
    		else if (strcmp(pBuf->extend, "3") == 0)
    			strcpy(pBuf->extend, "480i60Hz");
    		else if (strcmp(pBuf->extend, "4") == 0)
    			strcpy(pBuf->extend, "576i50Hz");
    		else if (strcmp(pBuf->extend, "5") == 0)
    			strcpy(pBuf->extend, "720p50Hz");
    		else if (strcmp(pBuf->extend, "6") == 0)
    			strcpy(pBuf->extend, "576i60Hz");
    		else if (strcmp(pBuf->extend, "7") == 0)
    			strcpy(pBuf->extend, "1080p60Hz");
    		else if (strcmp(pBuf->extend, "8") == 0)
    			strcpy(pBuf->extend, "576p50Hz");
            else if (strcmp(pBuf->extend, "9") == 0)
    			strcpy(pBuf->extend, "720p60Hz");
            else if (strcmp(pBuf->extend, "10") == 0)
    			strcpy(pBuf->extend, "1080p50Hz");
    		else
    			strcpy(pBuf->extend, "disable");


	    }else if (strcmp(m_szCmdInfo[1], "channelSwitchMode") == 0) {
	        if (strcmp(pBuf->extend, "0") == 0)
	            strcpy(pBuf->extend, "normal");
	        else if (strcmp(pBuf->extend, "1") == 0)
	            strcpy(pBuf->extend, "last picture");
	        else if (strcmp(pBuf->extend, "2") == 0)
	            strcpy(pBuf->extend, "smooth switch");

	    }else if (strcmp(m_szCmdInfo[1], "AspectRatio") == 0) {
			if (strcmp(pBuf->extend, "1") == 0)
				strcpy(pBuf->extend, "0");
			else if (strcmp(pBuf->extend, "2") == 0)
				strcpy(pBuf->extend, "1");
		}

    }while (0);

	sprintf(pBuf->buf, "%dread^%s", response_ret, m_szCmdInfo[1]);
	pBuf->len = 3 + strlen("read") + 1 + strlen(m_szCmdInfo[1]);

	printf("pBuf-buf = %s \n", pBuf->buf);
    printf("pBuf->len = %d \n", pBuf->len);
    printf("pBuf->extend len = %d \n", strlen(pBuf->extend));

    if (((strcmp(m_szCmdInfo[1], "ParasListMain") == 0)
        || (strcmp(m_szCmdInfo[1], "ParasListPip") == 0)
        || (strcmp(m_szCmdInfo[1], "Stream1ParaList") == 0)
        || (strcmp(m_szCmdInfo[1], "Stream2ParaList") == 0))
        && (strlen(pBuf->extend) <= 4)) {
        sprintf(pBuf->extend, "Cpu=unknown\nMem=unknown\nVideoCodec=unknown\nVideoResolution=unknown\n\
            VideoAspect=unknown\nPictureCode=unknown\nAudioCodec=unknown\nAudioBitRate=unknown\nAudioChannels=unknown\n\
            AudioSamplingRate=unknown\nPacketLost=unknown\nPacketDisorder=unknown\n\
            StreamDF=unknown\nTransportProtocol=unknown\nContinuityError=unknown\nSynchronisationError=unknown\n\
            EcmError=unknown\nDiffAvPlayTime=unknown\nVideoBufSize=unknown\nVideoUsedSize=unknown\nAudioBufSize=unknown\n\
            AudioUsedSize=unknown\nVideoDecoderError=unknown\nVideoDecoderDrop=unknown\nVideoDecoderUnderflow=unknown\n\
            VideoDecoderPtsError=unknown\nAudioDecoderError=unknown\nAudioDecoderDrop=unknown\nAudioDecoderUnderflow=unknown\n\
            AudioDecoderPtsError=unknown\n");

    }


    if(pBuf->extend != NULL) {
        sprintf(pBuf->buf + pBuf->len, "^");
        pBuf->len += 1;
    }

	return ret;
}


/**  为保持与原来时区机制兼容并支持半时区。lh 2010-6-19
**    采用半时区* 4 +100的方式来记录，如果时区大于52 认为是半时区
***/
static int mid_tool_timezone2str(int pTimezone, char* buf)
{
    int hour, min;
    int  tmp1;

    strcpy(buf, "UTC ");
    buf += 4;

    if(pTimezone < 52) {
        if(pTimezone > 0) {
            hour = pTimezone;
            buf[0] = '+';
        } else {
            hour = (0 - pTimezone);
            buf[0] = '-';
        }
        min = 0;

        sprintf(buf + 1, "%02d:%02d", hour, min);
    } else {
        tmp1 = pTimezone - 100;
        if(tmp1 > 0) {
            hour = tmp1 / 4;
            min = (tmp1 % 4) * 15;
            buf[0] = '+';
        } else {
            hour = (0 - tmp1) / 4;
            min = ((0 - tmp1) % 4) * 15;
            buf[0] = '-';
        }
        sprintf(buf + 1, "%02d:%02d", hour, min);
    }
    return 0;
}

