/*******************************************************************************
    公司：
            Yuxing software
    纪录：
            2008-1-26 21:12:26 create by Liu Jianhua
    模块：
            tr069
    简述：
            tr069 参数
 *******************************************************************************/

#include "tr069_header.h"

//------------------------------------------------------------------------------
char *tr069_fmt_time(struct TR069 *tr069, int ltime)
{
    tr069_time2str(ltime, tr069->buf);
    return tr069->buf;
}

//------------------------------------------------------------------------------
char *tr069_fmt_id(struct TR069 *tr069)
{
    tr069->cwmp_id ++;
    sprintf(tr069->buf, "%d", tr069->cwmp_id);
    return tr069->buf;
}

//------------------------------------------------------------------------------
char *tr069_fmt_int(struct TR069 *tr069, const char *fmt, int num)
{
    sprintf(tr069->buf, fmt, num);
    return tr069->buf;
}

//------------------------------------------------------------------------------
char *tr069_fmt_param(struct TR069 *tr069, const char *name)
{
    struct Param *param = tr069_param_hash_find(tr069, name);

    tr069->buf[0] = 0;
    if ((param->attr & TR069_ENABLE_READ) == 0)
        return tr069->buf;

    tr069->buf[0] = 0;
    if (param->prm_getval(param->name, tr069->buf, TR069_BUF_SIZE_4096))
        TR069Warn("getval %s\n", param->name);

    if (strstr(param->name, "Password"))
        TR069Debug("%s getValue *\n", param->name);
    else
        TR069Debug("%s getValue %s\n", param->name, tr069->buf);

    return tr069->buf;
}

#define CHECK_INDEX( )                                              \
do {                                                                \
    if (!tr069 || index < 0 || index >= tr069->table_size)   \
        TR069ErrorOut("tr069 = %p, index = %d\n", tr069, index);    \
    param = tr069->table_array[index];                              \
    if (!param)                                              \
        TR069ErrorOut("param is NULL\n");                           \
} while (0)

//------------------------------------------------------------------------------
uint32_t tr069_param_hash_value(const u_char *s)
{
    uint32_t h = 0;
    while (*s)
        h = 65599 * h + *s ++;
  return h % TR069_PARAM_HASH_SIZE_1999;
}


//------------------------------------------------------------------------------
struct Param* tr069_short_hash_find(const struct TR069 *tr069, const char *sname)
{
    struct Param *param;
    uint32_t index;

    if (!tr069 || !sname)
        TR069ErrorOut("tr069 = %p, sname = %p\n", tr069, sname);
    index = tr069_param_hash_value((u_char *)sname);
    param = tr069->prm_shash[index];

    while (param && strcmp(param->sname, sname))
        param = param->shortNext;

    return param;
Err:
    return NULL;
}

