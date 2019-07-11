#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <openssl/aes.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "MonitorManager.h"
#include "MonitorTool.h"
#include "MonitorTimer.h"
#include "MonitorAgent.h"


int monitorGetPingMagic( void );
void monitorSetPingRun( void );
int monitorStbMonitorPing (char *url,moni_buf_t buf,void* stb_func);
void monitorSetTraceRouteRun( void );
int monitorTraceroute( char *url, moni_buf_t buf, void* stb_func );
void monitorSetPingStopMagic( void );
void* monitorLoopGetInfo(void *param);


msgq_t g_Msg;

//ParseXML        *pParseXML;
unsigned char   g_monitor_aes_keys[17];
int             initialize_state;
int             already_connected; //stb_monitor is connected or not
moni_buf        g_monitorBuf;
char            stbMonitorIP[32];
int             sfdTrriger = -1;
int             MonitorConnectFlag = 0;

int  	        MonitorManager::g_MsgFd = -1;
int 	        MonitorManager::upgrade_flag = 0;
int 	        MonitorManager::client_fd = -1;
int 	        MonitorManager::server_fd = -1;
moni_buf 		MonitorManager::local_buf;
int             MonitorManager::s_stopFlag = 0;
ParseXML*       MonitorManager::s_cmdNameXML = NULL;

#ifdef ANDROID
//extern ParseXML *pParseXML;
extern int DoRead(HMW_MgmtConfig_SRC eSrcType, const char* szParm, char* pBuf, int iLen);
extern int DoWrite(HMW_MgmtConfig_SRC eSrcType, const char* szParm, char* pBuf, int iLen);
extern int DoNotify(HMW_MgmtMsgType eMsgType, unsigned int argc, void * argv[]);
extern int DoLogExp(const char* pszFile, int lLine, const char* pszFunc, int lThreadId, int enLogType, int enLogLevel, const char* pszModule, const char* format, ...);
#else
extern "C" int mgmtReadCallBack(HMW_MgmtConfig_SRC eSrcType, const char* szParm, char* pBuf, int iLen);
extern "C" int mgmtWriteCallback(HMW_MgmtConfig_SRC eSrcType, const char* szParm, char* pBuf, int iLen);
extern "C" int mgmtNotifyCallback(HMW_MgmtMsgType eMsgType, unsigned int argc, void * argv[]);
#endif

static MonitorManager *g_monitorManager = NULL;

MonitorManager::MonitorManager()
{
}

MonitorManager::~MonitorManager()
{
}

MonitorManager* MonitorManager::GetInstance()
{
    if (!g_monitorManager)
        g_monitorManager = new MonitorManager();
	return g_monitorManager;
}

ParseXML* MonitorManager::cmdNameXML()
{
	return s_cmdNameXML;
}

int MonitorManager::cmdNameXMLinit(char *xmlPath)
{
    if (!s_cmdNameXML)
        s_cmdNameXML = new ParseXML(xmlPath);
    return 0;
}


int MonitorManager::MonitorInit()
{
    pthread_t threadID = 0;
	STB_MONITOR_LOG("monitorInit ...\n");
	s_stopFlag = 0;

    STB_MONITOR_LOG("monitorTimerInit...\n");
	monitorTimerInit();

    STB_MONITOR_LOG("get StartupInfo....\n");
    monitorGetStartupInfo();

	if(MonitorDiaginit() == -1) {
        printf("Stb monitor diagnoses create thread error\n");
        return -1;
    }

    if (-1 !=server_fd)
        MonitorShutdown();

	pthread_t start_pthread = 0;
	pthread_attr_t tattr;
	pthread_attr_init(&tattr);
	pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
	pthread_create(&start_pthread, &tattr, MonitorStart, NULL);
	return 0;
}

