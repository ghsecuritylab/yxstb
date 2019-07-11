/*******************************************************************************
    公司：
            Yuxing software
    纪录：
            2008-1-26 21:12:26 create by Liu Jianhua
    模块：
            tr069
    简述：
            tr069 实现
 *******************************************************************************/
#ifndef __TR069_STRUCT_H__
#define __TR069_STRUCT_H__

#include <pthread.h>
#include "tr069_global.h"

struct Timer;

#define TR069_GLOBAL_NUM_16         16

#define TR069_VALUE_SIZE_256        256
#define TR069_NAME_SIZE_64          64
#define TR069_BUF_SIZE_4096         4096
#define TR069_BUF_LARGE_SIZE_64K    (64*1024)

#define TR069_COMMANDKEY_LEN_32     32

#define TR069_GLOBAL_HASH_SIZE_199  199
#define TR069_PARAM_HASH_SIZE_1999  1999

#define TR069_PARAM_BOOT_NUM_32     32
#define TR069_PARAM_INFORM_NUM_32   32
#define TR069_PARAM_REMAIN_NUM_64   64

#define TR069_EVENTCODE_LEN_32      32
#define TR069_EVENTCODE_EXTS_20     20

#define TR069_FILETYPE_LEN_64       64
#define TR069_USERNAME_LEN_256       256
#define TR069_PASSWORD_LEN_256       256
#define TR069_TARGETFILENAME_LEN_256 256

enum
{
    EVENT_INFORM_NONE = 0,
    EVENT_INFORM_VALID,
    EVENT_INFORM_POSTING
};

enum
{
    EVENTCODE_BOOTSTRAP = 0,
    EVENTCODE_BOOT,
    EVENTCODE_PERIODIC,
    EVENTCODE_SCHEDULED,
    EVENTCODE_VALUE_CHANGE,
    EVENTCODE_KICKED,
    EVENTCODE_CONNECTION_REQUEST,
    EVENTCODE_TRANSFER_COMPLETE,
    EVENTCODE_DIAGNOSTICS_COMPLETE,

    EVENTCODE_M_REBOOT,//9
    EVENTCODE_M_SCHEDULE,
    EVENTCODE_M_DOWNLOAD,
    EVENTCODE_M_UPLOAD,
    EVENTCODE_M_SHUTDOWN,

    EVENTCODE_X_BASICINFO,//14
    EVENTCODE_X_DOWNLOADCOMPLETE,
    EVENTCODE_X_UPLOADCOMPLETE,

    EVENTCODE_X_EXTEND,

    EVENTCODE_Y_ACTIVE,//18
    EVENTCODE_Y_SUSPEND,
    EVENTCODE_Y_GETRPCMETHODS,
    EVENTCODE_Y_SCHEDULED,

    EVENTCODE_Y_CONFIG_PRINT,//22
    EVENTCODE_Y_VALUE_CHANGE,
    EVENTCODE_Y_LOAD,
    EVENTCODE_Y_OBJECT_ADD,
    EVENTCODE_Y_OBJECT_DELETE,
    EVENTCODE_Y_SHUTDOWN
};

#define EVENTCODE_MAX EVENTCODE_X_EXTEND

enum
{
    /*9000*/ FAULT_SUPPORTED = 0,
    /*9001*/ FAULT_REQUEST_DENIED,
    /*9002*/ FAULT_INTERNAL_ERROR,
    /*9003*/ FAULT_ARGUMENTS,
    /*9004*/ FAULT_RESOURCES_EXCEEDED,
    /*9005*/ FAULT_PARAMETER_NAME,
    /*9006*/ FAULT_PARAMETER_TYPE,
    /*9007*/ FAULT_PARAMETER_VALUE,
    /*9008*/ FAULT_NON_WRITABLE,
    /*9009*/ FAULT_NOTIFICATION_REJECTED,
    /*9010*/ FAULT_DOWNLOAD,
    /*9011*/ FAULT_UPLOAD,
    /*9012*/ FAULT_AUTHENTICATION,
    /*9013*/ FAULT_PROTOCOL,
    /*9014*/ FAULT_DOWNLOAD_JOIN,
    /*9015*/ FAULT_DOWNLOAD_CONTACT,
    /*9016*/ FAULT_DOWNLOAD_ACCESS,
    /*9017*/ FAULT_DOWNLOAD_COMPLETE,
    /*9018*/ FAULT_DOWNLOAD_CORRUPTED,
    /*9019*/ FAULT_DOWNLOAD_AUTHENTICATION,

    FAULT_CODE_MAX
};

struct Reboot {
    int rebootstate;
    char commandkey[TR069_COMMANDKEY_LEN_32 + 4];
};

struct Schedule {
    int scheduletime;
    char commandkey[TR069_COMMANDKEY_LEN_32 + 4];
};

struct Load {
    struct Load *next;

    uint32_t SN;
    int isDownload;
    int faultcode;
    int transferstate;

