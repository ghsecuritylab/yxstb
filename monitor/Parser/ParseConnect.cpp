#include "ParseCmd.h"
#include "MonitorPing.h"
#include "MonitorTraceroute.h"
#include "MonitorAgent.h"

#include <stdlib.h>


#define IP_HEAD_LEN 20
#define ICMP_LEN 8
#define BUFFER_SIZE 50 * 1024
#define PING_RETURN_NUM 10


ParseConnect::ParseConnect()
{
}

ParseConnect::~ParseConnect()
{
}


int ParseConnect::Exec(moni_buf_t pBuf)
{
	int ret = 0;
	int response_ret = 0;
	int argc = 0;
	void *argv[2] = {NULL};
	char playUrl[LARGE_URL_MAX_LEN + 4] = {0};
	HMW_MgmtMsgType eMsgType = MGMT_MESSAGE_NONE;
	//
	printf("ParseConnect::Exec\n");
	do{
		if (strlen(m_szCmdInfo[1]) == 0){
			response_ret = MSG_COMMAND_FAIL_ERROR;
			break;

		} else if (strcmp(m_szCmdInfo[1], "ping") == 0) {
		    monitorPingConnect(pBuf, 24 + strlen("connect") + 1 + strlen("ping") + 1);
		    return 2;

		} else if (strcmp(m_szCmdInfo[1], "traceroute") == 0) {
		    monitorTracerouteConnect(pBuf, 24 + strlen("connect") + 1 + strlen("traceroute") + 1);
		    return 2;

		} else if (strcmp(m_szCmdInfo[1], "play") == 0){
			if (strlen(m_szCmdInfo[3]) == 0) {
				sprintf(pBuf->buf, "601connect");
			    return -1;
			}

			if(!strncmp(m_szCmdInfo[3], "Channel", 7)) {
				char* pos = strstr(m_szCmdInfo[3], ":");
				if(!pos) {
					printf("channel is invaild\n");
					sprintf(pBuf->buf, "601connect");
					return -1;
				}

				//
				eMsgType = MGMT_MT_PLAYER_BY_CHANNO;
				argc = 2;
				argv[0] = (int *)malloc(sizeof(int));
				argv[1] = (int *)malloc(sizeof(int));
				*(int *)argv[0] =  atoi(pos + 1);
                *(int *)argv[1] = 0;


			} else {
				sprintf(playUrl, "%s", m_szCmdInfo[3]);

				eMsgType = MGMT_MT_PLAYER_BY_URL;
				argc = 2;
				argv[0] = (char *)malloc(sizeof(char) * sizeof(playUrl));
				argv[1] = (int *)malloc(sizeof(int));
				strcpy((char *)argv[0], playUrl);
				((char *)argv[0])[strlen(playUrl)] = '\0';

			}

		}

		else if (strcmp(m_szCmdInfo[1], "stop") == 0){
			eMsgType = MGMT_MT_PLAYER_STOP;
			argc = 1;
			argv[0] = (int *)malloc(sizeof(int));

		}

		else if (strcmp(m_szCmdInfo[1], "pause") == 0){
			eMsgType = MGMT_MT_PLAYER_MPCTRL;
			argc = 1;
			argv[0] = (char *)malloc(sizeof("KEY_PAUSE_PLAY"));
			strcpy((char *)argv[0], "KEY_PAUSE_PLAY");

		}


		else if (strcmp(m_szCmdInfo[1], "fast_forward") == 0){
			eMsgType = MGMT_MT_PLAYER_MPCTRL;//?
			argc = 1;
			argv[0] = (char *)malloc(sizeof("KEY_FAST_FORWARD"));
			strcpy((char *)argv[0], "KEY_FAST_FORWARD");

		}

		else if (strcmp(m_szCmdInfo[1], "fast_backward") == 0){
			eMsgType = MGMT_MT_PLAYER_MPCTRL;//?
			argc = 0;
			argv[0] = (char *)malloc(sizeof("KEY_FAST_BACK"));
			strcpy((char *)argv[0], "KEY_FAST_BACK");
		}

		ret = MonitorAgent::GetInstance()->NotifyAgent(eMsgType, argc, argv);

		//需要对argc[0] argc[1] 返回值返回
		if (strcmp(m_szCmdInfo[1], "play") == 0){
			if (argv[1] != NULL) {
                printf("ret = %d\n", *((int *)argv[1]));
                if (*(int *)argv[1] == 1)
                    response_ret = MSG_CONNECT_OK;
                else
			        response_ret = MSG_PARAMETER_ERROR;
			}
			else
				response_ret = MSG_PARAMETER_ERROR;

	    } else if (strcmp(m_szCmdInfo[1], "stop") == 0){
            if (argv[0] != NULL) {
				if (*(int *)argv[0] == 1)
                    response_ret = MSG_CONNECT_OK;
                else
			        response_ret = MSG_PARAMETER_ERROR;
            }
			else
				response_ret = MSG_PARAMETER_ERROR;

		} else
		    response_ret = MSG_PARAMETER_ERROR;

	}while(0);

    printf("%d...\n", response_ret);
    sprintf(pBuf->buf, "%dconnect^%s", response_ret, m_szCmdInfo[1]);
    pBuf->len = 3 + strlen("connect") + 1 + strlen(m_szCmdInfo[1]);

    if(pBuf->extend != NULL) {
        sprintf(pBuf->buf + pBuf->len, "^");
        pBuf->len += 1;
    }

	if (argv[0] != NULL){
		free(argv[0]);
		argv[0] = NULL;
	}
	if (argv[1] != NULL){
		free(argv[1]);
		argv[1] = NULL;
	}

	return 0;
}



