#ifndef MonitorManager_h
#define MonitorManager_h

#include "MonitorDef.h"
#include "MonitorCmd.h"
#include "ParseXML.h"



int stb_startDebugInfo_ioctl(moni_buf_t buf, int len);
int stb_stopDebugInfo_ioctl(moni_buf_t buf, int len);

#ifndef ANDROID
int stbMonitorInit(char *XMLFilePath);
int stbMonitorConnectFlag(int value);
int stbMonitorShutDown();
#endif


class MonitorCmd;
class MonitorManager {
public:
    MonitorManager();
    virtual ~MonitorManager();

    void MonitorShutdown();
    void MonitorInitializeStateSet(int state);
    int MonitorInit();
    int MonitorDiagInit();
    int msgq_put(msgq_t msgq, char* msg, int sec);
    int monitor_cmd_response(moni_buf_t local_buf_temp);
    int monitor_cmd_recv_cmd(moni_buf_t local_buf_recv);
    static MonitorManager* GetInstance();
    static ParseXML* cmdNameXML();
    int cmdNameXMLinit(char *xmlPath);
    int monitorDiagMsgPut(DIAG_MSG* msg);
    int trriggerConnect(char *ip);

private:
    static void* MonitorStart(void* arg);
    static void* monitor_pthread_func(void* arg);
    static void* monitor_diagTask(void* arg);
    int MonitorClose();
    void monitor_diagProc(DIAG_MSG* msg);
    int MonitorDiaginit();
    int MonitorInitializeStateGet(void);
    int monitor_aes_cbc_decrypt(const char* in, char* out, const int length);
    int monitor_aes_cbc_encrypt(const char* in, unsigned char* out, const int length);
    int pkcs5_padding(const char* plain_data, int plain_data_len, char* padded_data, int padded_data_len, int block_size);
    int pkcs5_padding_remove(char* plain_data, int plain_data_len);

private:
    static int g_MsgFd;
    static int upgrade_flag;
    static int client_fd;
    static int server_fd;
    static moni_buf local_buf;
    static int s_stopFlag;
    static ParseXML  *s_cmdNameXML;
    //static mid_msgq_t g_mdMsg;
};
#endif //MonitorManager_h

