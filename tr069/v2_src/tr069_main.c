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

#include "tr069_ftp.h"
#include "tr069_header.h"

#include<string.h>

#define TR069_INTERVAL_MIN_15SEC    15
#define TR069_INTERVAL_RETRY_5SEC    5

static const char DiagState[][32] = {
    "Complete",
    "Error_CannotResolveHostName",
    "Error_MaxHopCountExceeded",
    "Error_Internal",
    "Error_Other",
};

typedef int (*ResponseFunc)(struct TR069 *tr069);

static int int_task_active(struct TR069 *tr069);
static int int_task_suspend(struct TR069 *tr069);

int g_tr069_digestAuth = 1;
//内蒙古局点没有bootstrap
int g_tr069_bootstrap = 1;

int g_tr069_targetUpgrade = 0;

static void tr069_retry_clear(struct TR069 *tr069)
{
    if (tr069->retry_interval != 0) {
        tr069->retry_interval = 0;
        tr069->retry_time = 0;
    }
}

//空函数，保证到时执行
static void tr069_retry_ontime(struct TR069 *tr069)
{
    TR069Printf("\n");
}

static void tr069_retry_reset(struct TR069 *tr069)
{
    int t = (int)tr069_sec( );
    if (tr069->retry_interval == 0)
        tr069->retry_interval = TR069_INTERVAL_RETRY_5SEC;
    else
        tr069->retry_interval *= 2;

    if (tr069->retry_connect < 0) {
        //TR069标准
        tr069->retry_time = t + tr069->retry_interval + (t * tr069->random_factor) % tr069->retry_interval;
    } else {
        //华为定制
        tr069->retry_time += tr069->retry_interval;
        if (tr069->retry_time < t)
           tr069->retry_time = t;
    }
    tr069_timer_create(tr069, tr069->retry_time, tr069_retry_ontime);
    tr069->retry_count ++;
}

static void tr069_create_url(struct TR069 *tr069)
{
    int len;

    len = sprintf(tr069->buf, "http://");
    tr069_port_getValue("Device.LAN.IPAddress", tr069->buf + len, TR069_BUF_SIZE_4096 - 8);
    len = (int)strlen(tr069->buf);
    len += sprintf(tr069->buf + len, ":7547/");
    tr069_param_read(tr069, "Device.ManagementServer.ConnectionRequestPath", tr069->buf + len, TR069_BUF_SIZE_4096 - len);
    tr069_param_write(tr069, "Device.ManagementServer.ConnectionRequestURL", tr069->buf, 0);

    //TR069Printf("ConnectionRequestURL = %s\n", tr069->buf);
}

//------------------------------------------------------------------------------
static void tr069_event_inset(struct TR069 *tr069, uint32_t code, int arg0, int arg1)
{
    switch(code) {
    case EVENTCODE_BOOTSTRAP:
    case EVENTCODE_BOOT:
    case EVENTCODE_PERIODIC:
    case EVENTCODE_SCHEDULED:
    case EVENTCODE_VALUE_CHANGE:
    case EVENTCODE_KICKED:
    case EVENTCODE_CONNECTION_REQUEST:
    case EVENTCODE_TRANSFER_COMPLETE:
    case EVENTCODE_DIAGNOSTICS_COMPLETE:
    case EVENTCODE_M_SHUTDOWN:
        break;
    case EVENTCODE_M_REBOOT:
    case EVENTCODE_M_SCHEDULE:
    case EVENTCODE_M_DOWNLOAD:
    case EVENTCODE_M_UPLOAD:
    {
        char *key = (char *)arg0;
        if (!key)
            TR069ErrorOut("key is NULL\n");
        if (strlen(key) > TR069_COMMANDKEY_LEN_32)
            TR069ErrorOut("key too large\n");
        strcpy(tr069->event_array[code].CommandKey, key);
        break;
    }
    case EVENTCODE_X_BASICINFO:
    case EVENTCODE_X_DOWNLOADCOMPLETE:
    case EVENTCODE_X_UPLOADCOMPLETE:
        return;

    case EVENTCODE_X_EXTEND:
        {
            GlobalEvent *event = tr069->event_global;
            while (event) {
                if (arg0 == event->eventID)
                    break;
                event = event->next;
            }
            if (!event) {
                event = (GlobalEvent *)IND_CALLOC(sizeof(GlobalEvent), 1);
                event->next = tr069->event_global;
                tr069->event_global = event;

                event->eventID = arg0;
            }
            tr069_global_eventSync(event);
        }
        return;

    case EVENTCODE_Y_OBJECT_ADD:
        {
            int isFound;
            char name[1024];
            name[0] = 0;
            isFound = tr069_global_findObject(arg0, arg1, name, 1024);
            if (isFound)
                tr069_object_add(tr069, name, arg1);
            TR069Printf("AddObject = %s, isFound = %d, instance = %d\n", name, isFound, arg1);
        }
        return;
    case EVENTCODE_Y_OBJECT_DELETE:
        {
            int isFound;
            char name[1024];
            name[0] = 0;
            isFound = tr069_global_findObject(arg0, arg1, name, 1024);
            if (!isFound) {
                struct Param *param;
                int len = strlen(name);

                if (0 != name[0] && arg1 > 0)
                    snprintf(name + len, TR069_BUF_SIZE_4096 - len, "%d.", arg1);
                param = tr069_param_hash_find(tr069, name);
                if (param)
                    tr069_object_delete(tr069, param);
            }

            TR069Printf("DeleteObject %s, isFound = %d, instance = %d\n", name, isFound, arg1);
        }
        return;

    case EVENTCODE_Y_ACTIVE:
        int_task_active(tr069);
        return;
    case EVENTCODE_Y_SUSPEND:
        int_task_suspend(tr069);
        return;
    case EVENTCODE_Y_GETRPCMETHODS:
    case EVENTCODE_Y_SCHEDULED:
        if (code == EVENTCODE_Y_GETRPCMETHODS)
            tr069->flag_getrpc = 1;
        if (tr069->state == TR069_STATE_SUSPEND)
            return;
        code = EVENTCODE_SCHEDULED;
        tr069->state = TR069_STATE_ACTIVE;
        tr069_retry_clear(tr069);
        break;
    case EVENTCODE_Y_CONFIG_PRINT:
        tr069_config_file_print(tr069);
        return;
    case EVENTCODE_Y_VALUE_CHANGE:
        return;
    case EVENTCODE_Y_SHUTDOWN:
        tr069_event_inset(tr069, EVENTCODE_M_SHUTDOWN, 0, 0);
        return;
    default:
        TR069ErrorOut("code = %d\n", code);
    }
    if (code >= EVENTCODE_MAX)
        TR069ErrorOut("code = %d\n", code);
    if (tr069->event_array[code].userFlag == 0)
        tr069->event_array[code].userFlag = 1;

    if (tr069->event_inform == EVENT_INFORM_NONE)
        tr069->event_inform = EVENT_INFORM_VALID;

Err:
    return;
}

