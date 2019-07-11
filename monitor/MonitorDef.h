#ifndef MonitorDef_h
#define MonitorDef_h


#define MONITOR_MSGLEN 2560
#define MONITOR_CMDLEN 64
#define MONITOR_PAEALEN 64
#define MONITOR_PORT 9003
//因为readparalist 需要512*1024，则统一都改为这么大
#define READ_BUFFER (512 * 1024)
//准许升级文件的最大值，避免把root目录写满
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

#define MSG_CONNECT_OK 200 // 所有的命令操作成功返回码	统一提示“操作成功”

#define MSG_UPGRADE_SAME_ERROR 300  // 升级版本与现有版本相同	“版本相同，升级没有执行”
#define MSG_UPGRADE_ING_OK  301   // 升级中	“升级中……”，并更新进度条。升级过程任何操作都不响应，并返回此信息
#define MSG_UPGRADE_END_OK 302   // 升级完成	“升级完成，自动重启”
#define MSG_UPGRADE_CHECK_ERROR 303 // 版本校验失败	“非法的版本文件，操作失败”

#define MSG_PARAMETER_ERROR  401  // 输入了非法参数	“参数非法，操作失败”
#define MSG_COMMAND_FAIL_ERROR 402  // 输入了非法命令	“命令非法，操作失败”
#define MSG_CHANNEL_UNDEFINE2_ERROR 403 // 输入了不存在频道	“频道不存在，操作失败”
#define MSG_HAVE_CLIENT_LINK 404 // 已有客户端连接	“已存在其它连接”

#define MSG_CONNECT_CHECK_ERROR 501 // 校验码错误，拒绝连接	“非法连接，或用户名和密码不正确”
#define MSG_COMMAND_UNDEFINE_ERROR 502 // 命令没有定义	“非法的操作”
#define MSG_ACCOUNT_LOCKOUT 503 //账号锁定

#define MSG_CHANNEL_UNDEFINE_ERROR  601  // 输入了不存的频道号	“频道不存在，操作失败”
#define MSG_TIMEOUT_ERROR 602   // 接收数据超时	“没有数据”
#define MSG_PLAY_URL_ERROR 603 // 接连播放错误	“播放连接不可用”

#define MSG_FILE_TRANSPORTING 701 // 文件传送中	“接收中……”，并更新进度条。

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