void* MonitorManager::MonitorStart(void *arg)
{
    struct sockaddr_in tcp_addr;
    socklen_t len = 0;
	int opt = 1;

    server_fd = socket(PF_INET, SOCK_STREAM, 0);
    if(server_fd < 0) {
        printf("Server sockect creat\n");
        return NULL;
    }

    len = sizeof(opt);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, len);

    memset(&tcp_addr, 0, sizeof(tcp_addr));
    tcp_addr.sin_family = AF_INET;
    tcp_addr.sin_port 	= htons(MONITOR_PORT);
    tcp_addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(server_fd, (struct sockaddr*)&tcp_addr, sizeof(tcp_addr)) < 0) {
        printf("Server socket bind error\n");
        close(server_fd);
        server_fd = -1;
        return NULL;
    }

    if(listen(server_fd, 1) < 0) {
        printf("Server socket listen error\n");
        close(server_fd);
        server_fd = -1;
        return NULL;
    }

    memset(&local_buf, 0, sizeof(local_buf));

    while(!s_stopFlag) {
        len = sizeof(tcp_addr);
        client_fd = accept(server_fd, (struct sockaddr*)&tcp_addr, &len);
        if(client_fd < 0) {
            printf("Server accept error\n");
            if (EBADF == errno)
                break;
            continue;
        }

        printf("Server accept success\n");
        len = 1;
        if(setsockopt(client_fd, SOL_SOCKET, SO_KEEPALIVE, &len, sizeof(len)) < 0)
            printf("setsockopt SO_KEEPALIVE error!!!\n");

        printf("setsockopt SO_KEEPALIVE success!!!\n");
        strcpy(stbMonitorIP, (const char *)inet_ntoa(tcp_addr.sin_addr));
        printf("Monitor ip is %s\n", stbMonitorIP);
		//s_stopFlag = 0;
        pthread_t stbmonitor_pthread = 0;
        pthread_attr_t tattr;
        pthread_attr_init(&tattr);
        pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
        pthread_create(&stbmonitor_pthread, &tattr, monitor_pthread_func, NULL);
    }
    return NULL;
}

int MonitorManager::MonitorClose()
{
    close(client_fd);
    client_fd = -1;
    return 0;
}

void MonitorManager::MonitorShutdown()
{
    if (-1 != client_fd)
        close(client_fd);

    client_fd = -1;

    if (-1 != server_fd) {
        shutdown(server_fd, SHUT_RDWR);
        close(server_fd);
    }
    server_fd = -1;
	//Ω· ¯Ω” ’œﬂ≥Ã
	s_stopFlag = 1;
}

int MonitorManager::MonitorDiaginit()
{
	static int isInited = 0;
    pthread_t diagTask_pid;

    if(isInited == 1)
        return 0;

    g_Msg = (msgq_t)malloc(sizeof(struct msgq));
	if (g_Msg == NULL)
		printf("malloc error\n");
	if (pipe(g_Msg->fds) == -1) {
        free(g_Msg);
		printf("pipe error\n");
	}
    fcntl(g_Msg->fds[0], F_SETFL, fcntl(g_Msg->fds[0], F_GETFL) | O_NONBLOCK);
	fcntl(g_Msg->fds[1], F_SETFL, fcntl(g_Msg->fds[1], F_GETFL) | O_NONBLOCK);
	g_Msg->size = sizeof(DIAG_MSG);
    g_MsgFd = g_Msg->fds[0];


    if(pthread_create(&diagTask_pid, NULL, MonitorManager::monitor_diagTask, NULL) != 0) {
        printf("Monitor diagnose create thread error\n");
        return -1;
    }

    isInited = 1;
    return 0;

}

void* MonitorManager::monitor_diagTask(void *arg)
{
	struct timeval tv;
    DIAG_MSG msg;
    fd_set rfds;
    int ret = -1;


    while(!s_stopFlag) {
        FD_ZERO(&rfds);
        FD_SET(g_MsgFd, &rfds);
        tv.tv_sec = 3 * 60 * 60;
        tv.tv_usec = 0;

        ret = select(g_MsgFd + 1, &rfds, NULL, NULL, &tv);
        if((ret > 0) && FD_ISSET(g_MsgFd, &rfds)) {
            memset(&msg, 0, sizeof(msg));
            if(read(g_MsgFd, (char *)&msg, g_Msg->size) > 0) {
                printf("Diagnose command: cmd(%d), length(%d), url(%s), buf(%s)\n", msg.cmd, msg.length, msg.url, msg.buf.buf);
                GetInstance()->monitor_diagProc(&msg);
                char cmd[][20] = {"ping","traceroute"};
                sprintf(msg.buf.buf, "200connect^%s", cmd[msg.cmd]);
                GetInstance()->monitor_cmd_response(&(msg.buf));
            }
        }
    }
    return NULL;
}

