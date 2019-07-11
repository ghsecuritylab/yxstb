#ifndef __PORTING_PAY_SHELL_H__
#define __PORTING_PAY_SHELL_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    PAY_SHELL_CMD_LOAD = 0,
    PAY_SHELL_CMD_UNLOAD,
    PAY_SHELL_CMD_REDOWNLOAD,
    PAY_SHELL_CMD_RELOAD,
    PAY_SHELL_CMD_REMSG,
    PAY_SHELL_CMD_MAX
}pay_shell_cmd_e;

typedef enum
{
    PAY_SHELL_SUCCESS = 0,
    PAY_SHELL_ERROR_INIT,
    PAY_SHELL_ERROR_DOWNLOAD,
    PAY_SHELL_ERROR_AUTH,
    PAY_SHELL_ERROR_LOAD,
    PAY_SHELL_ERROR_MSG2PAYSHELL,
    PAY_SHELL_ERROR_MSG2BROWSER,
    PAY_SHELL_ERROR_UNLOAD,
    PAY_SHELL_ERROR_DATA,
    PAY_SHELL_ERROR_STATUS,
    PAY_SHELL_ERROR_MAX
}pay_shell_error_e;

typedef enum
{
    PAY_SHELL_STATUS_LOADING = 0,
    PAY_SHELL_STATUS_LOAD_FINISH,   
    PAY_SHELL_STATUS_UNLOAD,
    PAY_SHELL_STATUS_MAX
}pay_shell_status_e;

typedef enum
{
    PAY_SHELL_EVENT_ERROR                       = 1000,
    PAY_SHELL_EVENT_PAY_CLIENT_DOWNLOAD_SUCCESS = 1001,
    PAY_SHELL_EVENT_IPTVSIG_DOWNLOAD_SUCCESS    = 1002, 
    PAY_SHELL_EVENT_PAY_CLIENT_AUTH_SUCCESS     = 1003,
    PAY_SHELL_EVENT_PAY_CLIENT_LOAD_SUCCESS     = 1004,
}pay_shell_event_e;

typedef struct 
{
    pay_shell_cmd_e cmd;
    char msg[1024];
}pay_shell_msg_t;

typedef struct
{
    void *dl_lib;
    int (*ioctl)(const char *, char *, int *);
}pay_shell_node_t;

typedef struct
{
    pay_shell_error_e errId;
    char errString[128];
}pay_shell_error_t;

typedef struct
{
    int eventID;
    char *value;
    void *handle;
}pay_shell_event_t;

int pay_shell_init(void);
int pay_shell_input_msg(int cmd, char *buf);
int pay_shell_output_msg(int status, int error);
char* pay_shell_string_error(int error);

char* pay_shell_ioctl(const char* data);

int pay_shell_redownload(void);
int pay_shell_reload(void);
int pay_shell_remsg(void);
int pay_shell_setHandle(void *handle);
void* pay_shell_getHandle(void);

static int pay_shell_task(void);


#ifdef __cplusplus
}
#endif

#endif