int ParseConnect::SetPara(HMW_MgmtMsgType &eMsgType, int &argc, void **argv)
{
	char playUrl[LARGE_URL_MAX_LEN + 4] = {0};

    printf("connect para = %s \n", m_szCmdInfo[1]);

	if (argv == NULL || strlen(m_szCmdInfo[1]) == 0)
		return -1;

	if (strcmp(m_szCmdInfo[1], "play") == 0){
		if (strlen(m_szCmdInfo[3]) == 0)
			return -1;

		if(!strncmp(m_szCmdInfo[3], "Channel", 7)) {
			char* pos = strstr(m_szCmdInfo[3], ":");
			if(!pos) {
				printf("channel is invaild\n");
				return -1;
			}

			//
			eMsgType = MGMT_MT_PLAYER_BY_CHANNO;
			argc = 2;
			argv[0] = (int *)malloc(sizeof(int));
			argv[1] = (int *)malloc(sizeof(int));
			*(int *)argv[0] =  atoi(pos + 1);
			return 0;

		} else {
			sprintf(playUrl, "%s", m_szCmdInfo[3]);

			eMsgType = MGMT_MT_PLAYER_BY_URL;
			argc = 2;
			argv[0] = (char *)malloc(sizeof(char) * sizeof(playUrl));
			argv[1] = (int *)malloc(sizeof(int));
			strcpy((char *)argv[0], playUrl);
			((char *)argv[0])[strlen(playUrl)] = '\0';
			return 0;
		}
		//memcpy(g_monitorBuf.buf, buf->buf, 8);
		//sprintf(buf->buf, "200connect");

	}

	else if (strcmp(m_szCmdInfo[1], "stop") == 0){
		eMsgType = MGMT_MT_PLAYER_STOP;
		argc = 1;
		argv[0] = (int *)malloc(sizeof(int));
		return 0;
	}

	else if (strcmp(m_szCmdInfo[1], "pause") == 0){
		eMsgType = MGMT_MT_PLAYER_MPCTRL;
		argc = 1;
		argv[0] = (char *)malloc(sizeof("KEY_PAUSE_PLAY"));
		strcpy((char *)argv[0], "KEY_PAUSE_PLAY");
		return 0;
	}


	else if (strcmp(m_szCmdInfo[1], "fast_forward") == 0){
		eMsgType = MGMT_MT_PLAYER_MPCTRL;//?
		argc = 1;
		argv[0] = (char *)malloc(sizeof("KEY_FAST_FORWARD"));
		strcpy((char *)argv[0], "KEY_FAST_FORWARD");
		return 0;
	}

	else if (strcmp(m_szCmdInfo[1], "fast_backward") == 0){
		eMsgType = MGMT_MT_PLAYER_MPCTRL;//?
		argc = 0;
		argv[0] = (char *)malloc(sizeof("KEY_FAST_BACK"));
		strcpy((char *)argv[0], "KEY_FAST_BACK");
		return 0;
	}


	else
		return -1;

	return 0;
}