void MonitorManager::monitor_diagProc(DIAG_MSG *msg)
{
    static int magic = 0;

    switch(msg->cmd) {
    case PING_START:
		printf("PING_START\n");
        if(msg->length != -1) {
            magic = monitorGetPingMagic();
            signal(SIGALRM, (void (*)(int))monitorSetPingStopMagic);
			alarm(msg->length);
        } else {
            printf("length is -1, all time ping\n");
        }
        monitorSetPingRun();
        monitorStbMonitorPing(msg->url, &(msg->buf), msg->func);
        break;
    case TRACEROUTE_START:
        monitorSetTraceRouteRun();
        monitorTraceroute(msg->url, &(msg->buf), msg->func);
        break;
    default:
        printf("cmd error! cmd is %d\n", msg->cmd);
        break;
    }
}

int MonitorManager::msgq_put(msgq_t msgq, char *msg, int sec)
{
	int ret;
	fd_set wset;
	struct timeval tv;

    printf("msgq_put()\n");
	if(msgq == NULL || sec < 0)
		printf("msgq = %p, sec = %d\n", msgq, sec);

	tv.tv_sec = sec;
	tv.tv_usec = 0;

	FD_ZERO(&wset);
	FD_SET( msgq->fds[1], &wset);

	ret = select(msgq->fds[1] + 1, NULL, &wset, NULL, &tv);
	if(ret <= 0)
		printf("select ret = %d\n", ret);

	ret = write(msgq->fds[1], msg, msgq->size);
	if (ret != msgq->size)
		printf("ret = %d, msgq->size = %d\n", ret, msgq->size);

	return 0;
}

int MonitorManager::trriggerConnect(char *ip)
{
    struct timeval tv = {3, 0};
    int flag = 0;
    int ret = 0;
    int k = 1;
    fd_set rfd;
    fd_set wfd;
    fd_set efd;

    pthread_t stbmonitor_pthread = 0;
    pthread_attr_t tattr;
    pthread_attr_init(&tattr);
    pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
    struct sockaddr_in dest;
    int len = sizeof(dest);
    if(!ip){
        printf("IP is null!\n");
        return -1;
    }

    FD_ZERO(&rfd);
    FD_ZERO(&wfd);
    FD_ZERO(&efd);
    memset(&dest, 0, sizeof(struct sockaddr_in));
    if(-1 != client_fd){
        close(client_fd);
        client_fd = -1;
    }
    client_fd = socket(PF_INET, SOCK_STREAM, 0);
    if(-1 == client_fd){
        printf("socket failed:%s\n", strerror(errno));
        return -1;
    }

    FD_SET(client_fd, &rfd);
    FD_SET(client_fd, &wfd);
    FD_SET(client_fd, &efd);
    flag = fcntl(client_fd, F_GETFL, 0);
    fcntl(client_fd, F_SETFL, flag | O_NONBLOCK);

    dest.sin_family = AF_INET;
    dest.sin_port = htons(MONITOR_PORT);
    dest.sin_addr.s_addr = inet_addr(ip);
    strcpy(stbMonitorIP,  ip);
    ret = connect(client_fd, (struct sockaddr *)&dest, sizeof(struct sockaddr));
    if(-1 == ret){
        printf("connect no success:%s\n", strerror(errno));
        ret = select(client_fd+1, &rfd, &wfd, &efd, &tv);
        if(ret >= 0){
            if(getpeername(client_fd, (struct sockaddr *)&dest, (socklen_t *)&len)){
                close(client_fd);
                client_fd = -1;
                return -1;
            }
        }else{
            close(client_fd);
            client_fd = -1;
            return -1;
        }
    }

    setsockopt(client_fd, SOL_SOCKET, SO_KEEPALIVE, (void*)&k , sizeof(k));
    pthread_create(&stbmonitor_pthread, &tattr, monitor_pthread_func, NULL);
    //while(-1 != client_fd);
    return 0;
}

