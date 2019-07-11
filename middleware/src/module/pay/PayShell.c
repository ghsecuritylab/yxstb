#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "SysSetting.h"
#include "PayShell.h"
#include "PayShellAssertions.h"
#include "configCustomer.h"
#include "MessageTypes.h"

#include "mid/mid_http.h"
#include "mid/mid_msgq.h"

#include "config/pathConfig.h"

static mid_msgq_t g_msgq = NULL;
static char *g_handle = NULL;
static char g_reply[50*1024] = {0};
static pay_shell_node_t g_psnode;

static const pay_shell_error_t g_pay_shell_err[] =
{
    {PAY_SHELL_SUCCESS,             "Congratulation, pay shell success!"},
    {PAY_SHELL_ERROR_INIT,          "Initializate pay shell error!"},
    {PAY_SHELL_ERROR_DOWNLOAD,      "Download pay client error!"},
    {PAY_SHELL_ERROR_AUTH,          "Authenticate pay client error!"},
    {PAY_SHELL_ERROR_LOAD,          "Load pay client error!"},
    {PAY_SHELL_ERROR_MSG2PAYSHELL,  "Send to pay shell messages error!"},
    {PAY_SHELL_ERROR_MSG2BROWSER,   "Send to browser messages error!"},
    {PAY_SHELL_ERROR_UNLOAD,        "Unload pay client error!"},
    {PAY_SHELL_ERROR_DATA,          "DATA error!"},
    {PAY_SHELL_ERROR_STATUS,        "STATUS error!"},
    {PAY_SHELL_ERROR_MAX,           "don't find the description of error!"}
};

int pay_shell_init(void)
{
    pthread_t pay_shell_handle;
    int ret = -1;
    static int init = 0;
    char cmd[512] = {0};

    if (init != 0)
        PayShellErrOut("repeat initialization!\n");

    if (access(DEFAULT_MODULE_PAY_DATAPATH, F_OK) == 0) {
        LogPayShellDebug("%s exists, remove it!\n", DEFAULT_MODULE_PAY_DATAPATH);
        sprintf(cmd, "rm -rf %s", DEFAULT_MODULE_PAY_DATAPATH);

        system(cmd);
    }

    ret = mkdir(DEFAULT_MODULE_PAY_DATAPATH, 0777);

    if(ret != 0)
        PayShellErrOut("mkdir %s failed: %s\n", DEFAULT_MODULE_PAY_DATAPATH, strerror(errno));

    memset(&g_psnode, 0, sizeof(pay_shell_node_t));

    g_msgq = mid_msgq_create(419, sizeof(pay_shell_msg_t));
    if(g_msgq == NULL)
        PayShellErrOut("msgq_create failed!\n");

    ret = pthread_create(&pay_shell_handle, NULL, (void *)pay_shell_task, NULL);
    if (ret)
        PayShellErrOut("pthread_create\n");

    init = 1;

    return PAY_SHELL_SUCCESS;
Err:
    return PAY_SHELL_ERROR_INIT;

}

int pay_shell_input_msg(int cmd, char *buf)
{
    int ret = -1;

    pay_shell_msg_t pmsg;
    memset(&pmsg, 0, sizeof(pay_shell_msg_t));


    if((cmd < PAY_SHELL_CMD_LOAD)||(cmd >= PAY_SHELL_CMD_MAX))
        PayShellErrOut("pmsg.cmd is %d error!\n", cmd);

    pmsg.cmd = cmd;
    strcpy(pmsg.msg, buf);

    if(pmsg.msg[1024-1] != '\0')
        PayShellErrOut("pmsg.msg error! please init on memset(pmsg.msg, 0, 1024*40);\n");

    ret = mid_msgq_put(g_msgq, (char *)(&pmsg), 2);
    if (ret)
        PayShellErrOut("msg put error!\n");

    return PAY_SHELL_SUCCESS;
Err:
    return PAY_SHELL_ERROR_MSG2PAYSHELL;
}

int pay_shell_output_msg(int status, int error)
{
    LogPayShellDebug("status(%d):%s\n", status, pay_shell_string_error(error));

    if (status == PAY_SHELL_EVENT_ERROR) {
        switch (error) {
            case PAY_SHELL_ERROR_DOWNLOAD:
            case PAY_SHELL_ERROR_AUTH:
            case PAY_SHELL_ERROR_LOAD:
                g_psnode.ioctl = 0xffffffff;
                break;
            default:
                break;
        }

        sendMessageToNativeHandler(MessageType_PayShell, PAY_SHELL_EVENT_ERROR, error, 0);
    } else {
        sendMessageToNativeHandler(MessageType_PayShell, status, 0, 0);
    }

    return 0;
}