//------------------------------------------------------------------------------
static int tr069_short_hash_inset(struct TR069 *tr069, struct Param *param)
{
    uint32_t index;

    if (tr069_short_hash_find(tr069, param->sname))
        TR069ErrorOut("%s\n", param->sname);

    index = tr069_param_hash_value((unsigned char *)param->sname);
    param->shortNext = tr069->prm_shash[index];
    tr069->prm_shash[index] = param;

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
struct Param* tr069_param_hash_find(const struct TR069 *tr069, const char *name)
{
    struct Param *param;
    uint32_t idx;

    if (!name)
        return NULL;
    idx = tr069_param_hash_value((unsigned char *)name);
    param = tr069->prm_hash[idx];

    while (param && strcmp(param->name, name))
        param = param->hashNext;

    return param;
}

//------------------------------------------------------------------------------
int tr069_param_hash_inset(struct TR069 *tr069, struct Param *param)
{
    char *p;
    uint32_t idx;
    uint32_t len;
    struct Object *obj;
    char name[TR069_NAME_FULL_SIZE_128 + 4];

    if (tr069_param_hash_find(tr069, param->name))
        TR069ErrorOut("alread inset %s\n", name);
    strcpy(name, param->name);
    len = strlen(name);
    if (name[len - 1] == '.')
        name[len - 1] = 0;
    p = strrchr(name, '.');
    if (p) {//objects
            struct Param *parent;

            p[1] = 0;
            parent = tr069_param_hash_find(tr069, name);
            if (!parent)
                TR069ErrorOut("%s parent is NULL\n", param->name);
            obj = &(parent->prm_objValue);
            param->treePrev = obj->lastChild;
            if (obj->lastChild)
                obj->lastChild->treeNext = param;
            else
                obj->firstChild = param;
            obj->lastChild = param;
            param->treeParent = parent;
    }

    idx = tr069_param_hash_value((unsigned char *)param->name);
    param->hashNext = tr069->prm_hash[idx];
    tr069->prm_hash[idx] = param;

    return 0;

Err:
    return -1;
}

void tr069_param_hash_remove(struct TR069 *tr069, char *name)
{
    struct Param *prev, *param;
    uint32_t idx;

    prev = NULL;
    idx = tr069_param_hash_value((unsigned char *)name);
    param = tr069->prm_hash[idx];

    while (param) {
        if (!strcmp(param->name, name))
            break;
        prev = param;
        param = param->hashNext;
    }
    if (param) {
        if (prev)
            prev->hashNext = param->hashNext;
        else
            tr069->prm_hash[idx] = param->hashNext;
    } else {
        TR069Error("name = %p hash damage!\n", name);
    }
}

//------------------------------------------------------------------------------
int tr069_param_new(struct TR069 *tr069, char *name, char *sname, uint32_t type, uint32_t vlen, uint32_t attr,
                    Tr069ValueFunc getval, Tr069ValueFunc chkval, Tr069ValueFunc setval, OnChgFunc onchg)
{
    int i;
    uint32_t nlen, snlen;
    struct Param *param, **table_array;

    param = NULL;

    if (!tr069 || !name)
        TR069ErrorOut("tr069 = %p, name = %p\n", tr069, name);

    if (tr069_param_hash_find(tr069, name))
        TR069ErrorOut("%s tr069_param_hash_find\n", name);
    nlen = strlen(name);
    if (sname) {
        if (tr069_short_hash_find(tr069, sname))
            TR069ErrorOut("%s tr069_short_hash_find\n", sname);
        snlen = strlen(sname);
    } else {
        snlen = 0;
    }
    if (nlen == 0 || nlen > TR069_NAME_FULL_SIZE_128 || snlen > TR069_NAME_SIZE_64)
        TR069ErrorOut("%s nlen = %d, snlen = %d\n", name, nlen, snlen);

    if (tr069->table_off >= tr069->table_size) {
        tr069->table_size += DEVICE_INDEX_NUM_2048;
        tr069->table_array = (struct Param **)IND_REALLOC(tr069->table_array, sizeof(struct Param *) * tr069->table_size);
        table_array = &tr069->table_array[tr069->table_size - DEVICE_INDEX_NUM_2048];
        for (i = 0; i < DEVICE_INDEX_NUM_2048; i++)
            table_array[i] = NULL;
    }

    param = (struct Param *)IND_CALLOC(sizeof(struct Param), 1);
    if (!param)
        TR069ErrorOut("%s malloc Param\n", name);

    switch(type) {
    case TR069_OBJECT:
        vlen = 0;
        break;
    case TR069_BOOLEAN:
    case TR069_INT:
    case TR069_UNSIGNED:
        vlen = 12;
        break;
    case TR069_DATETIME:
        vlen = 20;
        break;
    case TR069_STRING:
    case TR069_BASE64:
        if ((vlen < 0) && (attr | TR069_ENABLE_WRITE))
            TR069ErrorOut("%s vlen = %d\n", name, vlen);
        vlen++;
        break;
    default:
        TR069ErrorOut("%s type = %d\n", name, type);
    }

    if (vlen > 0 && (attr | TR069_ENABLE_WRITE)) {
        param->prm_string = (struct String *)IND_MALLOC(sizeof(struct String) + vlen);
        if (!param->prm_string)
            TR069ErrorOut("%s malloc String\n", name);
        param->prm_string->size = vlen;
    }

    tr069_strdup(&param->name, name);

    param->type = type;
    param->attr = attr;

    if (!getval)
        getval = tr069_port_getValue;
    if (attr & TR069_ENABLE_WRITE && !setval)
        setval = tr069_port_setValue;

    if (TR069_OBJECT != type) {
        param->prm_getval = getval;
        param->prm_chkval = chkval;
        param->prm_setval = setval;
    	param->prm_onchg = onchg;
    }

    if (tr069_param_hash_inset(tr069, param))
        TR069ErrorOut("tr069_param_hash_inset %s\n", param->name);
    if (param->attr & (TR069_ENABLE_CONFIG | TR069_ENABLE_SAVE) && param->type != TR069_OBJECT) {
        if (snlen) {
            tr069_strdup(&param->sname, sname);
            tr069_short_hash_inset(tr069, param);
        } else {
            TR069Warn("short name is NULL %s\n", param->name);
        }
    }
    i = tr069->table_off;
    param->index = i;

    table_array = tr069->table_array;
    table_array[i] = param;

    while (i < tr069->table_size && table_array[i])
        i++;
    tr069->table_off = i;

    return param->index;
Err:
    if (name)
        TR069Printf("%s\n", name);
    if (param) {
        if (param->prm_string)
            IND_FREE(param->prm_string);
        if (param->name)
            IND_FREE(param->name);
        if (param->sname)
            IND_FREE(param->sname);
        IND_FREE(param);
    }
    return -1;
}

//------------------------------------------------------------------------------
int tr069_param_virtual(struct TR069 *tr069, char *name, char *childName, uint32_t type, uint32_t vlen, uint32_t attr,
                    Tr069ValueFunc getval, Tr069ValueFunc setval)
{
    int nlen;
    struct Object *obj;
    struct Param *param, *child = NULL;

    param = tr069_param_hash_find(tr069, name);
    if (!param)
        TR069ErrorOut("%s not exist!\n", name);

    nlen = strlen(childName);
    if (nlen <= 0 || nlen >= TR069_NAME_SIZE_64)
        TR069ErrorOut("%s nlen = %d\n", childName, nlen);

    child = (struct Param *)IND_CALLOC(sizeof(struct Param), 1);
    if (!child)
        TR069ErrorOut("%s malloc Param\n", childName);

    switch(type) {
    case TR069_OBJECT:
    case TR069_BOOLEAN:
    case TR069_INT:
    case TR069_UNSIGNED:
    case TR069_DATETIME:
        break;
    case TR069_STRING:
    case TR069_BASE64:
        child->prm_string = (struct String *)IND_MALLOC(sizeof(struct String));
        if (!child->prm_string)
            TR069ErrorOut("%s malloc String\n", childName);
        child->prm_string->size = vlen;
        break;
    default:
        TR069ErrorOut("%s type = %d\n", childName, type);
    }

    tr069_strdup(&child->name, childName);
    child->type = type;
    child->attr = attr;

    child->prm_getval = getval;
    child->prm_setval = setval;

    obj = &param->prm_objVirtual;
    if (obj->lastChild)
        obj->lastChild->treeNext = child;
    else
        obj->firstChild = child;
    obj->lastChild = child;

    return 0;
Err:
    if (childName)
        TR069Printf("%s\n", childName);
    if (child) {
        if (child->prm_string)
            IND_FREE(child->prm_string);
        if (child->name)
            IND_FREE(child->name);
        IND_FREE(child);
    }
    return -1;
}

static void int_param_delete(struct TR069 *tr069, struct Param *param)
{
    struct Object *obj;
    struct Param *next, *prev;

    tr069->table_array[param->index] = NULL;
    if (tr069->table_off > param->index)
        tr069->table_off = param->index;

    obj = &param->treeParent->prm_objValue;
    next = param->treeNext;
    prev = param->treePrev;
    if (prev)
        prev->treeNext = next;
    else
        obj->firstChild = next;
    if (next)
        next->treePrev = prev;
    else
        obj->lastChild = prev;

    obj = &tr069->prm_object;
    if (param->paramNext || param == obj->lastChild) {
        prev = NULL;
        next = obj->firstChild;
        while (next != param) {
            prev = next;
            next = next->paramNext;
        }
        if (next) {
            if (prev)
                prev->paramNext = param->paramNext;
            else
                obj->firstChild = param->paramNext;
            if (param == obj->lastChild)
                obj->lastChild = prev;
        } else {
            TR069Error("name = %p prm_object damage!\n", param->name);
        }
    }

    if (param->isChange) {
        prev = NULL;
        next = tr069->prm_change;
        while (next != param) {
            prev = next;
            next = next->changeNext;
        }
        if (next) {
            if (prev)
                prev->changeNext = param->changeNext;
            else
                tr069->prm_change = param->changeNext;
        } else {
            TR069Error("name = %p prm_change damage!\n", param->name);
        }
    }

    tr069_param_hash_remove(tr069, param->name);

    if (TR069_OBJECT == param->type) {
        obj = &param->prm_objValue;

        while (obj->firstChild)
            int_param_delete(tr069, obj->firstChild);
    } else {
        if (param->prm_string)
            IND_FREE(param->prm_string);
    }

    if (param->name)
        IND_FREE(param->name);
    if (param->sname)
        IND_FREE(param->sname);
    IND_FREE(param);
}

int tr069_param_delete(struct TR069 *tr069, int index)
{
    struct Param *param;

    CHECK_INDEX( );

    int_param_delete(tr069, param);

Err:
    return -1;
}

//------------------------------------------------------------------------------

static int int_param_write(struct TR069 *tr069, int notify, struct Param *param, char *str, unsigned int x)
{
    Tr069ValueFunc chkval;
    OnChgFunc onchg;

    chkval = param->prm_chkval;
    if (chkval && chkval(param->name, str, x))
        TR069ErrorOut("chkval %s\n", param->name);

    if ((param->attr & TR069_ENABLE_READ) == 0 && str[0] == 0) {
        TR069Printf("ignore %s\n", param->name);
        return 0;
    }

    if (!param->prm_setval)
        TR069ErrorOut("%s prm_setval is NULL\n", param->name);

    if (notify) {
        onchg = NULL;
    } else {
        onchg = param->prm_onchg;
        if (onchg) {
            uint32_t l;
            char buf[TR069_BUF_SIZE_4096];
    
            if (!param->prm_getval)
                TR069ErrorOut("%s prm_getval is NULL\n", param->name);
    
            l = strlen(str);
            if (l >= param->prm_string->size)
                TR069ErrorOut("%s len = %d / %d\n", param->name, l, param->prm_string->size);
    
            buf[0] = 0;
            if (param->prm_getval(param->name, buf, TR069_BUF_SIZE_4096))
                TR069Warn("getval %s\n", param->name);
    
            if (strcmp(buf, str) == 0) {
                TR069Debug("%s not change!\n", param->name);
                onchg = NULL;
            }
        }
    }

    if (param->prm_setval(param->name, str, x))
        TR069Error("setval %s\n", param->name);

    if (onchg)
        onchg(tr069, param->name);

    return 1;
Err:
    return -1;
}

int tr069_param_write(struct TR069 *tr069, char *name, char *str, unsigned int x)
{
    struct Param *param;

    param = tr069_param_hash_find(tr069, name);
    if (!param)
        TR069ErrorOut("not exist! %p\n", name);

    return int_param_write(tr069, 1, param, str, x);
Err:
    return -1;
}

//------------------------------------------------------------------------------
int tr069_param_write_try(struct TR069 *tr069, int index, char *value)
{
    uint32_t len;
    struct Param *param;
    Tr069ValueFunc chkval;

    CHECK_INDEX( );
    if (!value)
        TR069ErrorOut("value = %p\n", value);

    if ((param->attr & TR069_ENABLE_WRITE) == 0)
        TR069ErrorOut("write disable!\n");

    len = strlen(value);
    if (param->type == TR069_BASE64 && base64_decode((u_char *)value, strlen(value), NULL, len) < 0)
        TR069ErrorOut("%s TR069_BASE64\n", param->name);
    if (len >= param->prm_string->size)
        TR069ErrorOut("%s TR069_STRING %d / %d\n", param->name, len, param->prm_string->size);

    chkval = param->prm_chkval;
    if (chkval && chkval(param->name, value, 0))
        TR069ErrorOut("%s chkval\n", param->name);

    if (TR069_BOOLEAN == param->type) {
        if (stricmp(value, "true") == 0)
            strcpy(param->prm_string->value, "1");
        else if (stricmp(value, "false") == 0)
            strcpy(param->prm_string->value, "0");
        else
            strcpy(param->prm_string->value, value);
    } else {
        strcpy(param->prm_string->value, value);
    }

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
int tr069_param_write_attr(struct TR069 *tr069, char *name, uint32_t notification)
{
    struct Param *param;

    param = tr069_param_hash_find(tr069, name);
    if (!param)
        TR069ErrorOut("not exist! %p\n", name);

    if ((param->attr & TR069_ENABLE_ATTR) == 0)
        TR069ErrorOut("attr no enable change %s\n", param->name);
    if (param->type == TR069_OBJECT)
        TR069ErrorOut("the param is object\n");

    param->currentNotification = (u_char)notification;

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
int tr069_param_read(struct TR069 *tr069, char *name, char *str, uint32_t x)
{
    struct Param *param;

    param = tr069_param_hash_find(tr069, name);
    if (!param)
        TR069ErrorOut("not exist! %p\n", name);

    if (!param->prm_getval)
        TR069ErrorOut("prm_getint is NULL %s\n", name);

    str[0] = 0;
    if (param->prm_getval(name, str, x))
        TR069Warn("getval %s\n", param->name);

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
static uint32_t string_cksum(u_char *buf)
{
    int i;
    u_char ch;
    uint32_t cksum;

    cksum = 1;
    for (i = 0; i < TR069_VALUE_SIZE_256; i ++) {
        ch = buf[i];
        if (ch == 0)
            break;
        cksum *= ch;
        cksum ++;
    }
    return cksum;
}

//------------------------------------------------------------------------------
static uint32_t int_param_cksum(struct TR069 *tr069, struct Param *param, char *str)
{
    uint32_t cksum = 0;

    switch(param->type) {
    case TR069_INT:
        {
            int val = 0;
            tr069_str2int(str, &val);
            cksum = val;
        }
        break;
    case TR069_UNSIGNED:
        tr069_str2uint(str, &cksum);
        break;
    case TR069_BOOLEAN:
        if (stricmp(str, "true") == 0)
            cksum = 1;
        else if (stricmp(str, "false") == 0)
            cksum = 0;
        else
            tr069_str2uint(str, &cksum);
        break;
    case TR069_DATETIME:
        tr069_str2time(str, &cksum);
        break;
    case TR069_STRING:
    case TR069_BASE64:
        cksum = string_cksum((u_char *)tr069->buf);
        break;
    default:
        break;
    }
    return cksum;
}

int tr069_param_read_cksum(struct TR069 *tr069, struct Param *param, unsigned int *pCksum)
{
    tr069->buf[0] = 0;
    if (!param->prm_getval)
        TR069ErrorOut("prm_getint is NULL %s\n", param->name);

    if (param->prm_getval(param->name, tr069->buf, TR069_BUF_SIZE_4096))
        TR069ErrorOut("getval %s\n", param->name);

    *pCksum = int_param_cksum(tr069, param, tr069->buf);

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
void tr069_param_check(const struct TR069 *tr069)
{
    int i, num;
    struct Param *param, **table_array;

    num = tr069->table_size;
    table_array = tr069->table_array;
    for (i = 0; i < num; i ++) {
        param = table_array[i];
        if (!param)
            continue;
        if (param->type != TR069_OBJECT) {
            if (!param->prm_getval)
                TR069Warn("%d/%x getval is NULL\n", i, i);
            if (param->attr & TR069_ENABLE_WRITE) {
                 if (!param->prm_setval)
                    TR069Warn("%d/%x setval is NULL\n", i, i);
            } else {
                 if (param->prm_setval)
                    TR069Warn("%d/%x setval not NULL\n", i, i);
            }
        }
    }
}

int tr069_param_index(struct TR069 *tr069, char *name)
{
    struct Param *param;

    param = tr069_param_hash_find(tr069, name);
    if (!param)
        TR069ErrorOut("%s tr069_param_hash_find\n", name);

    return param->index;
Err:
    return -1;
}

//------------------------------------------------------------------------------
int tr069_object_add(struct TR069 *tr069, char *parentName, int instance)
{
    struct Param *param, *elem;
    char name[1024];

    param = tr069_param_hash_find(tr069, parentName);
    if (!param)
        TR069ErrorOut("%s tr069_param_hash_find\n", parentName);
    if (param->type != TR069_OBJECT || 0 == (param->attr & TR069_ENABLE_ADD))
        TR069ErrorOut("type = %d, attr = %d\n", param->type, param->attr);

    if (instance <= 0) {
        param->prm_addIndex++;
        instance = param->prm_addIndex;
    }

    sprintf(name, "%s%d.", param->name, instance);
    tr069_param_new(tr069, name, NULL, TR069_OBJECT, 0, TR069_ENABLE_READ | TR069_ENABLE_DELETE, NULL, NULL, NULL, NULL);
    param->prm_objValue.firstChild->prm_addIndex = instance;
    TR069Printf("AddObject %s\n", name);

    elem = param->prm_objVirtual.firstChild;
    while (elem) {
        sprintf(name, "%s%d.%s", param->name, instance, elem->name);

        if (TR069_STRING == elem->type || TR069_BASE64 == elem->type)
            tr069_param_new(tr069, name, NULL, elem->type, elem->prm_string->size,
                                                              elem->attr, elem->prm_getval, elem->prm_chkval, elem->prm_setval, elem->prm_onchg);
        else
            tr069_param_new(tr069, name, NULL, elem->type, 0, elem->attr, elem->prm_getval, elem->prm_chkval, elem->prm_setval, elem->prm_onchg);

        elem = elem->treeNext;
    }

    return instance;
Err:
    return -1;
}

//------------------------------------------------------------------------------
int tr069_object_delete(struct TR069 *tr069, struct Param *param)
{
    struct Param *parent;

    if (param->type != TR069_OBJECT)
        TR069ErrorOut("%s type = %d\n", param->name, param->type);
    TR069Printf("DeleteObject %s\n", param->name);

    if (param->attr & TR069_ENABLE_DELETE) {
        parent = param->treeParent;
        int_param_delete(tr069, param);

        param = parent->prm_objValue.lastChild;
        if (param)
            parent->prm_addIndex = param->prm_addIndex;
        else
            parent->prm_addIndex = 0;
    } else if (param->attr & TR069_ENABLE_ADD) {
        struct Param *child, *next;

        child = param->prm_objValue.firstChild;
        while (child) {
            next = child->treeNext;
            if (TR069_OBJECT == child->type && (child->attr & TR069_ENABLE_DELETE))
                tr069_object_delete(tr069, child);
            child = next;
        }
    } else {
        TR069ErrorOut("%s attr = %d\n", param->name, param->attr);
    }

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
void tr069_paramObject_reset(struct TR069 *tr069)
{
    struct Param *next, *param;
    struct Object *obj;

    obj = &tr069->prm_object;

    param = obj->firstChild;
    obj->firstChild = NULL;
    obj->lastChild = NULL;
    while (param) {
        next = param->paramNext;
        param->paramNext = NULL;
        param = next;
    }
    tr069->prm_num = 0;
}

//------------------------------------------------------------------------------
int tr069_paramObject_inset(struct TR069 *tr069, struct Param *param, int object)
{
    struct Object *obj;

    if (!param)
        TR069ErrorOut("param is NULL\n");

    obj = &tr069->prm_object;
    if (param->paramNext || param == obj->lastChild)
        return 0;

    if (param->type != TR069_OBJECT || object == 1) {
        if (obj->lastChild)
            obj->lastChild->paramNext = param;
        else
            obj->firstChild = param;
        obj->lastChild = param;
        tr069->prm_num++;
    }

    if (param->type == TR069_OBJECT) {
        struct Param *child = param->prm_objValue.firstChild;
        while (child) {
            tr069_paramObject_inset(tr069, child, object);
            child = child->treeNext;
        }
    }

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
int tr069_paramObject_inset_leaf(struct TR069 *tr069, const struct Param *param)
{
    struct Param *child;

    if (param->type != TR069_OBJECT)
        TR069ErrorOut("%s type = %d\n", param->name, param->type);

    child = param->prm_objValue.firstChild;
    while (child) {
        tr069_paramObject_inset(tr069, child, 0);
        child = child->treeNext;
    }

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
int tr069_paramObject_inset_index(struct TR069 *tr069, int index)
{
    struct Param *param;

    CHECK_INDEX( );
    return tr069_paramObject_inset(tr069, param, 1);
Err:
    return -1;
}

//------------------------------------------------------------------------------
//notify 更改通告，如果为false，表面ACS写入，直接写同步cksum，更改不用通知ACS
int tr069_paramObject_apply_values(struct TR069 *tr069, int notify)
{
    struct Param *param;
    struct Object *obj;
    uint32_t cksum;

    obj = &tr069->prm_object;
    for (param = obj->firstChild; param; param = param->paramNext) {
        cksum = int_param_cksum(tr069, param, param->prm_string->value);

        if (strstr(param->name, "Password"))
            TR069Debug("%s setValue *\n", param->name);
        else
            TR069Debug("%s setValue %s\n", param->name, param->prm_string->value);
        cksum = string_cksum((u_char *)param->prm_string->value);
        int_param_write(tr069, notify, param, param->prm_string->value, 0);
        if (!notify && !strncmp("Device.ManagementServer.", param->name, 24))//ACS下发，要同步到应用层
            tr069_port_setValue(param->name, param->prm_string->value, 0);

        TR069Debug("%s, cksum = %08x / %08x notify = %d\n", param->name, cksum, param->cksum, notify);
        if (notify == 0) {
            param->cksum = cksum;
            param->cksum_flag = 1;
            tr069->save_flag = 1;
        }
    }

    tr069_paramObject_reset(tr069);
    return 0;
}

//------------------------------------------------------------------------------
void tr069_paramChange_inset(struct TR069 *tr069, struct Param *param)
{
    if (param->isChange)
        return;

    param->changeNext = tr069->prm_change;
    tr069->prm_change = param;
    param->isChange = 1;
}

/*------------------------------------------------------------------------------
    Forced Inform parameters
  ------------------------------------------------------------------------------*/
int tr069_param_inform_regist(struct TR069 *tr069, char *name)
{
    int i;
    struct Param *param;

    param = tr069_param_hash_find(tr069, name);
    if (!param)
        TR069ErrorOut("%s not exist!\n", name);

    if (tr069->prm_inform_num >= TR069_PARAM_INFORM_NUM_32)
        TR069ErrorOut("prm_inform_num = %d\n", tr069->prm_inform_num);
    for (i = 0; i < tr069->prm_inform_num; i ++) {
        if (param->index == tr069->prm_inform_array[i])
            return 0;
    }
    tr069->prm_inform_array[tr069->prm_inform_num] = param->index;
    tr069->prm_inform_num++;
    return 0;
Err:
    return -1;
}

/*------------------------------------------------------------------------------
    Forced Boot parameters
  ------------------------------------------------------------------------------*/
int tr069_param_boot_regist(struct TR069 *tr069, char *name)
{
#if 1
    return tr069_param_inform_regist(tr069, name);
#else
    int i;
    struct Param *param;

    param = tr069_param_hash_find(tr069, name);
    if (!param)
        TR069ErrorOut("%s not exist!\n", name);

    if (tr069->prm_boot_num >= TR069_PARAM_BOOT_NUM_32)
        TR069ErrorOut("prm_boot_num = %d\n", tr069->prm_boot_num);
    for (i = 0; i < tr069->prm_boot_num; i ++) {
        if (param->index == tr069->prm_boot_array[i])
            return 0;
    }
    tr069->prm_boot_array[tr069->prm_boot_num] = param->index;
    tr069->prm_boot_num++;
    return 0;
Err:
    return -1;
#endif
}

/*------------------------------------------------------------------------------
    FactoryReset时某些参数值需要保持不变
  ------------------------------------------------------------------------------*/
int tr069_param_restore_regist(struct TR069 *tr069, char *name)
{
    int i;
    unsigned int size;
    struct Param *param;
    struct Param_rmn *param_rmn;

    param = tr069_param_hash_find(tr069, name);
    if (!param)
        TR069ErrorOut("%s not exist!\n", name);
    if (!(param->attr & TR069_ENABLE_WRITE))
        TR069ErrorOut("%s readonly\n", name);

    if (tr069->prm_remain_num >= TR069_PARAM_REMAIN_NUM_64)
        TR069ErrorOut("remain_num = %d\n", tr069->prm_remain_num);

    for (i = 0; i < tr069->prm_remain_num; i ++) {
        if (param->index == tr069->prm_remain_array[i].index)
            return 0;
    }
    param_rmn = &tr069->prm_remain_array[tr069->prm_remain_num];

    switch(param->type) {
    case TR069_OBJECT:
        size = 0;
        break;
    case TR069_BOOLEAN:
    case TR069_INT:
    case TR069_UNSIGNED:
        size = 12;
        break;
    case TR069_DATETIME:
        size = 20;
        break;
    case TR069_STRING:
    case TR069_BASE64:
        size = param->prm_string->size;
        break;
    default:
        TR069ErrorOut("%s type = %d\n", name, param->type);
    }

    param_rmn->string = (char *)IND_MALLOC(size);
    if (!param_rmn->string)
        TR069ErrorOut("malloc %d\n", size);

    tr069->prm_remain_num ++;
    param_rmn->index = param->index;

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
int tr069_param_restore_prepare(struct TR069 *tr069)
{
    struct Param *param;
    struct Param_rmn *param_rmn;
    unsigned int size;
    int i, idx;

    for (i = 0; i < tr069->prm_remain_num; i ++) {
        param_rmn = &tr069->prm_remain_array[i];
        idx = param_rmn->index;
        param = tr069->table_array[idx];
        if (!param || (param->attr & TR069_ENABLE_WRITE) == 0)
            continue;

        size = param->prm_string->size;
        param_rmn->string[0] = 0;
        if (param->prm_getval(param->name, param_rmn->string, size))
            TR069Warn("getval %s\n", param->name);
    }
    return 0;
}

//------------------------------------------------------------------------------
int tr069_param_restore_complete(struct TR069 *tr069)
{
    int i, index;
    struct Param *param;
    struct Param_rmn *param_rmn;

    int addrtype;
    char buf[DEVICE_LAN_AddressingType_LEN_15  + 1];

    tr069_param_read(tr069, "Device.LAN.AddressingType", buf, DEVICE_LAN_AddressingType_LEN_15  + 1);
    if (strcmp(buf, "DHCP") == 0)
        addrtype = 0;
    else if (strcmp(buf, "Static") == 0)
        addrtype = 1;
    else if (strcmp(buf, "PPPoE") == 0)
        addrtype = 2;
    else
        addrtype = -1;

    for (i = 0; i < tr069->prm_remain_num; i ++) {
        param_rmn = &tr069->prm_remain_array[i];
        index = param_rmn->index;
        param = tr069->table_array[index];
        if (!param || (param->attr & TR069_ENABLE_WRITE) == 0)
            continue;
        if ((  index == g_tr106Index->LAN_IPAddress_index
            || index == g_tr106Index->LAN_SubnetMask_index
            || index == g_tr106Index->LAN_DefaultGateway_index
            || index == g_tr106Index->LAN_DNSServers_index) && addrtype != 1)
            continue;

        if (param->prm_setval(param->name, param_rmn->string, 0))
            TR069Error("setval %s\n", param->name);
    }
    return 0;
}

enum {
    TAG_ERROR = -1,
    TAG_NONE,
    TAG_VALUE,
    TAG_BEGIN,
    TAG_END,
    TAG_SIMPLE,
};

static int int_parse_tag(char **pbuf, char *tag, int size)
{
    int ret, len;
    char *buf, *begin, *end;

    buf = *pbuf;
    //找到'<'
    begin = strchr(buf, '<');
    if (!begin)
        return TAG_NONE;
    begin++;
    end = strchr(begin, '>'); 
    if (!end)
        TR069ErrorOut("\n");

    if ('/' == begin[0]) {
        begin++;
        ret = TAG_END;
    } else {
        ret = TAG_BEGIN;
    }

    len = end - begin;
    if (len <= 0)
        TR069ErrorOut("\n");
    if ('/' == begin[len - 1]) {
        if (TAG_BEGIN != ret)
            TR069ErrorOut("\n");
        len--;
        ret = TAG_SIMPLE;
    }
    if (tag) {
        if (len >= size)
            TR069ErrorOut("len = %d / %d\n", len, size);
        memcpy(tag, begin, len);
        tag[len] = 0;
    }
    *pbuf = end + 1;

    return ret;
Err:
    return TAG_ERROR;
}

static int int_parse_line(char *buf, char *tag, int tagSize, char *value, int valueSize)
{
    int ret, len;
    char *p;

    buf = strchr(buf, '<');
    if (!buf)
        return TAG_NONE;
    if (0 == strncmp(buf, "<parameter name=\"", 17)) {
        strcpy(tag, "parameter");

        buf += 17;
        p = strchr(buf, '"');
        if (!p)
            TR069ErrorOut("\n");
        len = p - buf;
        if (len >= valueSize)
            TR069ErrorOut("len = %d / %d\n", len, valueSize);
        memcpy(value, buf, len);
        value[len] = 0;

        return TAG_BEGIN;
    }

    ret = int_parse_tag(&buf, tag, tagSize);
    if (TAG_ERROR == ret)
        TR069ErrorOut("ret = %d\n", ret);
    if (TAG_NONE == ret)
        return TAG_NONE;

    if (0 == strcmp(tag, "parameter"))
        return ret;
    switch(ret) {
    case TAG_BEGIN:
        break;
    case TAG_SIMPLE:
        value[0] = 0;
        return TAG_VALUE;
    default:
        TR069ErrorOut("ret = %d\n", ret);
    }

    p = strchr(buf, '<');
    if (!p)
        TR069ErrorOut("\n");
    len = p - buf;
    if (len >= valueSize)
        TR069ErrorOut("len = %d / %d\n", len, valueSize);
    memcpy(value, buf, len);
    value[len] = 0;

    ret = int_parse_tag(&p, NULL, 0);
    if (TAG_END != ret)
        TR069ErrorOut("ret = %d\n", ret);

    return TAG_VALUE;
Err:
    return TAG_ERROR;
}

static int int_parse_param(int *pLine, struct TR069 *tr069, FILE *fp, char *paramName, char *virtualName);
static int int_parse_virtual(int *pLine, struct TR069 *tr069, FILE *fp, char *paramName)
{
    char buf[128], tag[16], value[64];
    int ret, line;

    line = *pLine;
    while (!feof(fp)) {
        buf[0] = 0;
        fgets(buf, 128, fp);
        line++;
        *pLine = line;

        value[0] = 0;
        ret = int_parse_line(buf, tag, 16, value, 64);
        switch (ret) {
        case TAG_NONE:
            break;

        case TAG_VALUE:
            break;

        case TAG_BEGIN:
            if (strcmp(tag, "parameter"))
                TR069ErrorOut("\n");
            if (0 == value[0])
                TR069ErrorOut("\n");

            ret = int_parse_param(pLine, tr069, fp, paramName, value);
            line = *pLine;
            if (ret)
                TR069ErrorOut("ret = %d\n", ret);
            break;

        case TAG_END:
            if (strcmp(tag, "parameter"))
                goto Err;
            return 0;

        default:
            TR069ErrorOut("ret = %d\n", ret);
        }
    }

    TR069Error("incomplete! %s\n", paramName);
    return -1;
Err:
    TR069Error("line%d: %s\n", line, buf);
    return -1;
}

/*
参数配置文件由多个参数数据结构组成，采用XML格式，如下
<param>
    <name>...</name>
    <type>...</type>
    [<size>...</size>]
    <WR>...</WR>
    [<inform/>]
    [<restore/>]
    [<notification/>]
</param>
属性name是参数名称
属性type是参数数据类型包括 object, int、unsigned、boolean、string
属性size是参数值长度，只有当type为string时才有效
属性WR是读写属性，包括 r、w、wr
当参数需要在HMS请求参数重置时不改变参数值，就需要restore属性
当参数需在本地发生变更时，参数需要立即上报给HMS，就需要notification属性

当参数属性为object时，子对象的成员嵌套在这个参数结构体内，且在其他属性之
后。一般情况下object的属性WR的值为r，如果WR的值为wr，表示该参数包含0~n
个可动态增减的子对象。

属性inform是与ACS交互时要在inform消息体中出现的参数属性
*/

static int int_parse_param(int *pLine, struct TR069 *tr069, FILE *fp, char *paramName, char *virtualName)
{
    char buf[128], tag[16], value[64];
    int ret, line, type, size, attr, isRestore, isActive, isBoot, isInform;

    type = -1;
    size = 0;
    attr = 0;
    isRestore = 0;
    isActive = 0;
    isBoot = 0;
    isInform = 0;

    line = *pLine;
    while (!feof(fp)) {
        buf[0] = 0;
        fgets(buf, 128, fp);
        line++;
        *pLine = line;

        value[0] = 0;
        ret = int_parse_line(buf, tag, 16, value, 64);
        switch (ret) {
        case TAG_NONE:
            break;

        case TAG_BEGIN:
            if (strcmp(tag, "parameter") || virtualName)
                TR069ErrorOut("\n");
            if (TR069_OBJECT != type)
                TR069ErrorOut("\n");
            if (0 == value[0])
                TR069ErrorOut("\n");
            if (0 == strcmp(value, "{i}.")) {
                ret = int_parse_virtual(pLine, tr069, fp, paramName);
            } else {
                char *childName;

                size = strlen(paramName) + strlen(value) + 1;
                childName = (char*)malloc(size);
                if (!childName)
                    return -1;
                sprintf(childName, "%s%s", paramName, value);
                ret = int_parse_param(pLine, tr069, fp, childName, NULL);
                line = *pLine;
                free(childName);
            }
            line = *pLine;
            if (ret)
                return -1;
            break;

        case TAG_END:
            if (strcmp(tag, "parameter"))
                TR069ErrorOut("unknown! %s\n", tag);
            if (TR069_OBJECT != type) {
                if (virtualName) {
                    tr069_param_virtual(tr069, paramName, virtualName, type, size, attr, NULL, NULL);
                } else if (!tr069_param_hash_find(tr069, paramName)) {
                    if (isActive)
                        attr |= TR069_ENABLE_ATTR;
                    tr069_param_new(tr069, paramName, NULL, type, size, attr, NULL, NULL, NULL, NULL);
                    if (isActive)
                        tr069_param_write_attr(tr069, paramName, NOTIFICATION_ACTIVE);

                    if (isRestore)
                        tr069_param_restore_regist(tr069, paramName);
                    if (isBoot)
                        tr069_param_boot_regist(tr069, paramName);
                    if (isInform)
                        tr069_param_inform_regist(tr069, paramName);
                }
            }
            return 0;

        case TAG_VALUE:
            if (0 == strcmp(tag, "type")) {
                if (0 == strcmp(value, "object"))
                    type = TR069_OBJECT;
                else if (0 == strcmp(value, "int"))
                    type = TR069_INT;
                else if (0 == strcmp(value, "unsigned"))
                    type = TR069_UNSIGNED;
                else if (0 == strcmp(value, "boolean"))
                    type = TR069_BOOLEAN;
                else if (0 == strcmp(value, "string"))
                    type = TR069_STRING;
                else if (0 == strcmp(value, "datetime"))
                    type = TR069_DATETIME;
                else
                    TR069ErrorOut("type = %s\n", value);
            } else if (0 == strcmp(tag, "size")) {
                size = atoi(value);
            } else if (0 == strcmp(tag, "WR")) {
                if (attr)
                    TR069ErrorOut("attr = %d\n", attr);
                if (0 == strcmp(value, "wr")) {
                    if (TR069_OBJECT == type)
                        attr = TR069_ENABLE_READ | TR069_ENABLE_ADD;
                    else
                        attr = TR069_ENABLE_READ | TR069_ENABLE_WRITE;
                } else if (0 == strcmp(value, "w")) {
                    attr = TR069_ENABLE_WRITE;
                } else {
                    attr = TR069_ENABLE_READ;
                }
                if (TR069_OBJECT == type && !tr069_param_hash_find(tr069, paramName))
                    tr069_param_new(tr069, paramName, NULL, TR069_OBJECT, 0, attr, NULL, NULL, NULL, NULL);
            } else if (0 == strcmp(tag, "restore")) {
                isRestore = 1;
            } else if (0 == strcmp(tag, "boot")) {
                isBoot = 1;
            } else if (0 == strcmp(tag, "inform")) {
                isInform = 1;
            } else if (0 == strcmp(tag, "notification")) {
                isActive = 1;
            }
            break;

        default:
            TR069ErrorOut("ret = %d\n", ret);
        }
    }

    TR069Error("incomplete! %s\n", paramName);
    return -1;
Err:
    TR069Error("line%d: %s\n", line, buf);
    return -1;
}

char *g_tr069_paramPath = NULL;
void tr069_param_load(struct TR069 *tr069)
{
    FILE *fp = NULL;
    int ret, line;
    char buf[128], tag[16], value[64];

    if (!g_tr069_paramPath)
        TR069ErrorOut("paramPath is NULL\n");
    fp = fopen(g_tr069_paramPath, "r");
    if (!fp)
        TR069ErrorOut("fopen %s\n", g_tr069_paramPath);

    line = 0;
    while (!feof(fp)) {
        buf[0] = 0;
        fgets(buf, 128, fp);
        line++;
        value[0] = 0;
        ret = int_parse_line(buf, tag, 16, value, 64);
        if (TAG_NONE == ret)
            continue;
        if (TAG_BEGIN != ret || strcmp(tag, "parameter"))
            TR069ErrorOut("line%d: %s\n", line, buf);
        if (int_parse_param(&line, tr069, fp, value, NULL))
            TR069ErrorOut("int_param_parse\n");
        break;
    }
    TR069Printf("OK!\n");

Err:
    if (fp)
        fclose(fp);
}