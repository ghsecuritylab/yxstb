/*******************************************************************************
    公司：
            Yuxing software
    纪录：
            2008-1-26 21:12:26 create by Liu Jianhua
    模块：
            tr069
    简述：
            输出接口
 *******************************************************************************/

#include "tr069_api.h"
#include "tr069_port.h"
#include "tr069_global.h"
#include "tr069_header.h"

typedef struct tagOperation Operation;
struct tagOperation {
    Operation *next;

    char *name;
    int len;
    Tr069ValueFunc func;
};

static Operation *gOperation = NULL;

static struct TR069 *g_tr069 = NULL;
static int g_init = 0;

int g_tr069LogLevel = 4;

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
static void *int_api_task(void *arg)
{
    if (!g_tr069)
        TR069ErrorOut("tr069 not init\n");
    TR069Printf("========\n");
    tr069_task_loop(g_tr069);
Err:
    return (void *)0;
}

//------------------------------------------------------------------------------
void tr069_api_message(unsigned int id, int arg0, int arg1)
{
    struct Message msg;

    msg.id = id;
    msg.arg0 = arg0;
    msg.arg1 = arg1;
    tr069_event_push(g_tr069, &msg);
}

static void int_api_testList(struct Param *param)
{
    TR069Printf("%s\n", param->name);
    if (TR069_OBJECT == param->type) {
        param = param->prm_objValue.firstChild;
        while (param) {
            int_api_testList(param);
            param = param->treeNext;
        }
    }
}

static int int_api_setValueTest(char *name, char *str, unsigned int val)
{
    name += 5;
    if (!strcmp(name, "Parse")) {
        tr069_test_parse(g_tr069, str, val);
    } else if (!strcmp(name, "List")) {
        struct Param *param = g_tr069->table_array[0];
        if (param) {
            pthread_mutex_lock(&g_tr069->mutex);
            int_api_testList(param);
            pthread_mutex_unlock(&g_tr069->mutex);
        }
    } else {
        TR069Error("Test. name = %s\n", name);
        return -1;
    }
    return 0;
}

static void int_api_eventLoad(int isDownload, char *fileType, int code)
{
    int sn;
    struct Load *load;

    TR069Printf("isDownload = %d, fileType = %s\n", isDownload, fileType);

    pthread_mutex_lock(&g_tr069->mutex);
    load = g_tr069->loadQueue;
    if (load) {
        if (isDownload == load->isDownload && !strcmp(fileType, load->FileType))
            sn = load->SN;
        else
            load = NULL;
    }
    pthread_mutex_unlock(&g_tr069->mutex);

    if (load)
        tr069_api_message(EVENTCODE_Y_LOAD, sn, code);
    else
        TR069Error("load = %p, isDownload = %d, fileType = %s\n", load, isDownload, fileType);
}

//------------------------------------------------------------------------------
/*
URL
BackupURL
Username
Password
PeriodicInformEnable
PeriodicInformInterval
ConnectionPath
ConnectionUsername
ConnectionPassword
 */
static int int_api_setValueConfig(char *name, char *str, unsigned int val)
{
    name += 7;

    if (!strcmp(name, "Save")) {
        if (!g_tr069)
            TR069ErrorOut("not init! name = %s\n", name);
        pthread_mutex_lock(&g_tr069->mutex);
        tr069_config_save(g_tr069);
        pthread_mutex_unlock(&g_tr069->mutex);
        return 0;
    }
    if (!strcmp(name, "Reset")) {
        tr069_api_message(EVENTCODE_Y_SUSPEND, 0, 0);
        pthread_mutex_lock(&g_tr069->mutex);
        tr069_config_reset(g_tr069);
        tr069_config_save(g_tr069);
        pthread_mutex_unlock(&g_tr069->mutex);
        return 0;
    }

    if (!strcmp(name, "Bootstrap")) {
        g_tr069_bootstrap = (int)val;
    } else if (!strcmp(name, "DigestAuth")) {
        g_tr069_digestAuth = (int)val;
    } else if (!strcmp(name, "TargetUpgrade")) {
        g_tr069_targetUpgrade = (int)val;
    } else if (!strcmp(name, "ParamPedant")) {
        g_tr069_paramPedant = (int)val;
    } else if (!strcmp(name, "HoldCookie")) {
        g_tr069_holdCookie = (int)val;
    } else if (!strcmp(name, "HTTPTimeout")) {
        if (val > 2)
            g_tr069_httpTimeout = (int)val;
    } else if (!strcmp(name, "STUNUsername")) {
        extern int needUserNameAttr;
        if(val)
            needUserNameAttr = val;
    } else if (!strcmp(name, "ParamPath")) {
        if (g_tr069_paramPath)
            free(g_tr069_paramPath);
        g_tr069_paramPath = strdup(str);
    } else if (!strcmp(name, "ConfigPath")) {
        tr069_port_infoPath(str);
    } else if (!strcmp(name, "LogLevel")) {
        g_tr069LogLevel = val;
    } else {
        TR069ErrorOut("Config. name = %s\n", name);
    }
    return 0;
Err:
    return -1;
}