static void tr069_event_on_change(struct TR069 *tr069)
{
    TR069Printf("EVENTCODE_VALUE_CHANGE\n");
    tr069_event_inset(tr069, EVENTCODE_VALUE_CHANGE, 0, 0);
}

static void tr069_event_on_save(struct TR069 *tr069)
{
    if (tr069->save_flag == 1) {
        TR069Printf("SYS Save\n");
        tr069_config_save(tr069);
    }
}

//------------------------------------------------------------------------------
static int tr069_event_change(struct TR069 *tr069, int index, int current, int delay)
{
    struct Param *param;
    int notification;

    if (index <= 0 || index > tr069->table_size)
        TR069ErrorOut("index = %d\n", index);

    if (index == g_tr106Index->ManagementServer_ConnectionRequestPath_index)
        index = g_tr106Index->ManagementServer_ConnectionRequestURL_index;
    param = tr069->table_array[index];
    notification = param->currentNotification;

    //TR069Printf("index = %d, notification = %d\n", index, notification);
    if (notification == NOTIFICATION_OFF) {
        tr069_param_read_cksum(tr069, param, &param->cksum);
        param->cksum_flag = 1;
        return 0;//Notification off
    }

    tr069_timer_create(tr069, current + 2, tr069_event_on_save);
    tr069_paramChange_inset(tr069, param);

    if (notification != NOTIFICATION_ACTIVE)
        return 0;

    if (delay > 0)
        tr069_timer_create(tr069, current + delay, tr069_event_on_change);
    else
        tr069_event_on_change(tr069);

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
void tr069_event_push(const struct TR069 *tr069, struct Message *msg)
{
    if (!tr069 || !msg)
        return;
    TR069Printf("send message id = %d\n", msg->id);
    tr069_msgq_putmsg(tr069->msgqPipe, (char *)msg);
}

static int int_http_connect(struct TR069 *tr069)
{
    int ret;
    char url[HTTP_URI_LEN];
    struct Digest *digest;
    struct Http *http;

    TR069Printf("tr069_http_connect\n");

    http = tr069->http;
    tr069_http_init(http);

    digest = &(http->digest);

    pthread_mutex_lock(&tr069->mutex);
    strcpy(digest->method, "POST");

    /*
        0024211华为 URLBackup
        STB默认连接主ACS地址，如果连接无法建立，则进行重试，重试两次（一共3次），
        如果三次均失败之后向连接备ACS地址，如果连接备ACS地址3次也失败则停止连接。
     */
    url[0] = 0;
    tr069_param_read(tr069, "Device.ManagementServer.URLBackup", url, HTTP_URI_LEN);
    if (!strnicmp(url, "http://", 7)) {
        if (tr069->retry_connect < 0) {
            tr069->retry_connect = 0;
        } else {
            if (tr069->retry_connect >= 6) {
                TR069Error("retry_connect = %d\n", tr069->retry_connect);
                tr069->retry_connect = 0;
            }
            if (tr069->retry_connect == 3)
                tr069->retry_interval = 0;
        }
    }
    if (tr069->retry_connect < 3)
        tr069_param_read(tr069, "Device.ManagementServer.URL", url, HTTP_URI_LEN);
    tr069_param_read(tr069, "Device.ManagementServer.Username", digest->username, DIGEST_PARAM_LEN);
    tr069_param_read(tr069, "Device.ManagementServer.Password", digest->password, DIGEST_PARAM_LEN);

    ret = tr069_checkurl(url, digest->uri, HTTP_URI_LEN, http->host, HTTP_HOST_LEN, &http->port);
    pthread_mutex_unlock(&tr069->mutex);
    if (ret)
        TR069ErrorOut("checkurl %s\n", url);

    TR069Printf("retry = %d, connect %s ...\n", tr069->retry_connect, url);
    tr069->retry_time = tr069_sec( );
    if (tr069_http_connect(tr069->http))
        TR069ErrorOut("connect %s\n", url);
    TR069Printf("connect OK\n");

    if (tr069->state == TR069_STATE_ERROR_CONNECT) {
        tr069->state = TR069_STATE_ACTIVE;
        if (tr069->retry_connect > 0 && tr069->retry_connect != 3) {
            if (tr069->retry_connect > 3)
                tr069->retry_connect = 3;
            else
                tr069->retry_connect = 0;
        }
        tr069_retry_clear(tr069);
    }
    return 0;
Err:
    if (tr069->state != TR069_STATE_SUSPEND) {
        if (tr069->retry_connect >= 0)
            tr069->retry_connect++;
        tr069->state = TR069_STATE_ERROR_CONNECT;
        tr069_retry_reset(tr069);
    }
    return -1;
}

static void int_http_disconnect(const struct TR069 *tr069)
{
    tr069_http_disconnect(tr069->http);
    TR069Printf("tr069_http_disconnect\n");
}

static int int_http_transfer(const struct TR069 *tr069)
{
    int ret;
    struct Http *http = tr069->http;

    strcpy(http->header, "SOAPAction: \r\nContent-Type: text/xml\r\n");
    ret = tr069_http_transfer(http, tr069->soap->buffer, (int *)&(tr069->soap->length), SOAP_ENVELOPE_LEN_128K + 1);

    return ret;
}

//------------------------------------------------------------------------------
struct TR069 *tr069_struct_create(void)
{
    struct TR069 *tr069 = NULL;

    tr069 = (struct TR069 *)IND_CALLOC(sizeof(struct TR069), 1);
    if (!tr069)
        TR069ErrorOut("malloc tr069\n");

    tr069->retry_connect = -1;
    tr069->sock_request = INVALID_SOCKET;

    tr069->table_size = DEVICE_INDEX_NUM_2048;
    tr069->table_array = (struct Param **)IND_CALLOC(sizeof(struct Param *) * tr069->table_size, 1);//按序号排列

    tr069->http = (struct Http *)IND_MALLOC(sizeof(struct Http));
    tr069_http_init(tr069->http);

    tr069->http_a = (struct Http *)IND_MALLOC(sizeof(struct Http));
    tr069_http_init(tr069->http_a);

    tr069->soap = (struct Soap *)IND_CALLOC(sizeof(struct Soap), 1);

    tr069_timer_init(tr069);

    tr069_tr106_init(tr069);
    tr069_param_load(tr069);

    tr069_config_init(tr069);

    pthread_mutex_init(&tr069->mutex, NULL);
    tr069->msgqPipe = tr069_msgq_create(20, sizeof(struct Message));
    if (!tr069->msgqPipe)
        TR069ErrorOut("tr069_msgq_create\n");
    tr069->msgqSock = tr069_msgq_fd(tr069->msgqPipe);

    tr069_config_load(tr069);

    {
        struct Reboot *reboot = &tr069->reboot;
        if (reboot->rebootstate) {
            reboot->rebootstate = REBOOT_STATE_COMPLETE;
            TR069Printf("EVENTCODE_M_REBOOT\n");
            tr069_event_inset(tr069, EVENTCODE_M_REBOOT, (int)(reboot->commandkey), 0);
        }
    }

    if (tr069->loadQueue) {//上传下载结果上报
        struct Load *load = tr069->loadQueue;

        if (TRANSFER_STATE_COMPLETE == load->transferstate) {
            //TR069Printf("EVENTCODE_TRANSFER_COMPLETE: %s\n", load->url);
            TR069Printf("EVENTCODE_TRANSFER_COMPLETE\n");
            tr069_event_inset(tr069, EVENTCODE_TRANSFER_COMPLETE, 0, 0);
            if (load->isDownload)
                tr069_event_inset(tr069, EVENTCODE_M_DOWNLOAD, (int)(load->CommandKey), 0);
            else
                tr069_event_inset(tr069, EVENTCODE_M_UPLOAD, (int)(load->CommandKey), 0);
        } else {
            load->loadtime = 0;
        }
    }
    tr069_port_getValue("Device.DeviceInfo.SerialNumber", tr069->buf, TR069_BUF_SIZE_4096);
    tr069->random_factor = atoi(tr069->buf + strlen(tr069->buf) - 6);

    return tr069;
Err:
    if (tr069) {
        if (tr069->http)
            IND_FREE(tr069->http);
        if (tr069->http_a)
            IND_FREE(tr069->http_a);
        if (tr069->soap)
            IND_FREE(tr069->soap);
        IND_FREE(tr069);
    }
    return NULL;
}

//------------------------------------------------------------------------------
static int int_parse(struct TR069 *tr069)
{
    uint32_t cmdlen;
    int ret = -1;
    struct Taget *tag;
    char *cmd;
    struct Soap *soap = tr069->soap;
    ResponseFunc response = NULL;

    //fault reset
    tr069->fault_code = -1;
    tr069->fault_args_num = 0;

    if (soap_in_envelope_begin(soap, tr069->cwmp_id_req, TR069_NAME_SIZE_64 + 1))
        TR069ErrorOut("soap_in_envelope_begin\n");

    tag = soap_tag(soap);
    cmd = tag->name;
    cmdlen = tag->namelen;
    TR069Printf("s > c: %s\n", cmd);

    pthread_mutex_lock(&tr069->mutex);

    if (cmdlen == 14 && memcmp(cmd, "InformResponse", 14) == 0)
    {
        ret = tr069_s2c_inform_response(tr069);
        response = tr069_c2s_default;
    }    
    else if (cmdlen == 24 && memcmp(cmd, "TransferCompleteResponse", 24) == 0)
    {
        ret = tr069_s2c_transfercomplete_response(tr069);
        response = tr069_c2s_default;
    }
    else if (cmdlen == 13 && memcmp(cmd, "GetRPCMethods", 13) == 0)
    {
        ret = tr069_s2c_getrpcmethods(tr069);
        response = tr069_c2s_getrpcmethods_response;
    }
    else if (cmdlen == 21 && memcmp(cmd, "GetRPCMethodsResponse", 21) == 0)
    {
        ret = tr069_s2c_getrpcmethods_response(tr069);
        response = tr069_c2s_default;
    }
    else if (cmdlen == 18 && memcmp(cmd, "SetParameterValues", 18) == 0)
    {
        ret = tr069_s2c_setparametervalues(tr069);
        response = tr069_c2s_setparametervalues_response;
    }
    else if (cmdlen == 18 && memcmp(cmd, "GetParameterValues", 18) == 0)
    {
        ret = tr069_s2c_getparametervalues(tr069);
        response = tr069_c2s_getparametervalues_response;
    }
    else if (cmdlen == 17 && memcmp(cmd, "GetParameterNames", 17) == 0)
    {
        ret = tr069_s2c_getparameternames(tr069);
        response = tr069_c2s_getparameternames_response;
    }
    else if (cmdlen == 22 && memcmp(cmd, "GetParameterAttributes", 22) == 0)
    {
        ret = tr069_s2c_getparameterattributes(tr069);
        response = tr069_c2s_getparameterattributes_response;
    }
    else if (cmdlen == 22 && memcmp(cmd, "SetParameterAttributes", 22) == 0)
    {
        ret = tr069_s2c_setparameterattributes(tr069);
        response = tr069_c2s_setparameterattributes_response;
    }
    else if (cmdlen == 9 && memcmp(cmd, "AddObject", 9) == 0)
    {
        ret = tr069_s2c_addobject(tr069);
        response = tr069_c2s_addobject_response;
    }
    else if (cmdlen == 12 && memcmp(cmd, "DeleteObject", 12) == 0)
    {
        ret = tr069_s2c_deleteobject(tr069);
        response = tr069_c2s_deleteobject_response;
    }
    else if (cmdlen == 6 && memcmp(cmd, "Reboot", 6) == 0)
    {
        ret = tr069_s2c_reboot(tr069);
        response = tr069_c2s_reboot_response;
    }
    else if (cmdlen == 8 && memcmp(cmd, "Download", 8) == 0)
    {
        ret = tr069_s2c_download(tr069);
        response = tr069_c2s_download_response;
    }
    else if (cmdlen == 6 && memcmp(cmd, "Upload", 6) == 0)
    {
        ret = tr069_s2c_upload(tr069);
        response = tr069_c2s_upload_response;
    }
    else if (cmdlen == 12 && memcmp(cmd, "FactoryReset", 12) == 0)
    {
        ret = tr069_s2c_factoryreset(tr069);
        response = tr069_c2s_factoryreset_response;
    }
    else if (cmdlen == 14 && memcmp(cmd, "ScheduleInform", 14) == 0)
    {
        ret = tr069_s2c_scheduleinform(tr069);
        response = tr069_c2s_scheduleinform_response;
    }
    else if (cmdlen == 5 && memcmp(cmd, "Fault", 5) == 0)
    {
        ret = tr069_s2c_fault(tr069);
        response = NULL;
    }
    else
    {
        ret = 0;
        TR069Printf("%s unsupported\n", cmd);
        response = tr069_c2s_unsupported_response;
    }
    if (ret) {
        TR069Printf("ERROR!\n");
        tr069->fault_code = FAULT_INTERNAL_ERROR;
    }

    if (soap_in_envelope_end(soap))
        TR069Printf("soap_in_envelope_end\n");

    if (tr069->fault_code == -1) {
        tr069->soap->length = 0;
        if (response) {
            ret = response(tr069);
        } else {
            ret = 0;
        }
    } else {
        ret = tr069_c2s_fault(tr069, cmd);
    }

    pthread_mutex_unlock(&tr069->mutex);
    if (ret)
        TR069ErrorOut("ret = %d\n", ret);

    return 0;

Err:
    return -1;
}

//------------------------------------------------------------------------------
static int int_parse_loop(struct TR069 *tr069)
{
    int ret = 0;

    while (ret == 0) {
        if (int_http_transfer(tr069))
            TR069ErrorOut("tr069_http_transfer\n");
        if (tr069->soap->length == 0) {
            if (EVENT_INFORM_POSTING == tr069->event_inform) {
                TR069Warn("EVENT_INFORM_POSTING\n");
                tr069->event_inform = EVENT_INFORM_NONE;
            }
            TR069Printf("s > c: Empty\n");
            if (!tr069->bootstrap)
                TR069ErrorOut("no InformResponse\n");
            break;
        }
        ret = int_parse(tr069);

        if (ret == 1)
            break;
        if (ret == 0)
            continue;
        TR069ErrorOut("ret = %d\n", ret);
    }

    if (tr069->state != TR069_STATE_ACTIVE && tr069->state != TR069_STATE_SUSPEND) {
        tr069->state = TR069_STATE_ACTIVE;
        tr069_retry_clear(tr069);
    }
    tr069->retry_count = 0;
    return 0;
Err:
    if (tr069->state != TR069_STATE_SUSPEND)
        tr069->state = TR069_STATE_ERROR_PARSE;
    tr069_retry_reset(tr069);
    return -1;
}

//------------------------------------------------------------------------------
void tr069_test_parse(struct TR069 *tr069, const char *buf, uint32_t len)
{
    struct Soap *soap = tr069->soap;

    if (!buf || len >= SOAP_ENVELOPE_LEN_128K)
        TR069ErrorOut("buf = %p, len = %d\n", buf, len);

    memcpy(soap->buffer, buf, len);
    soap->length = len;
    soap->buffer[len] = 0;

    int_parse(tr069);

    TR069Debug("%s\n", tr069->soap->buffer);
Err:
    return;
}

static void int_schedule_ontime(struct TR069 *tr069)
{
    struct Schedule *schedule = &tr069->schedule;
    TR069Printf("EVENTCODE_SCHEDULED\n");
    tr069_event_inset(tr069, EVENTCODE_SCHEDULED, (int)(schedule->commandkey), 0);
    schedule->scheduletime = 0;
}

//TR069调度访问
void tr069_schedule_timer(struct TR069 *tr069)
{
    struct Schedule *schedule = &tr069->schedule;
    if (schedule->scheduletime)
        tr069_timer_create(tr069, schedule->scheduletime, int_schedule_ontime);
}

static void int_config_upload(struct TR069 *tr069, struct Load *load)
{
    int len, ret;
    uint32_t starttime;

    TR069Printf("EVENTCODE_TRANSFER_COMPLETE\n");
    tr069_event_inset(tr069, EVENTCODE_TRANSFER_COMPLETE, 0, 0);
    TR069Printf("EVENTCODE_M_UPLOAD\n");
    tr069_event_inset(tr069, EVENTCODE_M_UPLOAD, (int)(load->CommandKey), 0);
    load->transferstate = TRANSFER_STATE_COMPLETE;

    load->faultcode = FAULT_UPLOAD;
    starttime = time(NULL);

    len = tr069_config_file_create(tr069, tr069->buf_large, TR069_BUF_LARGE_SIZE_64K + 1);
    if (len < 0)
        TR069ErrorOut("len = %d\n", len);

    if (strnicmp(load->URL, "http://", 7) == 0) {
        struct Digest *digest;
        struct Http *http;

        http = tr069->http;
        tr069_http_init(http);
        digest = &http->digest;

        memset(&(http->digest), 0, sizeof(struct Digest));
        strcpy(http->digest.method, "POST");
        if (tr069_checkurl(load->URL, digest->uri, HTTP_URI_LEN, http->host, HTTP_HOST_LEN, &http->port))
            TR069ErrorOut("tr069_checkurl\n");
        strcpy(digest->username, load->Username);
        strcpy(digest->password, load->Password);
        TR069Printf("upload %s\n", load->URL);

        if (tr069_http_connect(http))
            TR069ErrorOut("tr069_http_connect\n");
        ret = tr069_http_transfer(http, tr069->buf_large, &len, TR069_BUF_LARGE_SIZE_64K + 1); 
        tr069_http_disconnect(http);
        if (ret)
            TR069ErrorOut("http_transfer ret = %d\n", ret);
    } else if (strnicmp(load->URL, "ftp://", 6) == 0) {
        if (0 == load->Username[0])
            ret = tr069_ftp_put(load->URL, "anonymous", "", tr069->buf_large, len);
        else
            ret = tr069_ftp_put(load->URL, load->Username, load->Password, tr069->buf_large, len);
        if (ret < 0)
            TR069ErrorOut("tr069_ftp_get\n");
    } else {
        TR069ErrorOut("unknow transport protocol: %s\n", load->URL);
    }

    load->starttime = starttime;
    load->completetime = time(NULL);
    load->faultcode = 0;

Err:
    return;
}

static void int_config_download(struct TR069 *tr069, struct Load *load)
{
    int len, size, ret;
    char *buf = NULL;
    uint32_t starttime;

    TR069Printf("EVENTCODE_TRANSFER_COMPLETE\n");
    tr069_event_inset(tr069, EVENTCODE_TRANSFER_COMPLETE, 0, 0);
    TR069Printf("EVENTCODE_M_DOWNLOAD\n");
    tr069_event_inset(tr069, EVENTCODE_M_DOWNLOAD, (int)(load->CommandKey), 0);

    load->transferstate = TRANSFER_STATE_COMPLETE;

    load->faultcode = FAULT_DOWNLOAD;
    starttime = time(NULL);

    buf = tr069->buf_large;
    size = TR069_BUF_LARGE_SIZE_64K + 1;

    if (strnicmp(load->URL, "http://", 7) == 0) {
        struct Digest *digest;
        struct Http *http;
    
        http = tr069->http;
        tr069_http_init(http);
        digest = &http->digest;

        memset(&(http->digest), 0, sizeof(struct Digest));
        strcpy(http->digest.method, "GET");
        if (tr069_checkurl(load->URL, digest->uri, HTTP_URI_LEN, http->host, HTTP_HOST_LEN, &http->port))
            TR069ErrorOut("tr069_checkurl\n");
        strcpy(digest->username, load->Username);
        strcpy(digest->password, load->Password);
        TR069Printf("download %s\n", load->URL);

        if (tr069_http_connect(http)) {
            load->faultcode = FAULT_DOWNLOAD_CONTACT;
            TR069ErrorOut("tr069_http_connect\n");
        }
        len = 0;
        ret = tr069_http_transfer(http, buf, &len, size);
        tr069_http_disconnect(http);

        if (ret) {
            if (http->code == 403 || http->code == 404)
                load->faultcode = FAULT_DOWNLOAD_ACCESS;
            else
                load->faultcode = FAULT_DOWNLOAD_COMPLETE;
            TR069ErrorOut("http_transfer\n");
        }
    } else if (strnicmp(load->URL, "ftp://", 6) == 0) {
        if (0 == load->Username[0])
            ret = tr069_ftp_get(load->URL, "anonymous", "", buf, size);
        else
            ret = tr069_ftp_get(load->URL, load->Username, load->Password, buf, size);
        if (ret <= 0) {
            load->faultcode = FAULT_DOWNLOAD_ACCESS;
            TR069ErrorOut("tr069_ftp_get\n");
        } else {
            len = ret;
        }
    } else {
        TR069ErrorOut("unknow transport protocol: %s\n", load->URL);
    }

    if (tr069_config_file_apply(tr069, buf, len)) {
        load->faultcode = FAULT_DOWNLOAD_CORRUPTED;
        TR069ErrorOut("tr069_cfgfile_apply");
    }

    load->starttime = starttime;
    load->completetime = time(NULL);
    load->faultcode = 0;
Err:
    return;
}

static void int_load(struct TR069 *tr069, int sn, int code)
{
    struct Load *load;

    load = tr069->loadQueue;
    if (!load)
        TR069ErrorOut("load not found\n");
    load->completetime = time(NULL);

    TR069Printf("code = %d\n", code);
    if (!code) {
        load->faultcode = 0;
    } else if (code >= 9000 && code < 9999) {
        load->faultcode = code;
    } else {
        if (load->isDownload)
            load->faultcode = FAULT_DOWNLOAD;
        else
            load->faultcode = FAULT_UPLOAD;
    }

    load->transferstate = TRANSFER_STATE_COMPLETE;

    TR069Printf("EVENTCODE_TRANSFER_COMPLETE\n");
    tr069_event_inset(tr069, EVENTCODE_TRANSFER_COMPLETE, 0, 0);
    if (load->isDownload) {
        TR069Printf("EVENTCODE_M_DOWNLOAD\n");
        tr069_event_inset(tr069, EVENTCODE_M_DOWNLOAD, (int)(load->CommandKey), 0);
    } else {
        TR069Printf("EVENTCODE_M_UPLOAD\n");
        tr069_event_inset(tr069, EVENTCODE_M_UPLOAD, (int)(load->CommandKey), 0);
    }

    tr069_config_save(tr069);

Err:
    return;
}

static void int_load_ontime(struct TR069 *tr069)
{
    struct Load *load;

    load = tr069->loadQueue;
    if (!load)
        return;

    if (TRANSFER_STATE_COMPLETE == load->transferstate) {
        TR069Printf("EVENTCODE_TRANSFER_COMPLETE\n");
        tr069_event_inset(tr069, EVENTCODE_TRANSFER_COMPLETE, 0, 0);
        if (load->isDownload)
            tr069_event_inset(tr069, EVENTCODE_M_DOWNLOAD, (int)(load->CommandKey), 0);
        else
            tr069_event_inset(tr069, EVENTCODE_M_UPLOAD, (int)(load->CommandKey), 0);
        return;
    }


    if (TRANSFER_STATE_INIT == load->transferstate && load->loadtime <= tr069_sec( )) {
        int l;
        char *p, *url, message[20 + TR069_FILETYPE_LEN_64];

        load->transferstate = TRANSFER_STATE_START;
        if (load->isDownload) {
            if (0 == strcasecmp(load->FileType, "3 Vendor Configuration File")) {
                int_config_upload(tr069, load);
                return;
            }
            sprintf(message, "Message.Download.%s", load->FileType);
        } else {
            if (0 == strcasecmp(load->FileType, "1 Vendor Configuration File")) {
                int_config_download(tr069, load);
                return;
            }
            sprintf(message, "Message.Upload.%s", load->FileType);
        }
        url = load->URL;
        if (load->URL[0]) {
            p = NULL;
            TR069Printf("Username = %s\n", load->Username);
            if (load->Username[0] && load->Password[0])
                p = strstr(load->URL, "://");
            if (p) {
                p += 3;
                l = p - load->URL;

                url = tr069->buf_large;
                memcpy(url, load->URL, l);
                sprintf(url + l, "%s:%s@%s", load->Username, load->Password, load->URL + l);
            }
        }
        TR069Printf("%s url = %s\n", message, url);
        tr069_port_setValue(message, url, 0);
    }
}

//下载上传计算
void tr069_load_timer(struct TR069 *tr069)
{
    struct Load *load;

    load = tr069->loadQueue;
    if (load) {
        if (load->transferstate == TRANSFER_STATE_INIT)
            tr069_timer_create(tr069, load->loadtime, int_load_ontime);
    }
}

static void int_periodic_ontime(struct TR069 *tr069)
{
    TR069Printf("EVENTCODE_PERIODIC\n");
    tr069_event_inset(tr069, EVENTCODE_PERIODIC, 0, 0);
    tr069_periodic_timer(tr069);
}

//TR069周期访问
void tr069_periodic_timer(struct TR069 *tr069)
{
    unsigned int localtime, periodenable, interval, periodictime;

    localtime = time(NULL);

    periodenable = tr069_tr106_getUnsigned("Device.ManagementServer.PeriodicInformEnable");
    interval     = tr069_tr106_getUnsigned("Device.ManagementServer.PeriodicInformInterval");
    periodictime = tr069_tr106_getUnsigned("Device.ManagementServer.PeriodicInformTime");

    TR069Printf("periodenable = %d, interval = %d\n", periodenable, interval);
    if (periodenable && interval >= TR069_INTERVAL_MIN_15SEC) {
        if (periodictime) {
            TR069Printf("periodictime = %u, localtime = %u\n", periodictime, localtime);
            if (periodictime > localtime) {
                unsigned int clk = (periodictime - localtime) % interval;
                if (clk > 0)
                    interval = clk;
            } else {
                interval = interval - (localtime - periodictime) % interval;
            }
        }
        TR069Printf("interval = %u\n", interval);
        tr069_timer_create(tr069, tr069_sec( ) + interval, int_periodic_ontime);
    }
}

//------------------------------------------------------------------------------
static void int_notify_sync(struct TR069 *tr069)
{
    int i;
    uint32_t cksum;
    struct Param *param;

    //初始化 param change list
    for (i = 0; i < tr069->table_size; i ++) {
        param = tr069->table_array[i];
        if (!param)
            continue;

        if (TR069_OBJECT == param->type)
            continue;
        if (NOTIFICATION_OFF == param->currentNotification)
            continue;
        cksum = 0;
        if (tr069_param_read_cksum(tr069, param, &cksum))
            continue;
        //TR069Debug("%s cksum = %08x\n", param->name, cksum);
        if (cksum != param->cksum && 1 == param->cksum_flag) {
            TR069Printf("%s cksum = %08x / %08x\n", param->name, cksum, param->cksum);
            if (tr069->bootstrap) {
                tr069_paramChange_inset(tr069, param);
                if (NOTIFICATION_ACTIVE == param->currentNotification) {
                    TR069Printf("EVENTCODE_VALUE_CHANGE\n");
                    tr069_event_inset(tr069, EVENTCODE_VALUE_CHANGE, 0, 0);
                }
            }
        }
    }
}

//------------------------------------------------------------------------------
static int int_task_active(struct TR069 *tr069)
{
    int current, sock = INVALID_SOCKET;
    struct sockaddr_in sa;

    TR069Printf("int_task_active\n");

    if (tr069->sock_request != INVALID_SOCKET)
        TR069ErrorOut("sock_request = %d\n", tr069->sock_request);

    memset((char *)&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(TR069_REQUEST_PORT);
    sa.sin_addr.s_addr = INADDR_ANY;
    sock = socket(AF_INET, SOCK_STREAM, 0);

    {
        int opt = 1;
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void*)&opt, sizeof(opt)) < 0)
          TR069ErrorOut("setsockopt errno = %d! %s\n", errno, strerror(errno));
    }

    if (bind(sock, (struct sockaddr *)&sa, sizeof(sa)))
        TR069ErrorOut("bind errno = %d! %s\n", errno, strerror(errno));
    if (listen(sock, SOMAXCONN) != 0)
        TR069ErrorOut("listen\n");

    tr069->sock_request = sock;

    tr069->state = TR069_STATE_ACTIVE;
    tr069_retry_clear(tr069);

    TR069Printf("EVENTCODE_BOOT\n");
    tr069_event_inset(tr069, EVENTCODE_BOOT, 0, 0);
    tr069->state_boot = 1;
    tr069->retry_connect = 0;

    tr069_periodic_timer(tr069);
    tr069_load_timer(tr069);
    tr069_schedule_timer(tr069);

    current = (int)tr069_sec( );
    tr069_timer_create(tr069, current + INTERVAL_CurrentDay_SEC, tr106_CurrentDay_ontime);
    tr069_timer_create(tr069, current + INTERVAL_QuarterHour_SEC, tr106_QuarterHour_ontime);

    tr069_tr106_SetParamValue("Device.ManagementServer.UDPConnectionRequestAddress", "", 0);

    {
        struct Param *param;
        while (tr069->prm_change) {
            param = tr069->prm_change;
            tr069->prm_change = param->changeNext;

            param->changeNext = NULL;
            param->isChange = 0;
        }
    }

    int_notify_sync(tr069);

    return 0;