void *MonitorManager::monitor_pthread_func(void *arg)
{
    int ret = -1;
    fd_set read_fd;
    fd_set error_fd;
    struct timeval tv = {30, 0};
    int timeoutFlag = 0;
    socklen_t len = 0;
    int connected_fd = -1;

    if(client_fd < 0) {
        printf("Client fd is error %d\n", client_fd);
        close(client_fd);
        client_fd = -1;
        return NULL;
    }
    connected_fd = client_fd;
    printf("monitor_pthread_func\n");

    while(!s_stopFlag) {
        FD_ZERO(&read_fd);
        FD_ZERO(&error_fd);
        FD_SET(connected_fd, &read_fd);
        FD_SET(connected_fd, &error_fd);
        tv.tv_sec = 60;

        if (MonitorConnectFlag == 0) {
            printf("monitor switch is off!\n");
            break;
        }

        ret = select(connected_fd + 1, &read_fd, NULL, &error_fd, &tv);
        if(ret < 0) {
            printf("%s,select's error!!!\n", strerror(errno));
            close(sfdTrriger);
            sfdTrriger = -1;
            close(connected_fd);
            connected_fd = -1;
            break;
        }

        if(client_fd != connected_fd) {
            printf("Other stbmonitor connet!!!\n");
            close(sfdTrriger);
            sfdTrriger = -1;
            close(connected_fd);
            connected_fd = -1;
            break;
        }
        if(ret == 0) {
            timeoutFlag++;
            if(timeoutFlag == 15)	{
                printf("WARNING:the last connect is 15 minutes \n");
                timeoutFlag = 0;
                close(connected_fd);
                connected_fd = -1;
                close(sfdTrriger);
                sfdTrriger = -1;
                break;
            }
            continue;
        }

        timeoutFlag = 0;
        if(client_fd < 0) {
            printf("Client fd %d\n", client_fd);
            break;
        }
        if(FD_ISSET(connected_fd, &error_fd)) {
            printf("%s,client's socekt error!!!\n", strerror(errno));
            close(connected_fd);
            connected_fd = -1;
            break;
        }
        if(FD_ISSET(connected_fd, &read_fd)) {
            ret = -1;
            len = sizeof(ret);
            getsockopt(connected_fd, SOL_SOCKET, SO_ERROR, (void*)&ret, &len);

            if(ret != 0) {
                printf("%s,client's socket timeout!!!\n", strerror(errno));
                close(connected_fd);
                connected_fd = -1;
                break;
            }
            memset(local_buf.buf, 0, sizeof(local_buf.buf));
            local_buf.len = 0;

            ret = GetInstance()->monitor_cmd_recv_cmd(&local_buf);
            if(ret == 0) {
                printf("%s,client's socket close!!!\n", strerror(errno));
                close(connected_fd);
                connected_fd = -1;
                already_connected = 0;
                //xuke sendMessageToEPGBrowser(MessageType_Prompt, 0, 0, 0);
                break;
            }
            if(ret == -2) {  // √â√Ω¬º¬∂√ê¬¥√é√Ñ¬º√æ√ä¬ß¬∞√ú
                printf("Upgrade file write error\n");
                sprintf(local_buf.buf, "401ioctl^upgrate");
                local_buf.len = strlen("401ioctl^upgrate");
                GetInstance()->monitor_cmd_response(&local_buf);
                continue;
            }

            len = local_buf.len;
            *(local_buf.buf + len) = '\0';

            //if(len < 255)
                //printf("recv buff(%s)\n", local_buf.buf);

            int userPwdCheckRet = MonitorCmd::GetInstance()->CmdSort(&local_buf);
            if((len < 16 + 5) || userPwdCheckRet < 0) {
                printf("do not send any message!!!\n");
                continue;
            }

            if(userPwdCheckRet == MSG_CONNECT_CHECK_ERROR) {
                printf("The monitor recv cm is wrong, the md5 check is wrong\n");
                break;
            }

            if(local_buf.extend != NULL) {
                free(local_buf.extend);
                local_buf.extend = NULL;
            }

        }
    }
    upgrade_flag = 0;
    close(connected_fd);
    connected_fd = -1;
    #ifdef ANDROID
		MonitorAgent::GetInstance()->NotifyAgent(MGMT_MT_EXIT_DEBUG, 0, NULL);
	#endif
    GetInstance()->MonitorInitializeStateSet(0);
    return NULL;
}

