#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/socket.h>
#include "ParseCmd.h"
#include "MonitorManager.h"
#include "ParseXML.h"
#include "MonitorTool.h"
#include "MonitorAgent.h"

int stopRealTimeLog();
void* MonitorUpgrade(void* pSocketPort);
void monitorSetLogOutPutType(int value);
void monitorSetLogType(int type);
void monitorSetLogLevel(int level);
void MonitorLog();


extern long long g_FileLength;
int   g_LogOutType;
int   g_LogType;
int   g_LogLevel;
FILE *g_pUpgradefile;
STBDebugInfoType g_debugInfoType = DEBUG_INFO_NONE;

ParseIoctl::ParseIoctl()
{
}

ParseIoctl::~ParseIoctl()
{
}

int ParseIoctl::Exec(moni_buf_t pBuf)
{
	int ret = 0;
	int response_ret = 0;
	int argc = 0;
	void *argv[2] = {NULL};
	pthread_t pupgrate;
	int flag_upgrade = 0;
    long long file_length = g_FileLength;

	//

	printf("ParseIoctl::Exec\n");
	do{
		if (strlen(m_szCmdInfo[1]) == 0 || strlen(m_szCmdInfo[1]) > 63){
			response_ret = MSG_COMMAND_FAIL_ERROR;
			break;

		} else if (strcmp(m_szCmdInfo[1], "reboot") == 0){
			MonitorAgent::GetInstance()->NotifyAgent(MGMT_MT_TOOL_REBOOT, 0, argv);
            response_ret = MSG_CONNECT_OK;
			break;

        } else if (strcmp(m_szCmdInfo[1], "upgrade") == 0) {
#ifdef ANDROID
			    g_pUpgradefile = fopen("/cache/update.zip", "w+");
			    if (g_pUpgradefile == NULL)
					printf("fopen error!\n");

				srand((unsigned)time(NULL));
				m_tUpgradeReceivePort = 40000 + (int)(rand() % 20001);
				pthread_create(&pupgrate, NULL, MonitorUpgrade, (void *)&m_tUpgradeReceivePort);

				usleep(10);
				pBuf->extend = (char *)malloc(8);
				memset(pBuf->extend, 0, 8);
				sprintf(pBuf->extend, "%d", m_tUpgradeReceivePort);
				printf("Upgrade port %d\n", m_tUpgradeReceivePort );
	            response_ret = MSG_CONNECT_OK;
				break;
#endif
			//是否强制升级
			//int force_upgrade = 0;
			argv[0] = (int *)malloc(sizeof(int));
			if (strcmp(m_szCmdInfo[2], "/f") == 0) {
				*((int *)argv[0]) = 1;
				MonitorAgent::GetInstance()->NotifyAgent(MGMT_MT_UPGRADE_SET_FORCE, 1, argv);
			}else {
				*((int *)argv[0]) = 0;
				MonitorAgent::GetInstance()->NotifyAgent(MGMT_MT_UPGRADE_SET_FORCE, 1, argv);
			}
			free(argv[0]);
			argv[0] = NULL;

			//发送文件头信息
			argc = 2;
			argv[0] = (char *)malloc(20);
			argv[1] = (int *)malloc(sizeof(int));
			strcpy((char *)argv[0], "");
			MonitorAgent::GetInstance()->NotifyAgent(MGMT_MT_UPGRADE_SET_UPHEADER, argc, argv);
			flag_upgrade = *(int *)argv[1];
			free(argv[0]);
			argv[0] = NULL;
			free(argv[1]);
			argv[1] = NULL;

			//获取升级文件保存位置句柄
			argc = 1;
			argv[0] = (FILE **)malloc(sizeof(FILE *));
			MonitorAgent::GetInstance()->NotifyAgent(MGMT_MT_UPGRADE_GET_DOWNHANDLE, argc, argv);
            g_pUpgradefile = *((FILE **)argv[0]);
            free(argv[0]);
			argv[0] = NULL;

			//close
			argc = 0;
			MonitorAgent::GetInstance()->NotifyAgent(MGMT_MT_UPGRADE_SET_CLOSEWORK, argc, argv);

			srand((unsigned)time(NULL));
			m_tUpgradeReceivePort = 40000 + (int)(rand() % 20001);
			pthread_create(&pupgrate, NULL, MonitorUpgrade, (void *)&m_tUpgradeReceivePort);
			pBuf->extend = (char *)malloc(8);
			memset(pBuf->extend, 0, 8);
			sprintf(pBuf->extend, "%d", m_tUpgradeReceivePort);
            response_ret = MSG_CONNECT_OK;

			free(argv[0]);
			argv[0] = NULL;
			free(argv[1]);
			argv[1] = NULL;
			break;


		} else if (strcmp(m_szCmdInfo[1], "restore_setting") == 0) {
            ret = MonitorAgent::GetInstance()->NotifyAgent(MGMT_CPE_CONFIG_FACTORYRESET, 0, NULL);
			ret = MonitorAgent::GetInstance()->NotifyAgent(MGMT_MT_TOOL_REBOOT, 0, NULL);
            if (ret != 0)
				return -1;
			response_ret = MSG_CONNECT_OK;
			break;

		} else if (strcmp(m_szCmdInfo[1], "set_log_out_type") == 0) {
		    #ifdef ANDROID
            monitorSetLogOutPutType(atoi(m_szCmdInfo[3]));
            MonitorLog();
            MonitorAgent::GetInstance()->WriteAgent(MGMT_CONFIG_MT, "SetLogOutType", m_szCmdInfo[3], READ_BUFFER);
            response_ret = MSG_CONNECT_OK;
            break;
            #endif

            g_LogOutType = atoi(m_szCmdInfo[3]);
			std::string strSTB = MonitorManager::cmdNameXML()->GetSTBPara("LogOutPutType");
			printf("strSTB = %s\n", strSTB.c_str());
			ret = MonitorAgent::GetInstance()->WriteAgent(MGMT_CONFIG_MT, strSTB.c_str(), m_szCmdInfo[3], READ_BUFFER);
			response_ret = MSG_CONNECT_OK;
			break;

		} else if (strcmp(m_szCmdInfo[1], "set_log_type") == 0) {
		    #ifdef ANDROID
            monitorSetLogType(atoi(m_szCmdInfo[3]));
            MonitorLog();
            MonitorAgent::GetInstance()->WriteAgent(MGMT_CONFIG_MT, "SetLogType", m_szCmdInfo[3], READ_BUFFER);
            response_ret = MSG_CONNECT_OK;
            break;
            #endif
            g_LogType = atoi(m_szCmdInfo[3]);
			std::string strSTB = MonitorManager::cmdNameXML()->GetSTBPara("LogType");
			printf("strSTB = %s\n", strSTB.c_str());
			ret = MonitorAgent::GetInstance()->WriteAgent(MGMT_CONFIG_MT, strSTB.c_str(), m_szCmdInfo[3], READ_BUFFER);
			response_ret = MSG_CONNECT_OK;
			break;

		} else if (strcmp(m_szCmdInfo[1], "set_log_level") == 0) {
		    #ifdef ANDROID
            monitorSetLogLevel(atoi(m_szCmdInfo[3]));
            MonitorLog();
            MonitorAgent::GetInstance()->WriteAgent(MGMT_CONFIG_MT, "SetLogLevel", m_szCmdInfo[3], READ_BUFFER);
            response_ret = MSG_CONNECT_OK;
            break;
            #endif
            g_LogLevel = atoi(m_szCmdInfo[3]);
			std::string strSTB = MonitorManager::cmdNameXML()->GetSTBPara("LogLevel");
			printf("strSTB = %s\n", strSTB.c_str());
			ret = MonitorAgent::GetInstance()->WriteAgent(MGMT_CONFIG_MT, strSTB.c_str(), m_szCmdInfo[3], READ_BUFFER);
			response_ret = MSG_CONNECT_OK;
			break;

		} else if (strcmp(m_szCmdInfo[1], "collect_stbStatus") == 0) {
			argv[0] = (int *)malloc(sizeof(int));
			argv[1] = (char *)malloc(100);
			printf("1111xuke \n");
			//collect();
			fflush(stdin);//temp
			fflush(stdout);//temp
		    //MonitorAgent::GetInstance()->NotifyAgent(MGMT_MT_GET_COLLECT_FILEPATH, 2, argv);
            response_ret = MSG_CONNECT_OK;
			break;

        } else if (strcmp(m_szCmdInfo[1], "remotepcap") == 0) {
            monitorRemoteCapture(pBuf, 24 + strlen("ioctl") + 1 + strlen("remotepcap") + 1);
            break;

        } else if (strcmp(m_szCmdInfo[1], "starStartupInfo") == 0){
            printf("write start\n");
            ret = MonitorAgent::GetInstance()->WriteAgent(MGMT_CONFIG_MT, (const char *)"StartupCaptured", (char *)"1", 2);
			response_ret = MSG_CONNECT_OK;

            break;

        } else if (strcmp(m_szCmdInfo[1], "stopStartupInfo") == 0){
            printf("write stop\n");
            ret = MonitorAgent::GetInstance()->WriteAgent(MGMT_CONFIG_MT, (const char *)"StartupCaptured", (char *)"0", 2);
            monitorStopStartupInfo();
            response_ret = MSG_CONNECT_OK;
            break;

        } else if (strcmp(m_szCmdInfo[1], "UploadStartupInfo") == 0){
            char *strAddr = pBuf->buf + 30 + strlen("UploadStartupInfo") + 6;
            printf("write addr %s\n", strAddr);
            ret = MonitorAgent::GetInstance()->WriteAgent(MGMT_CONFIG_MT, (const char *)"StartupUploadAddr", strAddr, strlen(strAddr) + 1);
            monitorUploadStartupInfo(strAddr);
            response_ret = MSG_CONNECT_OK;
            break;

        } else if (strcmp(m_szCmdInfo[1], "startDebugInfo") == 0){
            printf("g_debugInfoType = %d\n", g_debugInfoType);
            if (DEBUG_INFO_NONE != g_debugInfoType)
                return -1;

            MonitorAgent::GetInstance()->WriteAgent(MGMT_CONFIG_MT, (const char *)"StartDebugInfo", "", 1);
            if (pBuf == NULL) {
                printf("in dfx\n");
                g_debugInfoType = DEBUG_INFO_DFX;
                monitorStartDebugInfo(NULL);

            } else if (pBuf != NULL) {
                printf("in tool\n");
                g_debugInfoType = DEBUG_INFO_TOOL;
                monitorStartDebugInfo(pBuf->buf + 30 + strlen("startDebugInfo") + 1);
            }

            response_ret = MSG_CONNECT_OK;
            break;

        } else if (strcmp(m_szCmdInfo[1], "stopDebugInfo") == 0){
            MonitorAgent::GetInstance()->WriteAgent(MGMT_CONFIG_MT, (const char *)"StopDebugInfo", "", 1);
            if ((pBuf == NULL) && (DEBUG_INFO_DFX == g_debugInfoType)) {
                printf("stop dfx\n");
                monitorStopDebugInfo(NULL);
                g_debugInfoType = DEBUG_INFO_NONE;

            } else if ((pBuf != NULL) && (DEBUG_INFO_TOOL == g_debugInfoType)) {
                printf("stop tool\n");
                monitorStopDebugInfo(pBuf->buf + 30 + strlen("stopDebugInfo") + 1);
                g_debugInfoType = DEBUG_INFO_NONE;
            }

            response_ret = MSG_CONNECT_OK;
            break;

        } else if (strcmp(m_szCmdInfo[1], "UploadDebugInfo") == 0){
            monitorUploadDebugInfo(pBuf->buf + 30 + strlen("UploadDebugInfo") + 1);
            response_ret = MSG_CONNECT_OK;
            break;

        } else if (strcmp(m_szCmdInfo[1], "getAllDebugInfo") == 0){
            response_ret = MSG_CONNECT_OK;
            break;

        } else {
        }


		if (ret != 0)
			return -1;

		response_ret = MSG_CONNECT_OK;
	}while(0);

    sprintf(pBuf->buf, "%dioctl^%s", response_ret, m_szCmdInfo[1]);
    pBuf->len = 3 + strlen("ioctl") + 1 + strlen(m_szCmdInfo[1]);

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



void* ParseIoctl::MonitorUpgrade(void* pSocketPort)
{
    int servernew_fd = -1;
	int lastPercent = 0;
    int monitornew_port = 0;
    int recv_len = 0;
    int temp_len = 0;
    int error = 0;
    long long file_length = 0;
	int wlen = 0;
    socklen_t len = 0;
    int clientnew_fd = -1;
    long long write_len = 0;
    fd_set readnew_fd, errornew_fd;
    struct sockaddr_in tcp_addr;
    unsigned char* bufferHead = 0;
    struct moni_buf tMonitorMsg;

    memset(&tMonitorMsg, 0, sizeof(struct moni_buf));
    bufferHead = (unsigned char *)malloc(1024 * 1024 * 2);
    monitornew_port = *((int *)pSocketPort);
	recv_len = 1024 * 1024 * 2;
    servernew_fd = socket(PF_INET, SOCK_STREAM, 0);
    if(servernew_fd < 0) {
        printf("server sockect creat error!\n");
        sprintf(tMonitorMsg.buf, "303ioctl^upgrate");
        tMonitorMsg.len = strlen(tMonitorMsg.buf);
        goto ERR;
    }

    memset(&tcp_addr, 0, sizeof(tcp_addr));
    tcp_addr.sin_family = AF_INET;
    tcp_addr.sin_port = htons(monitornew_port);
    tcp_addr.sin_addr.s_addr = INADDR_ANY;
    if(bind(servernew_fd, (struct sockaddr*)&tcp_addr, sizeof(tcp_addr)) < 0) {
        printf("server socket bind");
        sprintf(tMonitorMsg.buf, "303ioctl^upgrate");
        tMonitorMsg.len = strlen(tMonitorMsg.buf);
        goto ERR;
    }
    if(listen(servernew_fd, 1) < 0) {
        printf("server socket listen");
        sprintf(tMonitorMsg.buf, "303ioctl^upgrate");
        tMonitorMsg.len = strlen(tMonitorMsg.buf);
        goto ERR;
    }
    //monitorUpgradeDownloadStart();
    while(1) {
        len = sizeof(tcp_addr);
        clientnew_fd = accept(servernew_fd, (struct sockaddr*)&tcp_addr, &len);
        if(clientnew_fd < 0) {
            printf("server accept");
            continue;
        }
        printf("server accept success\n");

        len = 1;
        if(setsockopt(clientnew_fd, SOL_SOCKET, SO_KEEPALIVE, &len, sizeof(len)) < 0) {
            printf("setsockopt SO_KEEPALIVE error!!!\n");
            goto ERR;
        }

        //
		file_length = g_FileLength;

        //file_length = monitorUpgradeGetLength();
        while(1)  {
            struct timeval tm = {30, 0};
            FD_ZERO(&readnew_fd);
            FD_ZERO(&errornew_fd);
            FD_SET(clientnew_fd, &readnew_fd);
            FD_SET(clientnew_fd, &errornew_fd);
            if(select(clientnew_fd + 1, &readnew_fd, (fd_set *) NULL, &errornew_fd, &tm) <= 0) {
                //printf("Error on tcp select request: %s", strerror(errno));
                goto END;
            }
            if(FD_ISSET(clientnew_fd, &readnew_fd) || FD_ISSET(clientnew_fd, &errornew_fd)) {
                error = -1;
                len = sizeof(error);
                getsockopt(clientnew_fd, SOL_SOCKET, SO_ERROR, (void*)&error, &len);
                if(error != 0) {
                    //printf("%s,client's socket timeout!!!\n", strerror(errno));
                    close(clientnew_fd);
                    clientnew_fd = -1;
                    //monitorUpgradeReceiveError();
                    break;
                }

				temp_len = recv(clientnew_fd, bufferHead, recv_len, MSG_NOSIGNAL);
			    if (temp_len < 0)
				    break;
				wlen = ::fwrite(bufferHead, temp_len, 1, g_pUpgradefile);

				write_len += temp_len;

                //发送进度
                int percent = (write_len * 100) / file_length;
				if (percent != lastPercent) {
					void *argv[2] = {NULL};
					#ifdef ANDROID
					argv[0] = &write_len;
                    argv[1] = &file_length;
                    #else
                    argv[0] = &percent;
                    #endif
					MonitorAgent::GetInstance()->NotifyAgent(MGMT_MT_UPGRADE_SET_DOWNLOAD_PER, 1, argv);
					lastPercent = percent;
				}

				if(write_len >= file_length)
                    goto END;
				#if 0
                monitorUpgradeSubmitBuffer(bufferHead, temp_len);

				#endif
            }
        }
    }

END:
	fclose(g_pUpgradefile);
	sync();
	free(bufferHead);

    //printf("Upgrade file lenght(%d) download lenght(%d)\n", file_length, write_len);
    if (file_length != write_len) {
        sprintf(tMonitorMsg.buf, "303ioctl^upgrate");
        tMonitorMsg.len = strlen(tMonitorMsg.buf);
        //
        MonitorAgent::GetInstance()->NotifyAgent(MGMT_HYBROAD_UPGRADE_EXIT, 0, NULL);
        //monitorUpgradeReceiveError();
    } else {
        #ifdef ANDROID
        //send burn percent 95
		sprintf(tMonitorMsg.buf, "302ioctl^upgrate^%d", 100);
	    tMonitorMsg.len = strlen(tMonitorMsg.buf);
	    MonitorManager::GetInstance()->monitor_cmd_response(&tMonitorMsg);

        close(clientnew_fd);
        clientnew_fd = -1;
        close(servernew_fd);
        servernew_fd = -1;

        //burn
        MonitorAgent::GetInstance()->NotifyAgent(MGMT_MT_UPGRADE_SET_BURN_START, 0, NULL);
        pthread_exit((void *)"update ok");
		#endif

        //burn
        MonitorAgent::GetInstance()->NotifyAgent(MGMT_MT_UPGRADE_SET_BURN_START, 0, NULL);

        //get burn percent
        void *argv[2] = {NULL};
		int burn_percent = 0;
	    argv[0] = &burn_percent;
		while (1) {
            usleep(500);
			MonitorAgent::GetInstance()->NotifyAgent(MGMT_MT_UPGRADE_GET_BURN_PROCESS, 1, argv);
			while(burn_percent <= 95 && burn_percent >= 0) {
				MonitorAgent::GetInstance()->NotifyAgent(MGMT_MT_UPGRADE_GET_BURN_PROCESS, 1, argv);
				printf("burn_percent = %d 111111111\n", burn_percent);
	        	sprintf(tMonitorMsg.buf, "301ioctl^upgrate^%d", burn_percent);
	        	tMonitorMsg.len = strlen(tMonitorMsg.buf);
	        	MonitorManager::GetInstance()->monitor_cmd_response(&tMonitorMsg);
	        	sleep(2);
    	    }
			if(burn_percent < 0){
                sprintf(tMonitorMsg.buf, "303ioctl^upgrate");
                tMonitorMsg.len = strlen(tMonitorMsg.buf);
				break;
            }else{
    	        sprintf(tMonitorMsg.buf, "302ioctl^upgrate^%d", burn_percent);
    	        tMonitorMsg.len = strlen(tMonitorMsg.buf);
				break;

            }
		}

	}
    printf("_ update ok!\n");

	MonitorManager::GetInstance()->monitor_cmd_response(&tMonitorMsg);
    close(clientnew_fd);
    clientnew_fd = -1;
    close(servernew_fd);
    servernew_fd = -1;
    pthread_exit((void *)"update ok");
ERR:
    MonitorManager::GetInstance()->monitor_cmd_response(&tMonitorMsg);
    //upgrade_flag = 0;
    free(bufferHead);
    close(servernew_fd);
    servernew_fd = -1;
    pthread_exit((void *)"update error");
    return NULL;
}