Err:
    if (sock != INVALID_SOCKET)
        closesocket(sock);
    return -1;
}

//------------------------------------------------------------------------------
static int int_task_suspend(struct TR069 *tr069)
{
    TR069Printf("int_task_suspend\n");

    if (tr069->state == TR069_STATE_SUSPEND)
        TR069ErrorOut("TR069_STATE_SUSPEND\n");
    if (tr069->sock_request == INVALID_SOCKET) {
        tr069_retry_clear(tr069);
    } else {
        closesocket(tr069->sock_request);
        tr069->sock_request = INVALID_SOCKET;
    }
    tr069->state = TR069_STATE_SUSPEND;

    tr069_timer_delete(tr069, tr106_CurrentDay_ontime);
    tr069_timer_delete(tr069, tr106_QuarterHour_ontime);

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
static void int_task_wait(struct TR069 *tr069)
{
    int ret;
    SOCKET maxfd;
    int currenttime;
    fd_set rset;
    struct timeval tv = {0, 0};
    struct Http *http_a = tr069->http_a;
    static int delay = 60;

    while(1) {
        currenttime = (int)tr069_sec( );

        FD_ZERO(&rset);
        FD_SET((unsigned)tr069->msgqSock, &rset);
        maxfd = (SOCKET)tr069->msgqSock;
        tv.tv_sec = 365 * 24 * 3600;

        if (tr069->sock_request != INVALID_SOCKET) {
            FD_SET((unsigned)tr069->sock_request, &rset);
            if (tr069->sock_request > maxfd)
                maxfd = tr069->sock_request;
        }

        if (tr069->state != TR069_STATE_SUSPEND) {
            int timertime, sec;

            tr069_timer_deal(tr069, currenttime);
            timertime = tr069_timer_time(tr069);
            if (timertime != -1) {
                if (timertime <= currenttime)
                    TR069ErrorOut("timertime <= currenttime\n");

                sec = (int)(timertime - currenttime);
                if (tv.tv_sec > sec)
                    tv.tv_sec = sec;
            }
        }

        switch(tr069->state) {
        case TR069_STATE_ACTIVE:
            if (!tr069->bootstrap) {
                TR069Printf("SYS bootstrap = %d\n", g_tr069_bootstrap);
                if (g_tr069_bootstrap) {
                    TR069Printf("EVENTCODE_BOOTSTRAP\n");
                    tr069_event_inset(tr069, EVENTCODE_BOOTSTRAP, 0, 0);
                }
            }
            if (tr069->event_global || tr069->event_inform)
                return;
            break;

        case TR069_STATE_ERROR_CONNECT:
        case TR069_STATE_ERROR_PARSE:
            if (tr069->retry_time <= currenttime)
                return;
            break;
        default:
            break;
        }

        TR069Printf("tv.tv_sec = %ld, sock = %d\n", tv.tv_sec, tr069->sock_request);

        ret = select((int)(maxfd + 1), &rset, NULL, NULL, &tv);
        if (ret < 0)
            TR069ErrorOut("select\n");

        if (ret > 0) {
            currenttime = (int)tr069_sec( );
            if (FD_ISSET((unsigned)tr069->msgqSock, &rset)) {
                struct Message msg;
                if (tr069_msgq_getmsg(tr069->msgqPipe, (char *)(&msg)))
                    TR069ErrorOut("pipe_read\n");
                TR069Printf("recv message id = %d, arg0 = %d, arg1 = %d\n", msg.id, msg.arg0, msg.arg1);
                if (msg.id == EVENTCODE_Y_VALUE_CHANGE) {
                    if (1 == msg.arg1)
                        tr069_event_change(tr069, msg.arg0, currenttime, 0);
                    else
                        tr069_event_change(tr069, msg.arg0, currenttime, 2);
                } else {
                    if(EVENTCODE_Y_LOAD == msg.id) {
                        int_load(tr069, msg.arg0, msg.arg1);
                    } else {
                        /* Enable/disable STUN detected according to tr069 task state. */
                        if (msg.id == EVENTCODE_Y_ACTIVE) {
                            uint32_t stunEnable = tr069_tr106_getUnsigned("Device.ManagementServer.STUNEnable");
                            TR069Printf("stunEnable status: %d\n", stunEnable);
                            if(stunEnable)
                                stun_api_pipe_message( STUN_TASK_ACTIVE, 0);
                        } else if(msg.id == EVENTCODE_Y_SUSPEND) {
                            stun_api_pipe_message( STUN_TASK_SUSPEND, 0);
                        }
                        tr069_event_inset(tr069, msg.id, msg.arg0, msg.arg1);
                    }
                }
            }

            if (tr069->sock_request != INVALID_SOCKET && FD_ISSET((unsigned)tr069->sock_request, &rset)) {
                char url[TR069_URL_LEN_1024 + 4];
                struct Digest *digest = &tr069->http_a->digest;

                TR069Printf("Request!\n");
                tr069_create_url(tr069);

                if (tr069_param_read(tr069, "Device.ManagementServer.ConnectionRequestURL", url, TR069_URL_LEN_1024))
                    TR069ErrorOut("ConnectionRequestURL\n");
                if (tr069_checkurl(url, digest->uri, HTTP_URI_LEN, http_a->host, HTTP_HOST_LEN, &http_a->port))
                    TR069ErrorOut("ConnectionRequestURL = %s\n", url);

                digest->username[0] = 0;
                digest->password[0] = 0;
                if (g_tr069_digestAuth) {
                    tr069_param_read(tr069, "Device.ManagementServer.ConnectionRequestUsername", digest->username, DIGEST_PARAM_LEN);
                    tr069_param_read(tr069, "Device.ManagementServer.ConnectionRequestPassword", digest->password, DIGEST_PARAM_LEN);
                }

                {
                    int length;
                    char *buffer;
                    struct Soap *soap = tr069->soap;

                    buffer = soap->buffer;
                    length = SOAP_ENVELOPE_LEN_128K;
                    ret = tr069_http_accept(tr069->http_a, tr069->sock_request, buffer, &length);
                    if (ret < 0)
                        TR069Error("http_accept\n");
                    if (ret == 1) {
                        if (length > 0) {
                            buffer[length] = 0;
                            if (!memcmp(buffer, "<SOAP-ENV:Envelope", 18)) {
                                soap->length = length;
                                if (int_parse(tr069)) {
                                    TR069Error("int_parse\n");
                                } else {
                                    buffer[soap->length] = 0;
#ifndef ANDROID_LOGCAT_OUTPUT
                                    TR069Printf("\n\n%s\n\n", buffer);
#endif
                                }
                            } else {
                                length = 0;
                            }
                        }
                        if (tr069->state == TR069_STATE_ACTIVE && length <= 0) {
                            TR069Printf("EVENTCODE_CONNECTION_REQUEST\n");
                            tr069_event_inset(tr069, EVENTCODE_CONNECTION_REQUEST, 0, 0);
                        }
                    }
                }
            }
        }

        delay = 60;
        continue;
    Err:
        sleep((uint32_t)(delay + tr069->random_factor % delay));
        delay *= 2;
        continue;
    }
}

//------------------------------------------------------------------------------
static void int_task_tail(struct TR069 *tr069)
{
    int i;
    struct Param *param;

    if (tr069->save_flag == 1) {
        TR069Printf("SYS Save\n");
        tr069_config_save(tr069);
    }

    if (tr069->sys_reset) {
        pthread_mutex_lock(&tr069->mutex);
        TR069Printf("SYS Reset\n");
        tr069_param_restore_prepare(tr069);
        tr069_port_setValue("Message.Reset", "", 0);
        tr069_config_reset(tr069);
        tr069->sys_reset = 0;
        tr069_param_restore_complete(tr069);
        tr069_config_save(tr069);

        pthread_mutex_unlock(&tr069->mutex);

        tr069_port_setValue("Message.Notification", "", 0);//ip地址信息改变需要通知应用层保存
#if !defined(ANDROID)
        tr069_port_setValue("Message.ReBoot", "", 0);
#endif
        return;
    }

    if (tr069->reboot.rebootstate == REBOOT_STATE_START) {
        TR069Printf("SYS Reboot\n");
        tr069_port_setValue("Message.ReBoot", "", 0);
    }

    if (tr069->diag_ping) {
        struct Ping ping;
        char addr[18];

        TR069Printf("Ping diagnostic\n");

        pthread_mutex_lock(&tr069->mutex);
        tr069_param_read(tr069, "Device.LAN.IPPingDiagnostics.Host", ping.host, DIAG_HOST_LEN);

        ping.count   = tr069_tr106_getUnsigned("Device.LAN.IPPingDiagnostics.NumberOfRepetitions");
        ping.timeout = tr069_tr106_getUnsigned("Device.LAN.IPPingDiagnostics.Timeout");
        ping.datalen = tr069_tr106_getUnsigned("Device.LAN.IPPingDiagnostics.DataBlockSize");
        ping.dscp    = tr069_tr106_getUnsigned("Device.LAN.IPPingDiagnostics.DSCP");
        pthread_mutex_unlock(&tr069->mutex);

        tr069_port_getValue("Device.LAN.IPAddress", addr, 18);
        if (strcmp(ping.host, addr) == 0 || strcmp(ping.host, "127.0.0.1") == 0) {
            i = 0;
            ping.successcount = ping.count;
            ping.failurecount = 0;
            ping.averagetime = 0;
            ping.minimumtime = 0;
            ping.maximumtime = 0;
        } else {
            if (ping.timeout == 0)
                ping.timeout = 5000;
            i = tr069_diag_ping(&ping);
            TR069Printf("Ping result %d\n", i);
            if (i < 0 || i > DIAG_ERROR_OTHER)
                i = DIAG_ERROR_OTHER;
        }

        pthread_mutex_lock(&tr069->mutex);
        param = tr069_param_hash_find(tr069, "Device.LAN.IPPingDiagnostics.DiagnosticsState");
        tr069_tr106_SetParamValue(param->name, (char*)DiagState[i], 0);

        param = tr069_param_hash_find(tr069, "Device.LAN.IPPingDiagnostics.SuccessCount");
        tr069_tr106_SetParamValue(param->name, NULL, ping.successcount);
	    param = tr069_param_hash_find(tr069, "Device.LAN.IPPingDiagnostics.FailureCount");
        tr069_tr106_SetParamValue(param->name, NULL, ping.failurecount);
        param = tr069_param_hash_find(tr069, "Device.LAN.IPPingDiagnostics.AverageResponseTime");
        tr069_tr106_SetParamValue(param->name, NULL, ping.averagetime);
        param = tr069_param_hash_find(tr069, "Device.LAN.IPPingDiagnostics.MinimumResponseTime");
        tr069_tr106_SetParamValue(param->name, NULL, ping.minimumtime);
        param = tr069_param_hash_find(tr069, "Device.LAN.IPPingDiagnostics.MaximumResponseTime");
        tr069_tr106_SetParamValue(param->name, NULL, ping.maximumtime);
        pthread_mutex_unlock(&tr069->mutex);

        TR069Printf("EVENTCODE_DIAGNOSTICS_COMPLETE\n");
        tr069_event_inset(tr069, EVENTCODE_DIAGNOSTICS_COMPLETE, 0, 0);
        tr069->diag_ping = 0;
    }
    if (tr069->diag_trace) {
        struct Trace trace;
        char addr[18];
        char name[1024];

        TR069Printf("Trace route diagnostic\n");

        pthread_mutex_lock(&tr069->mutex);

        tr069_param_read(tr069,  "Device.LAN.TraceRouteDiagnostics.Host", trace.host,DIAG_HOST_LEN);
        trace.timeout = tr069_tr106_getUnsigned("Device.LAN.TraceRouteDiagnostics.Timeout");
        trace.datalen = tr069_tr106_getUnsigned("Device.LAN.TraceRouteDiagnostics.DataBlockSize");
        trace.hopmax  = tr069_tr106_getUnsigned("Device.LAN.TraceRouteDiagnostics.MaxHopCount");
        trace.dscp    = tr069_tr106_getUnsigned("Device.LAN.TraceRouteDiagnostics.DSCP");

        param = tr069_param_hash_find(tr069, "Device.LAN.TraceRouteDiagnostics.RouteHops.");
        tr069_object_delete(tr069, param);
        pthread_mutex_unlock(&tr069->mutex);

        tr069_port_getValue("Device.LAN.IPAddress", addr, 18);
        if (strcmp(trace.host, addr) == 0 || strcmp(trace.host, "127.0.0.1") == 0) {
            i = 0;
            trace.resptime = 0;
            trace.hopnum = 0;
        } else {
            if (trace.timeout == 0)
                trace.timeout = 2000;
            i = tr069_diag_trace(&trace);
            TR069Printf("RESULT! %d\n", i);
            if (i < 0 || i > DIAG_ERROR_OTHER)
                i = DIAG_ERROR_OTHER;
            if (trace.hopnum > TRACEROUTE_HOPCOUNT_MAX_64)
                trace.hopnum = TRACEROUTE_HOPCOUNT_MAX_64;
        }

        pthread_mutex_lock(&tr069->mutex);

        param = tr069_param_hash_find(tr069, "Device.LAN.TraceRouteDiagnostics.DiagnosticsState");
        tr069_tr106_SetParamValue(param->name, (char*)DiagState[i], 0);
        param = tr069_param_hash_find(tr069, "Device.LAN.TraceRouteDiagnostics.ResponseTime");
	    tr069_tr106_SetParamValue(param->name, NULL, trace.resptime);
        param = tr069_param_hash_find(tr069, "Device.LAN.TraceRouteDiagnostics.NumberOfRouteHops");
        tr069_tr106_SetParamValue(param->name, NULL, trace.hopnum);

        param = tr069_param_hash_find(tr069, "Device.LAN.TraceRouteDiagnostics.RouteHops.");
        strcpy(name, param->name);
        for (i = 0; i < (int)trace.hopnum; i ++) {
            tr069_object_add(tr069, name, i + 1);
            tr106_tr106_set_routehop(i, &trace.routehop[i]);
        }
        pthread_mutex_unlock(&tr069->mutex);

        TR069Printf("EVENTCODE_DIAGNOSTICS_COMPLETE\n");
        tr069_event_inset(tr069, EVENTCODE_DIAGNOSTICS_COMPLETE, 0, 0);
        tr069->diag_trace = 0;
    }
}

//------------------------------------------------------------------------------
void tr069_task_loop(struct TR069 *tr069)
{
    if (!tr069)
        return;

    while (1) {
        int_task_wait(tr069);

        if (int_http_connect(tr069) == 0) {

            tr069_create_url(tr069);

            pthread_mutex_lock(&tr069->mutex);
            if (tr069->prm_change) {
                TR069Printf("EVENTCODE_VALUE_CHANGE\n");
                tr069_event_inset(tr069, EVENTCODE_VALUE_CHANGE, 0, 0);
            }
            tr069_c2s_inform(tr069);
            pthread_mutex_unlock(&tr069->mutex);

            int_parse_loop(tr069);

            int_http_disconnect(tr069);
            int_task_tail(tr069);
        }
        //下面主要目的是为了配合上层应用进行开机升级管理。
        if (tr069->state_boot) {
            tr069_timer_deal(tr069, (int)tr069_sec( ));
            switch(tr069->state) {
            case TR069_STATE_SUSPEND:
                TR069Printf("TR069_STATE_SUSPEND\n");
                break;
            case TR069_STATE_ACTIVE:
                TR069Printf("TR069_STATE_ACTIVE\n");
                tr069_port_setValue("Message.BootSucceed", "", 0);
                break;
            default:
                TR069Printf("TR069_POST_ERROR_CONNECT\n");
                tr069_port_setValue("Message.BootError", "", 0);
                break;
            }
            tr069->state_boot = 0;
        }
        //应付电信测试
        if (tr069->flag_getrpc)
            tr069->flag_getrpc = 0;
    }
}