int MonitorManager::monitor_cmd_recv_cmd(moni_buf_t local_buf_recv)
{
    int error = 0;
    int len = 0;
    char temp_buf[MONITOR_MSGLEN + 1] = {0};

    if(upgrade_flag == 0) {
    //xuke    if(monitorUpgradeGetLength() != 0)
            upgrade_flag = 1; //√â√Ω¬º¬∂√Å√∑¬≥√å
    }

    while(len < MONITOR_MSGLEN && (error = recv(client_fd, local_buf_recv->buf + len, MONITOR_MSGLEN - len, MSG_DONTWAIT)) > 0)
        len += error;

    if(MonitorInitializeStateGet() == 2) {
        memcpy(temp_buf, local_buf_recv->buf, len);
        memset(local_buf_recv->buf, 0, sizeof(local_buf_recv->buf));

        len = monitor_aes_cbc_decrypt(temp_buf, local_buf_recv->buf, len);
        if(len == 0)
            return 0;

        local_buf_recv->len = len;
        printf("\nRecive data(%s) lenght(%d)\n", local_buf_recv->buf, len);
        return error;
    } else {
        local_buf_recv->len = len;
        printf("\nRecive data(%s) lenght(%d)\n", local_buf_recv->buf, len);
        return error;
    }
    return 0;
}

/***************************************************************************
 *Àµ√˜£∫÷˜“™”√”⁄∑‚◊∞“ª∏ˆªÿ”¶TCP±®Œƒµƒ∫Ø ˝
 ****************************************************************************/
int MonitorManager::monitor_cmd_response(moni_buf_t local_buf_temp)
{
    int sendtotallen = 0;
    int len = 0;
    int error = 0;
    char senddata[MAX_SEND_LEN] = {0};
    char *temp_senddata = NULL;
    char *temp_recvdata = NULL;

    memset(senddata, 0, MAX_SEND_LEN);
    sendtotallen = strlen(local_buf_temp->buf);
    strcpy(senddata, local_buf_temp->buf);

    //printf("The monitor state(%d) response(%s)\n", MonitorInitializeStateGet(), senddata);
    if(MonitorInitializeStateGet() > 1) { // ≈–∂œinitialize ∫ÛÀ˘”–µƒ◊¥Ã¨º”√‹°£
        if(local_buf_temp->extend != NULL && (int)(MAX_SEND_LEN - sendtotallen) > (int)(17 + strlen(local_buf_temp->extend))) {
            memcpy(senddata + sendtotallen, local_buf_temp->extend, strlen(local_buf_temp->extend));
            sendtotallen += strlen(local_buf_temp->extend);

            temp_senddata = (char *) malloc(sendtotallen + 1);
            if(NULL == temp_senddata) {
                free(local_buf_temp->extend);
                local_buf_temp->extend = NULL;
                printf("Malloc filed !\n");
                return 0;
            }

            memset(temp_senddata, 0, sendtotallen);
            memcpy(temp_senddata, senddata, sendtotallen);
            *(temp_senddata + sendtotallen) = '\0';
            memset(senddata, 0, sendtotallen);
            sendtotallen = monitor_aes_cbc_encrypt(temp_senddata, (unsigned char *)senddata, sendtotallen);
            *(senddata + sendtotallen) = '\0';
            free(temp_senddata);
            temp_senddata = NULL;
        } else if(local_buf_temp->extend != NULL && (int)(MAX_SEND_LEN - sendtotallen) <= (int)(strlen(local_buf_temp->extend) + 17)) {
            sendtotallen += strlen(local_buf_temp->extend);
            temp_recvdata = (char *) malloc(sendtotallen + 1);
            if(NULL == temp_recvdata) {
                free(local_buf_temp->extend);
                local_buf_temp->extend = NULL;
                printf("Malloc filed !\n");
                return 0;
            }

            memset(temp_recvdata, 0, sendtotallen + 1);
            strcpy(temp_recvdata, local_buf_temp->buf);
            strcat(temp_recvdata, local_buf_temp->extend);

            temp_senddata = (char*) malloc(sendtotallen + 32);
            if(NULL == temp_senddata) {
                if(temp_recvdata) {
                    free(temp_recvdata);
                    temp_recvdata = NULL;
                }
                if(local_buf_temp->extend) {
                    free(local_buf_temp->extend);
                    local_buf_temp->extend = NULL;
                }
                return 0;
            }

            memset(temp_senddata, 0, sendtotallen + 32);

            sendtotallen = monitor_aes_cbc_encrypt(temp_recvdata, (unsigned char *)temp_senddata, strlen(temp_recvdata));
            if(temp_recvdata) {
                free(temp_recvdata);
                temp_recvdata = NULL;
            }

            len = 0;
            while(sendtotallen > len) {
                error = 0;
                error = send(client_fd, temp_senddata + len, sendtotallen - len, MSG_NOSIGNAL);
                if(error <= 0) {
                    printf("monitor send message error:%d/%p", sendtotallen, temp_senddata);
                    break;
                }
                len += error;
            }
            if(temp_senddata) {
                free(temp_senddata);
                temp_senddata = NULL;
            }
            if(local_buf_temp->extend) {
                free(local_buf_temp->extend);
                local_buf_temp->extend = NULL;
            }
            return 0;

        } else {
            //sendtotallen = local_buf_temp->len;
            temp_senddata = (char*) malloc(sendtotallen + 1);
            if(NULL == temp_senddata) {
                printf("malloc filed!!!!!!\n");
                return 0;
            }
            strcpy(temp_senddata, senddata);
            memset(senddata, 0, MAX_SEND_LEN);
            //sendtotallen = pkcs5_padding(temp_senddata,sendtotallen,senddata,sendtotallen,16);
            sendtotallen = monitor_aes_cbc_encrypt(temp_senddata, (unsigned char *)senddata, strlen(temp_senddata));
            free(temp_senddata);
            temp_senddata = NULL;

        }
    } else {
        memcpy(senddata, local_buf_temp->buf, local_buf_temp->len);
        sendtotallen = local_buf_temp->len;
        if(local_buf_temp->extend != NULL && (int)(MAX_SEND_LEN - sendtotallen) > (int)(1 + strlen(local_buf_temp->extend))) {
            memcpy(senddata + sendtotallen, local_buf_temp->extend, strlen(local_buf_temp->extend));
            sendtotallen += strlen(local_buf_temp->extend) + 1;
            *(senddata + sendtotallen) = '\0';
        }

        MonitorInitializeStateSet(2);
    }
    //printf("Response data(%s)\n", senddata);
    len = 0;
    while(sendtotallen > len) {
        error = 0;
        error = send(client_fd, senddata + len, sendtotallen - len, MSG_NOSIGNAL);
        if(error <= 0) {
            printf("monitor send message error:%d/%p", local_buf_temp->len, local_buf_temp->extend);
            break;
        }
		printf("==========send %d bytes \n", error);
        len += error;
    }

    if(local_buf_temp->extend != NULL) {
        free(local_buf_temp->extend);
        local_buf_temp->extend = NULL;
    }
    return 0;
}


