#ifndef MonitorDef_h
#define MonitorDef_h


#define MONITOR_MSGLEN 2560
#define MONITOR_CMDLEN 64
#define MONITOR_PAEALEN 64
#define MONITOR_PORT 9003
//��Ϊreadparalist ��Ҫ512*1024����ͳһ����Ϊ��ô��
#define READ_BUFFER (512 * 1024)
//׼�������ļ������ֵ�������rootĿ¼д��
#define MAX_UPGRADE_FILE  1024*1024*60
#define MAX_SEND_LEN 1024*4
#define IDENTIFY_CODE_LEN 8
#define SESSIONID_LEN 16
#define pause -55555
#define fast_forward  -55556
#define fast_backward -55557
#define URL_LEN 512
#define CONFIG_STRING_LEN (3 * 1024)
#define CHANNEL_STRING_LEN (32 * 1024)
#define ENCRYPT_MAX_LEN 4096
#define MODULUS "A7964C580775747B016CB256310C59FD8856BBAC0652690618BC4A9D05BF32A8DEC90A6929E7957C843358F1D55D0FDED1A3924D1EA6FA5381EB70BB0A74B6BE1E554A65392D69F36A65FC01461BEB4018644DEFDBA9647649F196872CBBEA86EAB0E4F8112BF92B5FE3365771F62F0D28D6FA8BF25B48860C64109B9F274F09"


#define PUBLIC_EXPONENT 65537
#define VECTOR "8D352C149D0327BF"
#define REMOTEPCAPFILE "/var/"
#define REMOTEPCAPFILEEXT ".cap"
#define COLLECTFOLDERNAME "/var/STBDebugInfo"
#define REMOTEPCAPFILESIZE (150*1024*1024)
#define KEY_LEN 17
#define OUTPUT_LEN 129
#define ICMP_ECHO	8
#define LARGE_URL_MAX_LEN 1024

#define MSG_CONNECT_OK 200 // ���е���������ɹ�������	ͳһ��ʾ�������ɹ���

#define MSG_UPGRADE_SAME_ERROR 300  // �����汾�����а汾��ͬ	���汾��ͬ������û��ִ�С�
#define MSG_UPGRADE_ING_OK  301   // ������	�������С������������½����������������κβ���������Ӧ�������ش���Ϣ
#define MSG_UPGRADE_END_OK 302   // �������	��������ɣ��Զ�������
#define MSG_UPGRADE_CHECK_ERROR 303 // �汾У��ʧ��	���Ƿ��İ汾�ļ�������ʧ�ܡ�

#define MSG_PARAMETER_ERROR  401  // �����˷Ƿ�����	�������Ƿ�������ʧ�ܡ�
#define MSG_COMMAND_FAIL_ERROR 402  // �����˷Ƿ�����	������Ƿ�������ʧ�ܡ�
#define MSG_CHANNEL_UNDEFINE2_ERROR 403 // �����˲�����Ƶ��	��Ƶ�������ڣ�����ʧ�ܡ�
#define MSG_HAVE_CLIENT_LINK 404 // ���пͻ�������	���Ѵ����������ӡ�

#define MSG_CONNECT_CHECK_ERROR 501 // У������󣬾ܾ�����	���Ƿ����ӣ����û��������벻��ȷ��
#define MSG_COMMAND_UNDEFINE_ERROR 502 // ����û�ж���	���Ƿ��Ĳ�����
#define MSG_ACCOUNT_LOCKOUT 503 //�˺�����

#define MSG_CHANNEL_UNDEFINE_ERROR  601  // �����˲����Ƶ����	��Ƶ�������ڣ�����ʧ�ܡ�
#define MSG_TIMEOUT_ERROR 602   // �������ݳ�ʱ	��û�����ݡ�
#define MSG_PLAY_URL_ERROR 603 // �������Ŵ���	���������Ӳ����á�

#define MSG_FILE_TRANSPORTING 701 // �ļ�������	�������С������������½�������

#define DIAG_ERROR_NONE				0
#define DIAG_ERROR_RESOLVE			1
#define DIAG_ERROR_HOPEXCEEDED		2
#define DIAG_ERROR_INTERNAL			3
#define DIAG_ERROR_OTHER			4

#define DIAG_HOST_LEN				256
#define DIAG_HOST_NUM				64
#define USER_LEN                    (32 + 2)

#define GET_FLAG_ERROR               -2

struct moni_buf {
    int len;
    char buf[MONITOR_MSGLEN + 1];
    char* extend;
    char remoteip[20];
};

typedef struct moni_buf* moni_buf_t;

typedef enum {
    PING_START = 0,
    TRACEROUTE_START
} DIAG_CMD;

typedef struct {
    DIAG_CMD cmd;
    int length;
    char url[256];
    struct moni_buf buf;
    void* func;
} DIAG_MSG;

typedef struct{
	int src_type;
	char function_call[100];
	char call_value[100];
	char parameter[100];
	char data[READ_BUFFER];
}CMD_MSG;

typedef struct{
	int ret_value;
	char data[READ_BUFFER];
}RES_MSG;


struct msgq {
	int size;
	int fds[2];
};

typedef struct msgq* msgq_t;


#define STB_MONITOR_LOG(x,...)  printf(x,##__VA_ARGS__)


typedef long long msec_t;
typedef void (*PTR_CALLBACK)(moni_buf_t buf,char *str);

#endif //MonitorDef_h