int pay_shell_redownload(void)
{
    return pay_shell_input_msg(PAY_SHELL_CMD_REDOWNLOAD, "pay_shell_cmd_redownload");
}

int pay_shell_reload(void)
{
    return pay_shell_input_msg(PAY_SHELL_CMD_RELOAD, "pay_shell_cmd_reload");
}

int pay_shell_remsg(void)
{
    return pay_shell_input_msg(PAY_SHELL_CMD_REMSG, "pay_shell_cmd_remsg");
}

char* pay_shell_string_error(int error)
{
    int num = 0;
    for (num; num <= PAY_SHELL_ERROR_MAX; num++) {
        if(g_pay_shell_err[num].errId == error)
            break;
    }

    return g_pay_shell_err[num].errString;
}

static int pay_shell_download(char *url)
{
    int ret = -1;

    LogPayShellDebug("url is %s\n", url);

    ret = file_http_get(url, NULL, 0, NULL, 0, 2);

    if(ret == HTTP_OK_READDATA)
        return PAY_SHELL_SUCCESS;
    else
        return PAY_SHELL_ERROR_DOWNLOAD;
}

static int pay_shell_compare_file(const char* fileA, const char* fileB)
{
    char bufA[1024] = {0};
    char bufB[1024] = {0};
    char *ptrA = NULL;
    char *ptrB = NULL;

    int fd = -1;

    fd = open(fileA, O_RDONLY);
    if (fd < 0) {
        PayShellErrOut("open %s failed: %s\n", fileA, strerror(errno));
    }
    read(fd, bufA, 1024);
    close(fd);

    fd = open(fileB, O_RDONLY);
    if (fd < 0) {
        PayShellErrOut("open %s failed: %s\n", fileB, strerror(errno));
    }
    read(fd, bufB, 1024);
    close(fd);

    /*******wangtonggui changed**************/
    ptrA = strstr(bufA, "= ");
    ptrB = strstr(bufB, "= ");

    ptrA += strlen("= ");
    ptrB += strlen("= ");//here after '=',  is a space

    LogPayShellDebug("ptra = %s\n", ptrA);
    LogPayShellDebug("ptrb = %s\n", ptrB);

    return strcmp(ptrA, ptrB);
Err:
    return -1;
}

static int pay_shell_auth(char* key_name)
{
    char iptvsig[512] = {0};
    char pay_client[512] = {0};
    char cmd_dgst[512] = {0};
    char cmd_rsautl[512] = {0};
    char md[512] = {0};
    char md_ver[512] = {0};

    if (pay_client == NULL) {
        PayShellErrOut("para error!\n");
    }

    snprintf(pay_client, sizeof(pay_client), "%s/lib_%s.so", DEFAULT_MODULE_PAY_DATAPATH, key_name);
    LogPayShellDebug("pay_client is %s\n", pay_client);

    if (access(pay_client, R_OK | F_OK) != 0) {
        PayShellErrOut("access %s failed: %s\n", pay_client, strerror(errno));
    }

    snprintf(iptvsig, sizeof(iptvsig), "%s/%s", DEFAULT_MODULE_PAY_DATAPATH, DEFAULT_PAY_CLIENT_SIGN_FILE);

    snprintf(md, sizeof(md), "%s/md.txt", DEFAULT_MODULE_PAY_DATAPATH);
    snprintf(md_ver, sizeof(md_ver), "%s/md_ver.txt", DEFAULT_MODULE_PAY_DATAPATH);

    if (access(iptvsig, R_OK | F_OK ) == 0) {
        sprintf(cmd_dgst, "openssl dgst %s -out %s %s", DEFAULT_PAY_CLIENT_SIGN_TYPE, md, pay_client);
        system(cmd_dgst);

        sprintf(cmd_rsautl, "openssl rsautl -verify -pubin -inkey %s -in %s -out %s",
                DEFAULT_PAY_SHELL_PUBKEY_PATH, iptvsig, md_ver);

        system(cmd_rsautl);
    } else {
        PayShellErrOut("iptvsig don't exist!");
    }

    if(pay_shell_compare_file(md, md_ver) != 0)
        PayShellErrOut("Authenticate error!\n");

    return PAY_SHELL_SUCCESS;
Err:
    return PAY_SHELL_ERROR_AUTH;
}