int MonitorManager::MonitorInitializeStateGet(void)
{
    return initialize_state ;
}

void MonitorManager::MonitorInitializeStateSet(int state)
{
    if(MonitorInitializeStateGet() != state)
        initialize_state = state ;
}

int MonitorManager::monitor_aes_cbc_encrypt(const char *in, unsigned char *out, const int length)
{
    AES_KEY aes_ks;
    int encrypt_len = 0;
    int tmp_len = 0;
    char buf[17] = {'\0'};
    char input[ENCRYPT_MAX_LEN + 1] = {0};
    char * long_input = NULL;

    if(in == NULL || length == 0) {
        return -1;
    }
    tmp_len = length;
    AES_set_encrypt_key(g_monitor_aes_keys, 128, &aes_ks);
    if(length >= (ENCRYPT_MAX_LEN - 17)) {
        long_input = (char *)malloc(length + 17);
        if(NULL == long_input) {
            printf("malloc filed !!!!\n");
            return -1;
        }
        memset(long_input, 0, length + 17);
        encrypt_len = pkcs5_padding(in, length, long_input, tmp_len, 16);
        strncpy(buf, VECTOR, 16);
        buf[16] = '\0';
        AES_cbc_encrypt((unsigned char *)long_input, out, encrypt_len, &aes_ks, (unsigned char *)buf, 1);
        free(long_input);
        return encrypt_len;
    }
    encrypt_len = pkcs5_padding(in, length, input, tmp_len, 16);
    //printf("encrypt_len = %d\n", encrypt_len);
    strncpy(buf, VECTOR, 16);
    buf[16] = '\0';
    AES_cbc_encrypt((unsigned char *)input, out, encrypt_len, &aes_ks, (unsigned char *)buf, 1);
    return encrypt_len;
}