int tr069_api_registFunction(char *name, Tr069ValueFunc getValue, Tr069ValueFunc setValue)
{
    struct Param* param;

    if (!g_init)
        TR069ErrorOut("init = %d, name = %s\n", g_init, name);

    param = tr069_param_hash_find(g_tr069, name);
    if (!param)
        TR069ErrorOut("not find! name = %s\n", name);

    if (TR069_OBJECT == param->type) {
        struct Param *prm = param->prm_objVirtual.firstChild;
        while (prm) {
            prm->prm_getval = getValue;
            prm->prm_setval = setValue;
            prm = prm->treeNext;
        }
    } else {
        param->prm_getval = getValue;
        param->prm_setval = setValue;
    }

    return 0;
Err:
    return -1;
}

int tr069_api_registOperation(char *name, Tr069ValueFunc func)
{
    int l, len;
    Operation *op;

    if (!g_init)
        TR069ErrorOut("init = %d, name = %s\n", g_init, name);

    len = strlen(name);
    op = gOperation;

    while (op) {
        if (name[0] == op->name[0]) {
            l = op->len;
            if (l > len)
                l = len;
            if (!strncmp(name, op->name, l))
                TR069ErrorOut("name = %s / %s\n", name, op->name);
        }
        op = op->next;
    }

    op = (Operation *)malloc(sizeof(Operation));
    if (!op)
        TR069ErrorOut("malloc name = %s\n", name);

    op->name = strdup(name);
    op->len = len;
    op->func = func;
 
    op->next = gOperation;
    gOperation = op;

    return 0;
Err:
    return -1;
}

static int int_api_setValueDevice(char *name, char *str, unsigned int val)
{
    int ret, idx;

    pthread_mutex_lock(&g_tr069->mutex);
    idx = tr069_param_index(g_tr069, name);
    if (idx > 0)
        ret = tr069_param_write(g_tr069, name, str, val);
    else
        ret = -1;
    pthread_mutex_unlock(&g_tr069->mutex);


    if (ret == 1) {
        if (0 == strncmp(name, "Device.ManagementServer.", 24)) {
            char *n = name + 24;
            if (0 == strcmp(n, "URL")
                || 0 == strcmp(n, "Username")
                || 0 == strcmp(n, "Password")
                || 0 == strcmp(n, "PeriodicInformEnable\n")
                || 0 == strcmp(n, "PeriodicInformInterval\n")
                || 0 == strcmp(n, "ConnectionRequestUsername\n")
                || 0 == strcmp(n, "ConnectionRequestPassword\n"))
                ret = 0;
        }
        if (ret)
            tr069_api_message(EVENTCODE_Y_VALUE_CHANGE, idx, val);
    } else if (ret) {
       TR069ErrorOut("tr069_param_write\n");
    }

    return 0;
Err:
    return -1;
}

static int int_api_setValueTask(char *name, char *str, unsigned int val)
{
    name += 5;
    if (!strcmp(name, "Active"))
        tr069_api_message(EVENTCODE_Y_ACTIVE, 0, 0);
    else if (!strcmp(name, "Suspend"))
        tr069_api_message(EVENTCODE_Y_SUSPEND, 0, 0);
    else if (!strcmp(name, "Connect"))
        tr069_api_message(EVENTCODE_CONNECTION_REQUEST, 0, 0);
    else
        TR069ErrorOut("Task. name = %s\n", name);

    return 0;
Err:
    return -1;
}

