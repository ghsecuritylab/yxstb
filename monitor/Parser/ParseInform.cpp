#include "ParseCmd.h"
#include "MonitorAgent.h"

#include <sys/vfs.h>
#include <dirent.h>
#include <stdlib.h>


extern"C" int mid_ntp_status(void);
extern"C" int NativeHandlerGetState();


long long g_FileLength = 0;


ParseInform::ParseInform()
{
}

ParseInform::~ParseInform()
{
}

int ParseInform::Exec(moni_buf_t pBuf)
{
	int ret = 0;
	int response_ret = 0;
	int argc = 0;
	void *argv[2] = {NULL};
	HMW_MgmtMsgType eMsgType = MGMT_MESSAGE_NONE;

	do{
	    // BOOT | STANDBY 启动过程中，NTP失败时仍可升级
    	// keyMode = NativeHandlerGetState();
        // if((0 == keyMode && mid_ntp_status() != 0) || (3 == keyMode)) {
            // printf("keyMode(%d), ntp status(%d)\n", keyMode, mid_ntp_status());
            // response_ret = MSG_COMMAND_UNDEFINE_ERROR;
            // break;
    	// }

		if (strlen(m_szCmdInfo[1]) == 0 || strlen(m_szCmdInfo[1]) > 63) {
			response_ret = MSG_COMMAND_FAIL_ERROR;
			// printf("The command para is error !\n");
			break;
		}

#if 0
        if (strcmp(m_szCmdInfo[1], "set_upgradelength") == 0) {
            // MGMT_MT_UPGRADE_GET_WORKSTAT
			int upgrade = 0;

			argv[0] = &upgrade;
            MonitorAgent::GetInstance()->NotifyAgent(MGMT_MT_UPGRADE_GET_WORKSTAT, 1, argv);
            if (upgrade != 1) {
                // printf("upgrade MGMT_MT_UPGRADE_GET_WORKSTAT error! \n");
				return -1;
			}

			//MGMT_MT_UPGRADE_SET_LENGTH
			int file_length = atoi(m_szCmdInfo[3]);

			argv[0] = &file_length;
            MonitorAgent::GetInstance()->NotifyAgent(MGMT_MT_UPGRADE_SET_LENGTH, 1, argv);
			///SetPara(eMsgType, argc, argv);
		}
#endif

		//pcap
		if (strcmp(m_szCmdInfo[1], "pcap_maxfilesize") == 0) {
            stb_pcap_maxfilesize(pBuf, 0);
            response_ret = MSG_CONNECT_OK;
			break;
		} else if (strcmp(m_szCmdInfo[1], "set_upgradelength") == 0){
#ifdef ANDROID
			g_FileLength = atoi(m_szCmdInfo[3]);
            response_ret = MSG_CONNECT_OK;
			break;
#endif
			int upgrade = 0;
			argv[0] = &upgrade;
			MonitorAgent::GetInstance()->NotifyAgent(MGMT_MT_UPGRADE_GET_WORKSTAT, 1, argv);
			if (upgrade != 1) {
				printf("upgrade MGMT_MT_UPGRADE_GET_WORKSTAT error! \n");
				return -1;
			}

			//MGMT_MT_UPGRADE_SET_LENGTH
			eMsgType = MGMT_MT_UPGRADE_SET_LENGTH;
			argc = 1;
			argv[0] = (int *)malloc(sizeof(int));
			*(int *)argv[0] = atoi(m_szCmdInfo[3]);
			g_FileLength = *(int *)argv[0];
		} else if (strcmp(m_szCmdInfo[1], "get_upgradeprecent") == 0) { // MGMT_MT_UPGRADE_GET_BURN_PROCESS
			eMsgType = MGMT_MT_UPGRADE_GET_BURN_PROCESS;
			argc = 1;
			argv[0] = (int *)malloc(sizeof(int));
		}
		// SetPara(eMsgType, argc, argv);

		ret = MonitorAgent::GetInstance()->NotifyAgent(eMsgType, argc, argv);
		if (ret != 0)
			return -1;
		response_ret = MSG_CONNECT_OK;
	}while(0);

    sprintf(pBuf->buf, "%dinform^%s", response_ret, m_szCmdInfo[1]);
    pBuf->len = 3 + strlen("inform") + 1 + strlen(m_szCmdInfo[1]);

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


int ParseInform::SetPara(HMW_MgmtMsgType &eMsgType, int &argc, void **argv)
{
	if (argv == NULL)
		return -1;

	printf("inform para = %s \n", m_szCmdInfo[1]);
	//MGMT_MT_UPGRADE_SET_LENGTH
	if (strcmp(m_szCmdInfo[1], "set_upgradelength") == 0) {
		//MGMT_MT_UPGRADE_GET_WORKSTAT
		int upgrade = 0;

		argv[0] = &upgrade;
		MonitorAgent::GetInstance()->NotifyAgent(MGMT_MT_UPGRADE_GET_WORKSTAT, 1, argv);
		if (upgrade != 1) {
			printf("upgrade MGMT_MT_UPGRADE_GET_WORKSTAT error! \n");
			return -1;
		}

		//MGMT_MT_UPGRADE_SET_LENGTH
		eMsgType = MGMT_MT_UPGRADE_SET_LENGTH;
		argc = 1;
		argv[0] = (int *)malloc(sizeof(int));
		*(int *)argv[0] = atoi(m_szCmdInfo[3]);
		g_FileLength = *(int *)argv[0];
	} else if (strcmp(m_szCmdInfo[1], "get_upgradeprecent") == 0) { // MGMT_MT_UPGRADE_GET_BURN_PROCESS
		eMsgType = MGMT_MT_UPGRADE_GET_BURN_PROCESS;
		argc = 1;
		argv[0] = (int *)malloc(sizeof(int));
	} else if (strcmp(m_szCmdInfo[1], "pcap_maxfilesize") == 0) {
		// eMsgType =
		argc = 0;
		// *argv = NULL;
	} else if (strcmp(m_szCmdInfo[1], "get_pcapfilesize") == 0) {
		// eMsgType =
		argc = 0;
		// *argv = NULL;
	} else if (strcmp(m_szCmdInfo[1], "get_capfileuploadszie") == 0) {
		// eMsgType =
		argc = 0;
		// *argv = NULL;
	} else
		return -1;
	return 0;
}

int ParseInform::stb_pcap_maxfilesize(moni_buf_t buf, int len)
{
    char maxfilesize[64] = {0};

    sprintf(maxfilesize, "%lu^null", GetFreeVarSize() / 2);
    if (buf->extend) {
        free(buf->extend);
    }
    buf->extend = (char *)malloc(strlen(maxfilesize)+1);
    if (!buf->extend) {
        printf("malloc failed!\n");
        return -1;
    }
    strcpy(buf->extend, maxfilesize);
    return 1;
}

unsigned long ParseInform::GetFreeDiskBytes(const char * path)
{
    struct statfs buf;

    if (statfs(path, &buf) != 0) {
        perror("statfs");
        return 0;
    }
    return (unsigned long)buf.f_bfree * 4 * 1024;
}

unsigned long ParseInform::GetFreeVarSize(void)
{
    return GetFreeDiskBytes("/var");
}