    char CommandKey[TR069_COMMANDKEY_LEN_32 + 4];
    char FileType[TR069_FILETYPE_LEN_64 + 4];
    char URL[TR069_URL_LEN_1024 + 4];
    char Username[TR069_USERNAME_LEN_256 + 4];
    char Password[TR069_PASSWORD_LEN_256 + 4];
    char TargetFileName[TR069_TARGETFILENAME_LEN_256 + 4];

    uint32_t loadtime;
    uint32_t starttime;
    uint32_t completetime;
};

struct Object {
    struct Param *firstChild;
    struct Param *lastChild;
};

struct String {
    uint32_t size;
    char value[4];
};

struct Param {
    struct Param *treeNext;
    struct Param *treePrev;
    struct Param *treeParent;

    struct Param *hashNext;/* hash table next */
    struct Param *shortNext;/* short hash table next */

    struct Param *paramNext;
    struct Param *changeNext;
    int           isChange;

    uint32_t type;
    uint32_t attr;
    int index;

    uint32_t cksum;
    uint8_t cksum_flag;
    uint8_t changeNotification;
    uint8_t currentNotification;
    uint8_t defaultNotification;

    union {

        struct {
            Tr069ValueFunc chkValue;
            Tr069ValueFunc getValue;
            Tr069ValueFunc setValue;

            OnChgFunc onchg;
            struct String *string;
        } prm_ma;

        struct {
            int addIndex;
            struct Object objValue;
            struct Object objVirtual;
        } obj_ma;

    } methodattr;

    char *name;/*完整名*/
    char *sname;/*短名*/
};

struct Param_rmn {
    int index;
    char *string;
};

#define prm_chkval      methodattr.prm_ma.chkValue
#define prm_setval      methodattr.prm_ma.setValue
#define prm_getval      methodattr.prm_ma.getValue

#define prm_onchg       methodattr.prm_ma.onchg

#define prm_string      methodattr.prm_ma.string
#define prm_objValue    methodattr.obj_ma.objValue
#define prm_objVirtual  methodattr.obj_ma.objVirtual
#define prm_addIndex    methodattr.obj_ma.addIndex

struct FaultArg {
    char paramname[TR069_NAME_FULL_SIZE_128 + 4];
    int faultcode;
};

struct Event {
    int userFlag;
    char CommandKey[TR069_COMMANDKEY_LEN_32 + 4];
};

enum {
    GLOBAL_TYPE_INT = 0,
    GLOBAL_TYPE_STR32,
    GLOBAL_TYPE_MAX
};

struct Global {
    struct Global*  next;
    int             type;
    char*           name;
    void*           addr;
};

struct TR069 {
    int state;        /* tr069 task state: Active, Suspend, Error. */
    int state_boot;

    int sys_reset;
    int sys_reboot;

    int diag_ping;
    int diag_trace;

    pthread_mutex_t mutex;

    TR069Msgq_t  msgqPipe;/* Tr069 thread accept user's commands. */
    int         msgqSock;

    SOCKET sock_request;/* Accept ACS's connect request based on http over TCP. */

    struct Timer *timer_pool;
    struct Timer *timer_queue;

    struct Event event_array[EVENTCODE_MAX]; /* Events that CFE notify ACS via Inform request. */
    int event_inform;    /* If has inform event, set TRUE. */

    GlobalEvent *event_global;

    int flag_getrpc;

    //连接失败重试机制
    int random_factor;
    int retry_time;
    int retry_state;
    int retry_interval;
    int retry_count;
    int retry_connect;//huawei C15

    int chg_delay;
    struct Param *chg_head;

    uint32_t cwmp_id;
    char cwmp_id_req[TR069_NAME_SIZE_64 + 4];
    char buf[TR069_BUF_SIZE_4096 + 4];
    char buf_large[TR069_BUF_LARGE_SIZE_64K + 4];

    struct Param *prm_hash[TR069_PARAM_HASH_SIZE_1999];
    struct Param *prm_shash[TR069_PARAM_HASH_SIZE_1999];
    struct Param **table_array;//按序号排列
    int table_size;
    int table_off;

    struct Object prm_object;//与服务器交互参数列表
    int prm_pedant;//参数设置时，是否严格检测参数类型
    int prm_num;

    struct Param *prm_change;

    int prm_boot_array[TR069_PARAM_BOOT_NUM_32];
    int prm_boot_num;
    int prm_inform_array[TR069_PARAM_INFORM_NUM_32];
    int prm_inform_num;
    struct Param_rmn prm_remain_array[TR069_PARAM_REMAIN_NUM_64];
    int prm_remain_num;

    int inst_num;
    char addrtype_config[15 + 1];

    int fault_code;
    int fault_args_num;
    struct FaultArg fault_args_array[TR069_FAULT_NUM];

    struct Soap *soap;
    struct Http *http;
    struct Http *http_a;//accept http

    //信息保存
    int save_flag;    /* Sync variable that saved in flash. */

    Boolean bootstrap;
    struct Reboot reboot;
    struct Schedule schedule;

	struct Load *loadQueue;

    struct Global *global_hash[TR069_GLOBAL_HASH_SIZE_199];
    struct Global *global_table[TR069_GLOBAL_NUM_16];
    int global_num;
};

#endif //__TR069_STRUCT_H__