static int int_api_setValueEvent(char *name, char *str, unsigned int val)
{
    name += 6;
    if (!strcmp(name, "ValueChange")) {
        int idx = tr069_param_index(g_tr069, str);
        if (idx <= 0)
            TR069ErrorOut("ValueChange. param = %s\n", str);
        TR069Printf("EVENTCODE_VALUE_CHANGE %s\n", str);
        tr069_api_message(EVENTCODE_VALUE_CHANGE, idx, 0);
        return 0;
    }

    if (!strcmp(name, "AddObject"))
        return tr069_global_addObject(str);
    if (!strcmp(name, "DeleteObject"))
        return tr069_global_deleteObject(str, val);

    if (!strcmp(name, "Regist"))
        return tr069_global_eventRegist(str);

    if (!strcmp(name, "Parameter"))
        tr069_global_eventParam(val, str);
    else  if (!strcmp(name, "Post")) {
        if (str && strlen(str) > 0 && '0' <= str[0] && str[0] <= '9') {
            TR069Printf("string EventID = %s\n", str);
            val = atoi(str);
        }
        tr069_global_eventPost(val);
    }
    else if (!strcmp(name, "Shutdown"))
        tr069_api_message(EVENTCODE_Y_SHUTDOWN, 0, 0);
    else
        TR069ErrorOut("Event. name = %s\n", name);

    return 0;
Err:
    return -1;
}

static int int_api_setValueMessage(char *name, char *str, unsigned int val)
{
    name += 8;
    if (!strncmp(name, "Download.", 9))
        int_api_eventLoad(1, name + 9, (int)val);
    else if (!strcmp(name, "Upload."))
        int_api_eventLoad(0, name + 7, (int)val);
    else
        TR069ErrorOut("Message. name = %s\n", name);

    return 0;
Err:
    return -1;
}

int tr069_api_setValue(char *name, char *str, unsigned int val)
{
    Operation *op;

    if (str) {
        if (strstr(name, "Password"))
            TR069Printf("name = %s, str = *, val = %d\n", name, val);
        else
            TR069Printf("name = %s, str = %s, val = %d\n", name, str, val);
    } else {
        TR069Printf("name = %s, val = %d\n", name, val);
    }

    op = gOperation;
    while (op) {
        if (!strncmp(name, op->name, op->len))
            break;
        op = op->next;
    }
    if (!op) {
        if (!strncmp(name, "Config.", 7))//Config 在未调用tr069_api_init之前就要处理，所以只能单独
            return int_api_setValueConfig(name, str, val);
        TR069Error("unregist! name = %s\n", name);
        return -1;
    }
    if (!g_tr069)
        return -1;

    return op->func(name, str, val);



}

//------------------------------------------------------------------------------
int tr069_api_getValue(char *name, char *str, unsigned int size)
{
    int err = -1;
    if (str)
        str[0] = 0;

    if (!g_tr069)
        TR069ErrorOut("tr069 not init\n");
    TR069Printf("name = %s, size = %d\n", name, size);

    if (!strncmp(name, "Device.", 6)) {
        pthread_mutex_lock(&g_tr069->mutex);
        err = tr069_param_read(g_tr069, name, str, size);
        pthread_mutex_unlock(&g_tr069->mutex);
    } else if (!strcmp(name, "Opaque")) {
        err = tr069_global_getOpaque(str, size);
    } else if (!strcmp(name, "Event.Posting")) {
        err = tr069_global_eventPosting(size);
        sprintf(str, "%d", err);
        err = 0;
    } else if (!strncmp(name, "EventID.", 8)) {
        err = tr069_global_eventRegist(name + 8);
        sprintf(str, "%d", err);
        err = 0;
    }

    if (err)
        TR069ErrorOut("read %s\n", name);

    if (strstr(name, "Password"))
        TR069Printf("value = *\n");
    else
        TR069Printf("value = %s\n", str);

Err:
    return err;
}

int tr069_api_init(void)
{
    pthread_t thrd;

    if (g_tr069)
        TR069ErrorOut("g_tr069 = %p\n", g_tr069);
    TR069Printf("========\n");

    g_init = 1;

    g_tr069 = tr069_struct_create( );
    if (!g_tr069)
        TR069ErrorOut("tr069_struct_create\n");

    tr069_api_registOperation("Test.", int_api_setValueTest);
    tr069_api_registOperation("Task.", int_api_setValueTask);
    tr069_api_registOperation("Device.", int_api_setValueDevice);
    tr069_api_registOperation("Message.", int_api_setValueMessage);
    tr069_api_registOperation("Event.", int_api_setValueEvent);
    tr069_api_setValue("Event.Regist", "8 DIAGNOSTICS COMPLETE", 0);

    tr069_port_moduleInit( );

    g_init = 0;

    pthread_create(&thrd, NULL, int_api_task, NULL);
    pthread_create(&thrd, NULL, stun_api_task, NULL);


    return 0;
Err:
    return -1;
}