int MonitorManager::monitor_aes_cbc_decrypt(const char *in, char *out, const int length)
{
    int decrypt_len = 0;
    char buf[17] = {'\0'};
    AES_KEY aes_ks;

    if(in == NULL || length == 0 || length % 16 != 0) {
        return 0;
    }
    AES_set_decrypt_key(g_monitor_aes_keys, 128, &aes_ks);
    strncpy(buf, VECTOR, 16);
    buf[16] = '\0';
    AES_cbc_encrypt((unsigned char *)in, (unsigned char *)out, length, &aes_ks, (unsigned char *)buf, 0);
    decrypt_len = pkcs5_padding_remove(out, length);
    //printf("decrypt_len = %d\n" , decrypt_len);
    return decrypt_len;
}

int MonitorManager::pkcs5_padding(const char *plain_data, int plain_data_len, char *padded_data, int padded_data_len, int block_size)
{
    int pad_len = plain_data_len % block_size;

    if(plain_data_len > padded_data_len)
        return -1;

    memcpy(padded_data, plain_data, plain_data_len);
    if(pad_len) {
        pad_len = block_size - pad_len;
        int i;
        for(i = 0; i < pad_len; i ++) {
            padded_data[plain_data_len + i] = pad_len;
        }
    } else {
        pad_len = 16;
        int i;
        for(i = 0; i < pad_len; i ++) {
            padded_data[plain_data_len + i] = pad_len;
        }
    }

    //printf("pad_len = %d\n", pad_len);
    return pad_len + plain_data_len;
}


int MonitorManager::pkcs5_padding_remove(char *plain_data, int plain_data_len)//?o?¬°√Ä¬°√™?plain_data???¬¶√ã¬°√§√õ√é¬°√™??¬°√§plain_data¬¶√å¬°√©uf3¬°√®??plain_data_len + 1;¬°¬§??¬©√°?¬≥‚Ä°
{
    int padding_value = plain_data[plain_data_len - 1];

    while(padding_value > 0) {
        if(plain_data[plain_data_len - padding_value] != plain_data[plain_data_len - 1])
            return -1;
        padding_value --;
    }
    plain_data[plain_data_len - plain_data[plain_data_len - 1]] = '\0';
    return (plain_data_len - plain_data[plain_data_len - 1]);
}

int MonitorManager::monitorDiagMsgPut(DIAG_MSG *msg)
{
    printf("Send message: cmd(%d), length(%d), url(%s), buf(%s)\n", msg->cmd, msg->length, msg->url , msg->buf.buf);
    if(msgq_put(g_Msg, (char *)msg, 2) < 0)
        return -1;
    else
        return 0;
}

int stbMonitorInit(char *XMLFilePath)
{
	MonitorManager *manager = MonitorManager::GetInstance();
	MonitorAgent *agent = MonitorAgent::GetInstance();

	if (!manager || !agent)
		return -1;

#ifdef ANDROID
	agent->SetReadCallback((fnMgmtIoCallback)DoRead);
	agent->SetWriteCallback((fnMgmtIoCallback)DoWrite);
	agent->SetNotifyCallback((fnMgmtNotifyCallback)DoNotify);
	agent->SetLogExpCallback((fnMgmtLogExportCallback)DoLogExp);
#else
	agent->SetReadCallback((fnMgmtIoCallback)mgmtReadCallBack);
	agent->SetWriteCallback((fnMgmtIoCallback)mgmtWriteCallback);
	agent->SetNotifyCallback((fnMgmtNotifyCallback)mgmtNotifyCallback);
	//agent->SetLogExpCallback((fnMgmtLogExportCallback)DoLogExp);
#endif

    manager->cmdNameXMLinit(XMLFilePath);
	manager->MonitorInit();
	return 0;
}

#ifndef ANDROID
int stbMonitorConnectFlag(int value)
{
    MonitorConnectFlag = value;
    return 0;
}


int stbMonitorShutDown()
{
    MonitorManager::GetInstance()->MonitorShutdown();
    return 0;
}
#endif

