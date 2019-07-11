/*******************************************************************************
    公司：
            Yuxing software
    纪录：
            2008-11-28 11:05:39 create by Liu Jianhua
    模块：
            tr069
    简述：
            tr069 保存
 *******************************************************************************/

#include "tr069_header.h"

//------------------------------------------------------------------------------
static uint32_t int_hash_value(const u_char *s)
{
    uint32_t h = 0;
    while (*s)
        h = 65599 * h + *s ++;
  return h % TR069_GLOBAL_HASH_SIZE_199;
}

//------------------------------------------------------------------------------
static struct Global* int_hash_find(const struct TR069 *tr069, const char *name)
{
    struct Global *global;
    uint32_t index;

    if (!tr069 || !name)
        TR069ErrorOut("parameter\n");
    index = int_hash_value((u_char *)name);
    global = tr069->global_hash[index];

    while (global && strcmp(global->name, name))
        global = global->next;

    return global;
Err:
    return NULL;
}

//------------------------------------------------------------------------------
static int int_hash_inset(struct TR069 *tr069, struct Global *global)
{
    uint32_t idx;

    if (int_hash_find(tr069, global->name))
        TR069ErrorOut("already inset\n");

    idx = int_hash_value((u_char *)global->name);
    global->next = tr069->global_hash[idx];
    tr069->global_hash[idx] = global;

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
static int tr069_global_create(struct TR069 *tr069, char *name, int type, void *addr)
{
    struct Global *global;

    if (!name || type < 0 || type >= GLOBAL_TYPE_MAX || !addr)
        TR069ErrorOut("parameter\n");
    if (tr069->global_num >= TR069_GLOBAL_NUM_16)
        TR069ErrorOut("global_num = %d\n", tr069->global_num);
    if (int_hash_find(tr069, name))
        TR069ErrorOut("%s already exist\n", name);
    global = (struct Global *)IND_CALLOC(sizeof(struct Global), 1);
    if (!global)
        TR069ErrorOut("malloc\n");

    tr069_strdup(&global->name, name);
    if (!global->name) {
        IND_FREE(global);
        TR069ErrorOut("tr069_strdup\n");
    }
    global->type = type;
    global->addr = addr;

    tr069->global_table[tr069->global_num] = global;
    tr069->global_num ++;

    int_hash_inset(tr069, global);

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
void tr069_config_init(struct TR069 *tr069)
{
    int i;
    struct Param *param;

    tr069->bootstrap = 0;

    for (i = 0; i < tr069->table_size; i ++) {
        param = tr069->table_array[i];
        if (!param)
            continue;
        param->cksum_flag = 0;

        if (param->attr & TR069_ENABLE_ATTR) {
            param->currentNotification = NOTIFICATION_PASSIVE;
            param->defaultNotification = NOTIFICATION_PASSIVE;
        }
    }

    tr069_tr106_reset(tr069);

    tr069_global_create(tr069, "Bootstrap",           GLOBAL_TYPE_INT,   &tr069->bootstrap);
    tr069_global_create(tr069, "Reboot_state",        GLOBAL_TYPE_INT,   &tr069->reboot.rebootstate);
    tr069_global_create(tr069, "Reboot_commandkey",   GLOBAL_TYPE_STR32,  tr069->reboot.commandkey);
    tr069_global_create(tr069, "Schedule_time",       GLOBAL_TYPE_INT,   &tr069->schedule.scheduletime);
    tr069_global_create(tr069, "Schedule_commandkey", GLOBAL_TYPE_STR32,  tr069->schedule.commandkey);
}

//------------------------------------------------------------------------------
int tr069_config_reset(struct TR069 *tr069)
{
    int i, len;
    struct Param *param;

    tr069->bootstrap = 0;

    len = tr069_port_getDefault(tr069->buf_large, TR069_BUF_LARGE_SIZE_64K);
    if (len > 0)
        tr069_config_file_apply(tr069, tr069->buf_large, len);

    TR069Printf("#############################\n");
    for (i = 0; i < tr069->table_size; i ++) {
        param = tr069->table_array[i];
        if (!param)
            continue;
        param->currentNotification = param->defaultNotification;
        if (param->type != TR069_OBJECT)
            tr069_param_read_cksum(tr069, param, &param->cksum);
    }

    tr069->save_flag = 1;

    return 0;
}

//------------------------------------------------------------------------------
static int tr069_config_elem(const char *buf, int *pline, char *name, int name_len, char *value, int value_len)
{
    int len, i, line;

    len = 0;
    line = *pline;
    for (i = 0; isspace(buf[i]); i ++) {
        if (buf[i] == '\n')
            line ++;
    }
    buf += i;
    len += i;
    for (i = 0; isalnum(buf[i]) || buf[i] == '_'; i ++) ;
    if (i == 0) {
        if (buf[i] == 0)
            return 0;
        TR069ErrorOut("line %d paser\n", line);
    }
    if (name_len <= i)
        TR069ErrorOut("line %d name_len = %d, i = %d\n", line, name_len, i);
    memcpy(name, buf, (uint32_t)i);
    name[i] = 0;
    buf += i;
    len += i;

    for (i = 0; isspace(buf[i]); i ++) {
        if (buf[i] == '\n')
            line ++;
    }
    buf += i;
    len += i;

    if (buf[0] != '=')
        TR069ErrorOut("line %d op = 0x%x\n", line, buf[0]);
    buf ++;
    len ++;

    {
        char ch;

        i = 0;
        for (;;) {
            ch = *buf ++;
            if (!isgraph(ch))
                break;
            len ++;
            if (ch == '\\') {
                ch = *buf ++;
                len ++;
            }
            value[i ++] = ch;
            if (value_len <= i)
                TR069ErrorOut("line %d value_len = %d, i = %d\n", line, value_len, i);
        }
        value[i] = 0;
    }

    *pline = line;
    return len;
Err:
    return -1;
}

//------------------------------------------------------------------------------
static void tr069_config_load_global(struct TR069 *tr069, char *string)
{
    char name[TR069_NAME_SIZE_64 + 1], value[TR069_VALUE_SIZE_256 + 1];
    struct Global *global;
    int len, line;

    line = 1;
    while (1) {
Warn:
        len = tr069_config_elem(string, &line, name, TR069_NAME_SIZE_64, value, TR069_VALUE_SIZE_256);
        if (len == -1)
            TR069ErrorOut("tr069_config_elem\n");
        if (len == 0)
            break;
        if (strstr(name, "Password"))
            TR069Debug("name = %s, value = *\n", name);
        else
            TR069Debug("name = %s, value = %s\n", name, value);
        string += len;

        global = int_hash_find(tr069, name);
        if (!global)
            TR069WarnOut("Unknown global name %s\n", name);

        switch(global->type) {
        case GLOBAL_TYPE_INT:
            if (!isdigit(value[0]))
                TR069WarnOut("%s not digit\n", value);
            *((int *)global->addr) = atoi(value);
            break;
        case GLOBAL_TYPE_STR32:
            if (strlen(value) > 32)
                TR069WarnOut("%s too large\n", value);
            strcpy((char *)global->addr, value);
            break;
        default:
            break;
        }
    }

Err:
    return;
}

//------------------------------------------------------------------------------
static void tr069_config_load_param(struct TR069 *tr069, char *string)
{
    char name[TR069_NAME_SIZE_64 + 1], value[TR069_VALUE_SIZE_256 + 1];
    struct Param *param;
    int len, line;

    line = 1;
    while (1) {
Warn:
        len = tr069_config_elem(string, &line, name, TR069_NAME_SIZE_64, value, TR069_VALUE_SIZE_256);
        if (len == -1)
            TR069ErrorOut("tr069_config_elem\n");
        if (len == 0)
            break;
        if (strstr(name, "Password"))//不打印值，防止密码打印出
            TR069Debug("name = %s, value = *\n", name);
        else
            TR069Debug("name = %s, value = %s\n", name, value);
        string += len;

        param = tr069_short_hash_find(tr069, name);
        if (!param)
            TR069WarnOut("Unknown global name %s\n", name);
        if (!param->prm_setval)
            TR069WarnOut("prm_setval is NULL\n");

        if ((param->attr & TR069_ENABLE_SAVE) == 0)
            TR069WarnOut("%s write disable!\n", name);

        len = strlen(value);
        if (TR069_BASE64 == param->type || TR069_STRING == param->type) {
            if (len >= param->prm_string->size)
                TR069WarnOut("TR069_STRING %d / %d\n", len, param->prm_string->size);
        }

        if (TR069_DATETIME == param->type) {//为兼容老版本，做特殊处理
            unsigned int sec = atoi(value);
            tr069_tr106_SetParamValue(param->name, NULL, sec);
        } else {
            if (param->prm_chkval && param->prm_chkval(param->name, value, len))
                TR069WarnOut("pm_ckint\n");
            if (param->prm_setval(param->name, value, len))
                TR069Warn("setval %s\n", param->name);
        }
    }

Err:
    return;
}

//------------------------------------------------------------------------------
static void tr069_config_load_notifications(struct TR069 *tr069, char *string)
{
    char name[TR069_NAME_SIZE_64 + 1], value[TR069_VALUE_SIZE_256 + 1];
    struct Param *param;
    int len, line;

    line = 1;
    while (1) {
Warn:
        len = tr069_config_elem(string, &line, name, TR069_NAME_SIZE_64, value, TR069_VALUE_SIZE_256);
        if (len == -1)
            TR069ErrorOut("parse failed\n");
        if (len == 0)
            break;
        if (strstr(name, "Password"))
            TR069Debug("name = %s, value = *\n", name);
        else
            TR069Debug("name = %s, value = %s\n", name, value);
        string += len;

        param = tr069_short_hash_find(tr069, name);
        if (!param)
            TR069WarnOut("Unknown global name %s\n", name);

        if ((param->attr & TR069_ENABLE_ATTR) == 0)
            continue;
        param->currentNotification = atoi(value);
    }

Err:
    return;
}

//------------------------------------------------------------------------------
static void tr069_config_load_cksums(struct TR069 *tr069, char *string)
{
    char name[TR069_NAME_SIZE_64 + 1], value[TR069_VALUE_SIZE_256 + 1];
    struct Param *param;
    int len, line;

    line = 1;
    while (1) {
Warn:
        len = tr069_config_elem(string, &line, name, TR069_NAME_SIZE_64, value, TR069_VALUE_SIZE_256);
        if (len == -1)
            TR069ErrorOut("parse failed\n");
        if (len == 0)
            break;
        string += len;

        param = tr069_short_hash_find(tr069, name);
        if (!param)
            TR069WarnOut("Unknown global name %s\n", name);

        if ((param->attr & TR069_ENABLE_ATTR) == 0 || NOTIFICATION_OFF == param->currentNotification)
            continue;
        param->cksum = tr069_atoui(value);
        param->cksum_flag = 1;
        TR069Debug("cksum = %08x %s\n", param->cksum, name);
    }

Err:
    return;
}

//------------------------------------------------------------------------------
static void tr069_config_load_load(struct TR069 *tr069, char *string)
{
    char name[TR069_NAME_SIZE_64 + 1], value[TR069_VALUE_SIZE_256 + 1];
    int len, line;
    struct Load *load, *prev, *next;

    load = (struct Load *)IND_CALLOC(sizeof(struct Load), 1);
    if (!load)
        TR069ErrorOut("malloc Load\n");

    line = 1;
    while (1) {

        len = tr069_config_elem(string, &line, name, TR069_NAME_SIZE_64, value, TR069_VALUE_SIZE_256);
        if (len == -1)
            TR069ErrorOut("parse failed\n");
        if (len == 0)
            break;
        if (strstr(name, "Password"))//不打印值，防止密码打印出
            TR069Debug("name = %s, value = *\n", name);
        else
            TR069Debug("name = %s, value = %s\n", name, value);
        string += len;

        if (strcmp(name, "faultcode") == 0)
            load->faultcode = atoi(value);
        else if (strcmp(name, "transferstate") == 0)
            load->transferstate = atoi(value);
        else if (strcmp(name, "commandkey") == 0)
            strcpy(load->CommandKey, value);
        else if (strcmp(name, "filetype") == 0)
            strcpy(load->FileType, value);
        else if (strcmp(name, "downflag") == 0)
            load->isDownload = atoi(value);
        else if (strcmp(name, "url") == 0)
            strcpy(load->URL, value);
        else if (strcmp(name, "username") == 0)
            strcpy(load->Username, value);
        else if (strcmp(name, "password") == 0)
            strcpy(load->Password, value);
        else if (strcmp(name, "loadtime") == 0)
            load->loadtime = atoi(value);
        else if (strcmp(name, "starttime") == 0)
            load->starttime = atoi(value);
        else if (strcmp(name, "completetime") == 0)
            load->completetime = atoi(value);
    }

    load->loadtime = (int)tr069_sec( );//由于时间是相对开机时间，重启后立即升级或下载
    if (TRANSFER_STATE_START == load->transferstate)
        load->transferstate = TRANSFER_STATE_INIT;

    prev = NULL;
    next = tr069->loadQueue;
    while (next) {
        prev = next;
        next = next->next;
    }
    if (prev)
        prev->next = load;
    else
        tr069->loadQueue = load;

    load->next = NULL;

    load = NULL;

Err:
    if (load)
        IND_FREE(load);
    return;
}

//------------------------------------------------------------------------------
static int tr069_config_load_parse(struct TR069 *tr069, char *string, char *item, void (*load_func)(struct TR069 *, char *))
{
    int length, l;
    char *begin, *end, tag[20];

    length = 0;
    l = strlen(item);

    sprintf(tag, "<%s>", item);
    begin = strstr(string, tag);
    if (!begin)
        TR069ErrorOut("%s not fond\n", tag);
    sprintf(tag, "</%s>", item);
    end = strstr(begin, tag);
    if (!end)
        TR069ErrorOut("%s not fond\n", tag);

    length = end + 3 + l - string;
    *end = 0;
    load_func(tr069, begin + 1 + l + 1);

Err:
    return length;
}

//------------------------------------------------------------------------------
int tr069_config_load(struct TR069 *tr069)
{
    int l, len, size;
    char *buf;

    if (!tr069)
        TR069ErrorOut("tr069 is NULL\n");

    buf = tr069->buf_large;
    size = tr069_port_infoLoad(buf, TR069_BUF_LARGE_SIZE_64K);
    if (size <= 0) {
        tr069_config_reset(tr069);
        TR069ErrorOut("size = %d!\n", size);
    }
    TR069Printf("config len = %d\n", size);
    buf[size] = 0;
    //不能打印buf，避免显示密码

    len = 0;
    len += tr069_config_load_parse(tr069, buf + len, "Global",          tr069_config_load_global);
    len += tr069_config_load_parse(tr069, buf + len, "Param",           tr069_config_load_param);
    len += tr069_config_load_parse(tr069, buf + len, "Notifications",   tr069_config_load_notifications);
    len += tr069_config_load_parse(tr069, buf + len, "Cksums",          tr069_config_load_cksums);

    while (strstr(buf + len, "<Load>")) {
        l = tr069_config_load_parse(tr069, buf + len, "Load",            tr069_config_load_load);
        if (l <= 0)
            break;
        len += l;
    }

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
static int tr069_config_save_global(struct TR069 *tr069, char *buffer, int length)
{
    int i, l, len;
    struct Global *global;

    if (length < TR069_NAME_SIZE_64)
        TR069ErrorOut("+ length = %d\n", length);
    len = sprintf(buffer, "<Global>\n");
    length -= len;
    buffer += len;

    for (i = 0; i < tr069->global_num; i ++) {
        if (length < TR069_NAME_SIZE_64 + TR069_VALUE_SIZE_256)
            TR069ErrorOut("length = %d\n", length);

        global = tr069->global_table[i];
        switch(global->type) {
        case GLOBAL_TYPE_INT:
            l = sprintf(buffer, "%s=%d\n", global->name, *((int *)global->addr));
            break;
        case GLOBAL_TYPE_STR32:
            l = sprintf(buffer, "%s=%s\n", global->name, (char *)global->addr);
            break;
        default:
            TR069ErrorOut("type = %d\n", global->type);
        }
        len += l;
        length -= l;
        buffer += l;
    }
    if (length < TR069_NAME_SIZE_64)
        TR069ErrorOut("+ length = %d\n", length);
    len += sprintf(buffer, "</Global>\n");

    return len;
Err:
    return 0;
}

//------------------------------------------------------------------------------
static int tr069_config_save_param(struct TR069 *tr069, char *buffer, int length)
{
    int i, l, len;
    struct Param *param;

    TR069Printf("length = %d\n", length);

    if (length < TR069_NAME_SIZE_64)
        TR069ErrorOut("- length = %d\n", length);
    len = sprintf(buffer, "<Param>\n");
    length -= len;
    buffer += len;

    for (i = 0; i < tr069->table_size; i ++) {
        param = tr069->table_array[i];
        if (!param)
            continue;

        if (!param || (param->attr & TR069_ENABLE_SAVE) == 0 || !param->sname)
            continue;
        if (length < TR069_NAME_SIZE_64 + TR069_VALUE_SIZE_256)
            TR069ErrorOut("length = %d\n", length);

        l = sprintf(buffer, "%s=", param->sname);
        switch(param->type) {
        case TR069_INT:
        case TR069_UNSIGNED:
        case TR069_BOOLEAN:
            if (param->prm_getval(param->name, buffer + l, TR069_VALUE_SIZE_256))
                TR069Warn("getval %s\n", param->name);
            l += strlen(buffer + l);
            l += sprintf(buffer + l, "\n");
            break;
            break;
        case TR069_DATETIME:
            {
                unsigned int sec;
                sec = tr069_tr106_getUnsigned(param->name);
                l += sprintf(buffer + l, "%u\n", sec);
            }
            break;
        case TR069_STRING:
        case TR069_BASE64:
            {//空格等字符用\转意，与之对应的是，tr069_config_elem里面反响转换
                char *p, ch;
                struct String *string;

                string = param->prm_string;
                string->value[0] = 0;
                if (param->prm_getval(param->name, string->value, string->size))
                    TR069Warn("getval %s\n", param->name);

                p = string->value;
                for(;;) {
                    ch = *p ++;
                    if (ch == 0)
                        break;
                    if (isspace(ch) || ch == '\\')
                        buffer[l++] = '\\';
                    buffer[l++] = ch;
                }
                buffer[l++] = '\n';
            }
            break;
        default:
            TR069ErrorOut("type = %d\n", param->type);
        }
        len += l;
        length -= l;
        buffer += l;
    }

    if (length < TR069_NAME_SIZE_64)
        TR069ErrorOut("- length = %d\n", length);
    len += sprintf(buffer, "</Param>\n");

    return len;
Err:
    return 0;
}

//------------------------------------------------------------------------------
static int tr069_config_save_notifications(struct TR069 *tr069, char *buffer, int length)
{
    int i, l, len;
    struct Param *param;

    if (length < TR069_NAME_SIZE_64)
        TR069ErrorOut("- length = %d\n", length);
    len = sprintf(buffer, "<Notifications>\n");
    length -= len;
    buffer += len;


    for (i = 0; i < tr069->table_size; i ++) {
        param = tr069->table_array[i];
        if (!param)
            continue;
        if (0 == (param->attr & TR069_ENABLE_ATTR) || !param->sname)
            continue;
        if (param->currentNotification == param->defaultNotification)
            continue;
        if (length < TR069_NAME_SIZE_64 + TR069_VALUE_SIZE_256)
            TR069ErrorOut("length = %d\n", length);

        l = sprintf(buffer, "%s=%d\n", param->sname, param->currentNotification);
        len += l;
        length -= l;
        buffer += l;
    }

    if (length < TR069_NAME_SIZE_64)
        TR069ErrorOut("+ length = %d\n", length);
    len += sprintf(buffer, "</Notifications>\n");

    return len;
Err:
    return 0;
}

//------------------------------------------------------------------------------
static int tr069_config_save_cksums(struct TR069 *tr069, char *buffer, int length)
{
    int i, l, len;
    struct Param *param;

    if (length < TR069_NAME_SIZE_64)
        TR069ErrorOut("- length = %d\n", length);
    len = sprintf(buffer, "<Cksums>\n");
    length -= len;
    buffer += len;

    for (i = 0; i < tr069->table_size; i ++) {
        param = tr069->table_array[i];
        if (!param)
            continue;
        if (0 == (param->attr & TR069_ENABLE_ATTR) || NOTIFICATION_OFF == param->currentNotification || !param->sname)
            continue;
        if (length < TR069_NAME_SIZE_64 + TR069_VALUE_SIZE_256)
            TR069ErrorOut("length = %d\n", length);

        //TR069Debug("cksums = %08x %s\n", cksums[i], param->name);
        l = sprintf(buffer, "%s=%u\n", param->sname, param->cksum);
        len += l;
        length -= l;
        buffer += l;
    }

    if (length < TR069_NAME_SIZE_64)
        TR069ErrorOut("+ length = %d\n", length);
    len += sprintf(buffer, "</Cksums>\n");

    return len;
Err:
    return 0;
}

//------------------------------------------------------------------------------
#define TR069_LOAD_LENGTH_1024    1024
static int tr069_config_save_loads(struct TR069 *tr069, char *buffer, int length)
{
    int l, len;
    struct Load *load;

    len = 0;
    for (load = tr069->loadQueue; load; load = load->next) {
        if (length <= TR069_LOAD_LENGTH_1024)
            TR069ErrorOut("length = %d\n", length);
        l = snprintf(buffer, TR069_LOAD_LENGTH_1024, "<Load>\nfaultcode=%d\ntransferstate=%d\ncommandkey=%s\nfiletype=%s\ndownflag=%d\nurl=%s\nusername=%s\npassword=%s\nloadtime=%d\nstarttime=%d\ncompletetime=%d\n</Load>\n", 
                                load->faultcode, load->transferstate, load->CommandKey, load->FileType, load->isDownload, load->URL, load->Username, load->Password, load->loadtime, load->starttime, load->completetime);
        if (l < 0)
            TR069ErrorOut("l = %d\n", l);
        TR069Debug("filetype=%s\nurl=%s\n\n", load->FileType, load->URL);
        len += l;
        length -= l;
        buffer += l;
    }

    return len;
Err:
    return 0;
}

//------------------------------------------------------------------------------
int tr069_config_save(struct TR069 *tr069)
{
    int len;
    char *buf;

    if (!tr069)
        TR069ErrorOut("tr069 is NULL\n");
    len = 0;
    buf = tr069->buf_large;
    len += tr069_config_save_global(        tr069, buf + len, TR069_BUF_LARGE_SIZE_64K - len);
    len += tr069_config_save_param(         tr069, buf + len, TR069_BUF_LARGE_SIZE_64K - len);
    len += tr069_config_save_notifications( tr069, buf + len, TR069_BUF_LARGE_SIZE_64K - len);
    len += tr069_config_save_cksums(        tr069, buf + len, TR069_BUF_LARGE_SIZE_64K - len);
    len += tr069_config_save_loads(         tr069, buf + len, TR069_BUF_LARGE_SIZE_64K - len);

    TR069Printf("config len = %d\n", len);
    tr069_port_infoSave(buf, len);

    tr069->save_flag = 0;
    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
int tr069_config_file_create(struct TR069 *tr069, char *buffer, int length)
{
    int i, l, len;
    int addrtype;
    struct Param *param;
    char buf[DEVICE_LAN_AddressingType_LEN_15 + 1];

    tr069_param_read(tr069, "Device.LAN.AddressingType", buf, DEVICE_LAN_AddressingType_LEN_15 + 1);
    if (strcmp(buf, "DHCP") == 0)
        addrtype = 0;
    else if (strcmp(buf, "Static") == 0)
        addrtype = 1;
    else if (strcmp(buf, "PPPoE") == 0)
        addrtype = 2;
    else
        addrtype = -1;

    len = 0;
    for (i = 0; i < tr069->table_size; i ++) {
        if (length < TR069_NAME_FULL_SIZE_128 + TR069_VALUE_SIZE_256)
            TR069ErrorOut("length = %d\n", length);

        param = tr069->table_array[i];
        if (!param)
            continue;
        if (0 == (param->attr & TR069_ENABLE_CONFIG) || !param->sname)
            continue;
        if ((i == g_tr106Index->LAN_IPAddress_index || 
            i == g_tr106Index->LAN_SubnetMask_index || 
            i == g_tr106Index->LAN_DefaultGateway_index || 
            i == g_tr106Index->LAN_DNSServers_index) && addrtype != 1) {
            continue;
        }

        l = sprintf(buffer, "%s=%s\n", param->sname, tr069_fmt_param(tr069, param->name));
        len += l;
        length -= l;
        buffer += l;
    }

    return len;
Err:
    return 0;
}

//------------------------------------------------------------------------------
void tr069_config_file_print(struct TR069 *tr069)
{
    int i;
    struct Param *param;

    for (i = 0; i < tr069->table_size; i ++) {
        param = tr069->table_array[i];
        if (!param)
            continue;
        if (0 == (param->attr & TR069_ENABLE_CONFIG) || !param->sname)
            continue;
        TR069Printf("%s=%s\n", param->sname, tr069_fmt_param(tr069, param->name));
    }
}

//------------------------------------------------------------------------------
int tr069_config_file_apply(struct TR069 *tr069, char *buffer, int length)
{
    char name[TR069_NAME_SIZE_64 + 1], value[TR069_VALUE_SIZE_256 + 1];
    struct Param *param;
    char *buf = buffer;
    int len, line;

    buffer[length] = 0;

    line = 1;
    tr069->addrtype_config[0] = 0;
    tr069_paramObject_reset(tr069);
    while (1) {
        len = tr069_config_elem(buf, &line, name, TR069_NAME_SIZE_64, value, TR069_VALUE_SIZE_256);
        if (len == -1)
            TR069ErrorOut("parse failed\n");
        if (len == 0)
            break;
        buf += len;

        param = tr069_short_hash_find(tr069, name);
        if (!param)
            TR069ErrorOut("Unknown param short name %s\n", name);
        if (strstr(name, "Password"))//不打印值，防止密码打印出
            TR069Printf("name = %s, value = *\n", name);
        else
            TR069Printf("name = %s, value = %s\n", name, value);

        if (0 == value[0])
            continue;
        if (tr069_param_write_try(tr069, param->index, value))
            TR069ErrorOut("invalid value %s\n", name);

        if (tr069_paramObject_inset(tr069, param, 0))
            TR069ErrorOut("Resources exceeded %d / %s\n", param->index, param->name);
        if (param->index == g_tr106Index->LAN_AddressingType_index)
            tr069_paramObject_apply_values(tr069, 1);
    }
    tr069_paramObject_apply_values(tr069, 1);

    return 0;

Err:
    tr069_paramObject_reset(tr069);
    return -1;
}