static int pay_shell_load(char* key_name)
{
    char pay_client[512] = {0};
    char* dl_error;

    sprintf(pay_client, "%s/lib_%s.so", DEFAULT_MODULE_PAY_DATAPATH, key_name);
    LogPayShellDebug("pay_client is %s\n", pay_client);


    g_psnode.dl_lib = dlopen(pay_client, RTLD_LAZY);

    dl_error = dlerror();
    if(dl_error)
        PayShellErrOut( "dlopen %s fail!, %s\n", pay_client, dl_error );


    g_psnode.ioctl= dlsym(g_psnode.dl_lib, "UBankDevice_ioctl");

    dl_error = dlerror();
    if(dl_error)
        PayShellErrOut( "dlsym UBankDevice_ioctl fail!, %s\n", dl_error );


    return PAY_SHELL_SUCCESS;
Err:
    return PAY_SHELL_ERROR_LOAD;

}

char* pay_shell_ioctl(const char* data)
{
    int ret = 0, pret = -1, length, flag;
    char *p = NULL;

    while (g_psnode.ioctl == NULL) {
        LogPayShellDebug("run here!\n");
        sleep(1);
    }

    if(g_psnode.ioctl == 0xffffffff)
        return "";

    memset(g_reply, 0, sizeof(g_reply));

    ret = g_psnode.ioctl(data, g_reply, &length);
    LogPayShellDebug("ret is %d, length is %d\n", ret, length);
    LogPayShellDebug("g_reply is: \n<------\n%s\n------>\n",g_reply);

    return g_reply;
}

static int pay_shell_unload(pay_shell_node_t* node)
{
    dlclose(node->dl_lib);

    node->dl_lib = NULL;
    node->ioctl = NULL;

    return PAY_SHELL_SUCCESS;
}

static int pay_shell_get_payClientDomain(char* domain, int size)
{
    if(domain == NULL)
        PayShellErrOut("domain is NULL!");

    sysSettingGetString("PayClientDomain", domain, size, 0);

    return PAY_SHELL_SUCCESS;
Err:
    return PAY_SHELL_ERROR_DATA;
}

int pay_shell_setHandle(void *handle)
{
    g_handle = (char *)handle;
    LogPayShellDebug("g_handle is %#x\n", g_handle);
    return 0;
}

void* pay_shell_getHandle(void)
{
    return g_handle;
}

static int pay_shell_task(void)
{
    pay_shell_msg_t msg;
    pay_shell_status_e status;
    pay_shell_error_e ret;

    char payClientDomain[URL_LEN] = {0};
    char key_name[32] = {0};
    char version[16] = {0};
    char location[256] = {0};
    char url[512] = {0};
    char cmd_rm[128] = {0};

    int i = 0;

    status = PAY_SHELL_STATUS_UNLOAD;

    while (1) {
        memset( &msg, 0, sizeof( pay_shell_msg_t ));
        if (mid_msgq_get(g_msgq, (char *)&msg, 60, 0) <= 0) {
            continue;
        }

        LogPayShellDebug( "Pay shell received a message; cmd is %d\n", msg.cmd);

        if (msg.cmd == PAY_SHELL_CMD_LOAD) {
            if (status == PAY_SHELL_STATUS_LOAD_FINISH) {
                LogPayShellDebug("STATUS is %d\n", status);
                pay_shell_output_msg(PAY_SHELL_EVENT_ERROR, PAY_SHELL_ERROR_STATUS);
                continue;
            }

            LogPayShellDebug("Pay shell received a message; msg is %s\n", msg.msg);
            status = PAY_SHELL_STATUS_LOADING;

            sscanf(msg.msg, "%[^?]?version=%[^&]&location=%[^\n]", key_name, version, location);

            for(i=0; i<strlen(key_name); i++)
                key_name[i] = tolower(key_name[i]);

            for(i=0; i<strlen(location); i++)
                location[i] = tolower(location[i]);

            LogPayShellDebug("key_name is %s, version is %s, location is %s\n", key_name, version, location);

            if (g_handle == NULL) {
                LogPayShellDebug("g_handle is NULL\n");
                pay_shell_output_msg(PAY_SHELL_EVENT_ERROR, PAY_SHELL_ERROR_STATUS);
                continue;
            }
#if 1

Pay_Shell_Redownload:
            ret = pay_shell_get_payClientDomain(payClientDomain, sizeof(payClientDomain));
            if (ret != PAY_SHELL_SUCCESS) {
                LogPayShellDebug("%s\n",pay_shell_string_error(ret));
                pay_shell_output_msg(PAY_SHELL_EVENT_ERROR, ret);
                continue;
            }

            //download pay client
            sprintf(url, "%s/%s/lib_%s.so", payClientDomain, location, key_name);
            LogPayShellDebug("the url of pay client is %s\n", url);

            ret = pay_shell_download(url);
            if (ret != PAY_SHELL_SUCCESS) {
                LogPayShellDebug("%s\n",pay_shell_string_error(ret));
                pay_shell_output_msg(PAY_SHELL_EVENT_ERROR, ret);
                continue;
            } else {
                pay_shell_output_msg(PAY_SHELL_EVENT_PAY_CLIENT_DOWNLOAD_SUCCESS, ret);
            }


            //download iptvsig
            sprintf(url, "%s/%s/iptvsig-sha1.bin", payClientDomain, location);
            LogPayShellDebug("the url of iptvsig is %s\n", url);

            ret = pay_shell_download(url);
            if (ret != PAY_SHELL_SUCCESS) {
                LogPayShellDebug("%s\n",pay_shell_string_error(ret));
                pay_shell_output_msg(PAY_SHELL_EVENT_ERROR, ret);
                continue;
            } else {
                pay_shell_output_msg(PAY_SHELL_EVENT_IPTVSIG_DOWNLOAD_SUCCESS, ret);
            }


            //auth pay client by iptvsig
            ret = pay_shell_auth(key_name);
            if (ret != PAY_SHELL_SUCCESS) {
                LogPayShellDebug("%s\n",pay_shell_string_error(ret));
                pay_shell_output_msg(PAY_SHELL_EVENT_ERROR, ret);
                continue;
            } else {
                pay_shell_output_msg(PAY_SHELL_EVENT_PAY_CLIENT_AUTH_SUCCESS, ret);
            }

Pay_Shell_Reload:
            //load pay client
            ret = pay_shell_load(key_name);
            if (ret != PAY_SHELL_SUCCESS) {
                LogPayShellDebug("%s\n",pay_shell_string_error(ret));
                pay_shell_output_msg(PAY_SHELL_EVENT_ERROR, ret);
                continue;
            } else {
                pay_shell_output_msg(PAY_SHELL_EVENT_PAY_CLIENT_LOAD_SUCCESS, ret);
            }

            status = PAY_SHELL_STATUS_LOAD_FINISH;
#else
Pay_Shell_Redownload:
Pay_Shell_Reload:
            g_psnode.ioctl = 0xffffffff;
#endif

        }
        else if (msg.cmd == PAY_SHELL_CMD_UNLOAD) {
            if (status != PAY_SHELL_STATUS_LOAD_FINISH) {
                LogPayShellDebug("STATUS is %d\n", status);
                pay_shell_output_msg(PAY_SHELL_EVENT_ERROR, PAY_SHELL_ERROR_STATUS);
                continue;
            }

            LogPayShellDebug("Pay shell received a message; msg is %s\n", msg.msg);

            ret = pay_shell_unload(&g_psnode);
            if (ret != PAY_SHELL_SUCCESS) {
                LogPayShellDebug("%s\n",pay_shell_string_error(ret));
                pay_shell_output_msg(PAY_SHELL_EVENT_ERROR, ret);
                continue;
            }

            sprintf(cmd_rm, "rm -f %s/*", DEFAULT_MODULE_PAY_DATAPATH);
            system(cmd_rm);

            memset(payClientDomain, 0, sizeof(payClientDomain));
            memset(key_name, 0, sizeof(key_name));
            memset(version, 0, sizeof(version));
            memset(location, 0, sizeof(location));
            memset(url, 0, sizeof(url));
            memset(cmd_rm, 0, sizeof(cmd_rm));

            //g_handle = NULL;

            status = PAY_SHELL_STATUS_UNLOAD;
        }
        else if (msg.cmd == PAY_SHELL_CMD_REDOWNLOAD) {
            if (status != PAY_SHELL_STATUS_LOADING) {
                LogPayShellDebug("STATUS is %d\n", status);
                pay_shell_output_msg(PAY_SHELL_EVENT_ERROR, PAY_SHELL_ERROR_STATUS);
                continue;
            }

            memset(cmd_rm, 0, sizeof(cmd_rm));
            sprintf(cmd_rm, "rm -f %s/*", DEFAULT_MODULE_PAY_DATAPATH);
            system(cmd_rm);

            goto Pay_Shell_Redownload;

        }
        else if (msg.cmd == PAY_SHELL_CMD_RELOAD) {
            if (status != PAY_SHELL_STATUS_LOADING) {
                LogPayShellDebug("STATUS is %d\n", status);
                pay_shell_output_msg(PAY_SHELL_EVENT_ERROR, PAY_SHELL_ERROR_STATUS);
                continue;
            }

            goto Pay_Shell_Reload;

        }
        else {
            LogPayShellDebug("ERROR: CMD is INVALID!\n");
        }
    }
}
