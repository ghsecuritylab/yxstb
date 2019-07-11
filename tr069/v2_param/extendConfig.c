
#include "tr069_debug.h"

#include "extendConfig.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <pthread.h>

enum {
	CFG_TYPE_OBJECT = 1,
	CFG_TYPE_INT,
	CFG_TYPE_UINT,
	CFG_TYPE_INT64,
	CFG_TYPE_STRING
};

#define PRINT_SPACE_SIZE_256	256
#define NAME_FULL_SIZE_64		64

#define NAME_SIZE_64			64
#define VALUE_SIZE_256			256

#define HASH_SIZE_199           199

typedef struct __CfgParam CfgParam;
struct __CfgParam {
	CfgParam *next;/* hash table next */
	CfgParam *brother;

	int type;

	int len;//字符串长度

	union {
		int *addr_int;
		uint32_t *addr_uint;
		char *addr_string;
		CfgParam *child;
	} val;

	char name[4];/* name的长度是可变的 */
};

typedef struct __CfgTree CfgTree;
struct __CfgTree {
	CfgParam *prm_hash[HASH_SIZE_199];
};

static CfgTree *gTree = NULL;

static pthread_mutex_t gMutex = PTHREAD_MUTEX_INITIALIZER;

//------------------------------------------------------------------------------
void extendConfigInit(void)
{
    pthread_mutex_lock(&gMutex);
    if (!gTree)
	    gTree = (CfgTree *)calloc(sizeof(CfgTree), 1);
    pthread_mutex_unlock(&gMutex);
}

#define HASH_SIZE_199   199
static uint32_t stringHash(const u_char *s)
{
	uint32_t h = 0;
	while (*s)
		h = 65599 * h + *s ++;
  return h % HASH_SIZE_199;
}

//------------------------------------------------------------------------------
static CfgParam *param_find(CfgTree *tree, const char *paramname)
{
	CfgParam *param;
	uint32_t idx;

	if (paramname == NULL)
		TR069ErrorOut("paramname is NULL\n");
	idx = stringHash((unsigned char *)paramname);
	param = tree->prm_hash[idx];

	while(param && strcmp(param->name, paramname))
		param = param->next;

	return param;
Err:
	return NULL;
}

//------------------------------------------------------------------------------
static int param_inset(CfgTree *tree, CfgParam *param)
{
	char *p;
	uint32_t idx;
	uint32_t len;
	char name[NAME_FULL_SIZE_64];

	if (param_find(tree, param->name))
		TR069ErrorOut("exist\n");
	strcpy(name, param->name);
	len = strlen(name);
	if (name[len - 1] == '.')
		name[len - 1] = 0;
	p = strrchr(name, '.');
	if (p) {
			CfgParam *parent;

			p[0] = 0;
			parent = param_find(tree, name);
			if (parent == NULL)
				TR069ErrorOut("parent not exist\n");
			if (parent->val.child) {
				CfgParam *child = parent->val.child;
				while(child->brother)
					child = child->brother;
				child->brother = param;
			} else {
				parent->val.child = param;
			}
			param->brother = NULL;
	}

	idx = stringHash((unsigned char *)param->name);
	param->next = tree->prm_hash[idx];
	tree->prm_hash[idx] = param;

	return 0;

Err:
	return -1;
}

//------------------------------------------------------------------------------
static int param_print(CfgParam *param, char *buf, int len)
{
	int length, l, i;
	char *name, *p, ch;
	CfgParam *child;

	if (len < (NAME_SIZE_64 + VALUE_SIZE_256))
		TR069ErrorOut("len = %d\n", len);

	length = 0;

	if (CFG_TYPE_OBJECT == param->type) {
		child = param->val.child;
		while(child) {
			l = param_print(child, buf + length, len - length);
			if (l < 0)
				TR069ErrorOut("%s param_print = %d\n", param->name, l);
			length += l;
			child = child->brother;
		}
	} else {
		name = strchr(param->name, '.');
		if (name == NULL)
			TR069ErrorOut("param->name = %s\n", param->name);
		name ++;

	    switch(param->type) {
    	case CFG_TYPE_OBJECT:
    		break;
    	case CFG_TYPE_INT:
    		length = sprintf(buf, "%s=%d\n", name, *param->val.addr_int);
    		break;
    	case CFG_TYPE_STRING:
    		l = strlen(param->val.addr_string);
    		if (l >= param->len)
    			TR069ErrorOut("%s valuelen = %d/%d\n", param->name, l, param->len);
    		length = sprintf(buf, "%s=", name);
    		buf += length;
    		p = param->val.addr_string;
    		for (i = 0; i < l; i ++) {
    			ch = p[i];
    			if (ch == '\n' || ch == '\\') {
    				*buf ++ = '\\';
    				length ++;
    			}
    			*buf ++ = ch;
    			length ++;
    		}
    		*buf = '\n';
    		length ++;
    		break;
    	default:
    		TR069ErrorOut("%s type = %d\n", param->name, param->type);
    	}
	}

	return length;
Err:
	return -1;
}

//------------------------------------------------------------------------------
static CfgParam *param_cfg_inset(CfgTree *tree, const char *paramname)
{
	int len;
	CfgParam *param;

	if (!paramname)
		TR069ErrorOut("paramname = %p\n", paramname);
	len = strlen(paramname);
	if (len == 0 || len >= NAME_FULL_SIZE_64)
		TR069ErrorOut("paramname = %s\n", paramname);

	param = param_find(tree, paramname);
	if (param)
		TR069ErrorOut("param already exist\n");

	len = (len + sizeof(CfgParam)) & 0xfffc;
	param = (CfgParam *)calloc(len, 1);
	if (!param)
		TR069ErrorOut("malloc\n");

	strcpy(param->name, paramname);
	if (param_inset(tree, param))
		TR069ErrorOut("param_inset\n");

	return param;
Err:
	return NULL;
}

//------------------------------------------------------------------------------
int extendConfigInsetObject(const char *paramname)
{
    int ret = -1;
    CfgTree *tree;
	CfgParam *param;

    pthread_mutex_lock(&gMutex);
    tree = gTree;
    if (!tree)
        goto Err;

	if (!paramname)
		TR069ErrorOut("paramname is NULL\n");

	param = param_cfg_inset(tree, paramname);
	if (!param)
		TR069ErrorOut("tree_cfg_inset %s\n", paramname);

	param->type = CFG_TYPE_OBJECT;

	ret = 0;
Err:
    pthread_mutex_unlock(&gMutex);
	return ret;
}

//------------------------------------------------------------------------------
int extendConfigInsetInt(const char *paramname, int *addr)
{
    int ret = -1;
    CfgTree *tree;
	CfgParam *param;

    pthread_mutex_lock(&gMutex);
    tree = gTree;
    if (!tree)
        goto Err;

	if (!paramname || !addr)
		TR069ErrorOut("paramname = %p, addr = %p\n", paramname, addr);

	param = param_cfg_inset(tree, paramname);
	if (!param)
		TR069ErrorOut("tree_cfg_inset %s\n", paramname);

	param->type = CFG_TYPE_INT;
	param->val.addr_int = addr;

	ret = 0;
Err:
    pthread_mutex_unlock(&gMutex);
	return ret;
}

//------------------------------------------------------------------------------
int extendConfigInsetUnsigned(const char *paramname, unsigned int *addr)
{
    return extendConfigInsetInt(paramname, (int *)addr);
}

//------------------------------------------------------------------------------
int extendConfigInsetString(const char *paramname, char *addr, int len)
{
    int ret = -1;
    CfgTree *tree;
	CfgParam *param;

    pthread_mutex_lock(&gMutex);
    tree = gTree;
    if (!tree)
        goto Err;

	if (!paramname || !addr || len <= 0)
		TR069ErrorOut("paramname = %p, addr = %p, len = %d\n", paramname, addr, len);

	param = param_cfg_inset(tree, paramname);
	if (!param)
		TR069ErrorOut("tree_cfg_inset %s\n", paramname);

	param->type = CFG_TYPE_STRING;
	param->len = len;
	param->val.addr_string = addr;

	ret = 0;
Err:
    pthread_mutex_unlock(&gMutex);
	return ret;
}

//------------------------------------------------------------------------------
static int config_elem(const char *buf, int len, int line, char *name, int nsize, char *value, int vsize)
{
	char ch;
	int i;

	for (i = 0; (isalnum(buf[i]) || buf[i] == '.' || buf[i] == '_') && i < len; i ++) ;
	if (i == 0 || i >= len)
		TR069ErrorOut("line %d i = %d / %d\n", line, i, len);

	if (i >= nsize)
		TR069ErrorOut("line %d name_len = %d / %d\n", line, i, nsize);
	memcpy(name, buf, i);
	name[i] = 0;
	buf += i;
	len -= i;

	if (buf[0] != '=')
		TR069ErrorOut("line %d name = %s, op = 0x%02x\n", line, name, (uint32_t)((uint8_t)buf[0]));
	buf ++;
	len --;

	i = 0;
	ch = *buf ++;
	while(ch != '\n' && ch != 0x00 && len > 0) {
		if (ch == '\\') {
			ch = *buf ++;
			len --;
			if (len < 0)
				TR069ErrorOut("line %d error\n", line);
			value[i ++] = ch;
		} else {
			value[i ++] = ch;
		}
		if (i >= vsize)
			TR069ErrorOut("line %d too large\n", line);
		ch = *buf ++;
		len --;
	}
	value[i] = 0;

	return 0;
Err:
	return -1;
}

//------------------------------------------------------------------------------
static int config_input(CfgTree *tree, const char *rootname, char *buf, int len)
{
	int l, ret, line, rlen;
	char *p;
	CfgParam *param;
	char name[NAME_SIZE_64], value[VALUE_SIZE_256];

	if (rootname == NULL || buf == NULL || len < 0)
		TR069ErrorOut("rootname = %p, buf = %p, len = %d\n", rootname, buf, len);
	if (buf[len] != 0)
		TR069ErrorOut("EOS not exist\n");

	line = 1;
	rlen = sprintf(name, "%s.", rootname);
	if (rlen >= NAME_SIZE_64)
		TR069ErrorOut("rootname = %s\n", rootname);
	while (1) {
		while (isspace(buf[0]) && len > 0) {
			if (buf[0] == '\n')
				line ++;
			buf ++;
			len --;
		}
		if (len <= 0)
			break;

		p = buf;
		l = 0;
		while (*p != '\n' && l < len) {
			p ++;
			l ++;
		}

		ret = config_elem(buf, l, line, name + rlen, NAME_SIZE_64 - rlen, value, VALUE_SIZE_256);
		buf += l;
		len -= l;
		if (ret)
			continue;

		if (strncmp(rootname, name, strlen(rootname))) {
			TR069Warn("%s not child of %s at line %d\n", name, rootname, line);
			continue;
		}
		param = param_find(tree, name);
		if (param) {
			switch(param->type) {
			case CFG_TYPE_OBJECT:
				TR069Warn("%s is object at line %d\n", name, line);
				break;
			case CFG_TYPE_INT:
				if (value[0] == 0) {
					TR069Warn("value of %s error at line %d\n", name, line);
					break;
				}
				*param->val.addr_int = atoi(value);
				break;
			case CFG_TYPE_STRING:
				l = strlen(value);
				if (l >= param->len) {
					TR069Warn("value of %s too large at line %d\n", name, line);
					break;
				}
				strcpy(param->val.addr_string, value);
				break;
			default:
				break;
			}
		}
	}

	return 0;
Err:
	return -1;
}

//------------------------------------------------------------------------------
static int config_output(CfgTree *tree, const char *rootname, char *buf, int len)
{
	CfgParam *param;

	if (!rootname || !buf || len < PRINT_SPACE_SIZE_256)
		TR069ErrorOut("rootname = %p, buf = %p, len = %d\n", rootname, buf, len);

	param = param_find(tree, rootname);
	if (param == NULL)
		TR069ErrorOut("%s not exist\n", rootname);
	buf[0] = 0;

	len = param_print(param, buf, len);
	if (len < 0)
		TR069ErrorOut("%s param_print = %d\n", rootname, len);

	return len;
Err:
	return -1;
}

//------------------------------------------------------------------------------
static int param_size(CfgParam *param)
{
	int size, s;
	char *name, *p, ch;
	CfgParam *child;

	size = 0;

	if (CFG_TYPE_OBJECT == param->type) {
		child = param->val.child;
		while(child) {
		    s = param_size(child);
		    if (s <= 0)
			    TR069ErrorOut("name = %s, size = %d\n", child->name, s);
			size += s;
			child = child->brother;
		}
	} else {
		name = strchr(param->name, '.');
		if (!name)
			TR069ErrorOut("Invalid param name = %s\n", param->name);
		name++;

	    switch(param->type) {
    	case CFG_TYPE_OBJECT:
			TR069ErrorOut("Invalid object %s\n", param->name);
    	case CFG_TYPE_INT:
    	    size = strlen(name) + 1 + 10 + 1;
    		break;
    	case CFG_TYPE_STRING:
    	    size = strlen(name) + 1 + param->len + 1;
    		break;
    	default:
    		TR069ErrorOut("%s type = %d\n", param->name, param->type);
    	}
	}

	return size;
Err:
	return -1;
}

void extendConfigRead(const char *rootname)
{
    FILE *fp = NULL;
    char *buf = NULL;
    int len, size;
	CfgParam *param;
	char fileName[256];

    pthread_mutex_lock(&gMutex);

    size = 0;
	param = param_find(gTree, rootname);
    if (!param)
		TR069ErrorOut("%s not exist\n", rootname);
    if (CFG_TYPE_OBJECT != param->type)
		TR069ErrorOut("%s type is %d\n", rootname, param->type);

    size = param_size(param);
    if (size <= 0)
		TR069ErrorOut("size = %d\n", size);

    buf = malloc(size);
    if (!buf)
		TR069ErrorOut("malloc size = %d\n", size);

    sprintf(fileName, "/data/yx_config_%s.cfg", rootname);

    fp = fopen(fileName, "rb");
    if (!fp)
		TR069ErrorOut("fopen %s!\n", fileName);

    len = fread(buf, 1, size, fp);
    if (len <= 0 || len >= size)
        TR069ErrorOut("len = %d / size = %d!\n", len, size);

	buf[len] = 0;
	config_input(gTree, rootname, buf, len);

Err:
    pthread_mutex_unlock(&gMutex);
    if (fp)
        fclose(fp);
	if (buf)
	    free(buf);
}

void extendConfigWrite(const char *rootname)
{
    FILE *fp = NULL;
    char *buf = NULL;
    int l, len, size;
	CfgParam *param;
	char fileName[256];

    pthread_mutex_lock(&gMutex);

    size = 0;
	param = param_find(gTree, rootname);
    if (!param)
		TR069ErrorOut("%s not exist\n", rootname);
    if (CFG_TYPE_OBJECT != param->type)
		TR069ErrorOut("%s type is %d\n", rootname, param->type);

    size = param_size(param);
    if (size <= 0)
		TR069ErrorOut("size = %d\n", size);

    size += NAME_SIZE_64 + VALUE_SIZE_256;
    buf = malloc(size);
    if (!buf)
		TR069ErrorOut("malloc size = %d\n", size);

    sprintf(fileName, "/data/yx_config_%s.cfg", rootname);

    fp = fopen(fileName, "wb");
    if (!fp)
		TR069ErrorOut("fopen %s!\n", fileName);

	len = config_output(gTree, rootname, buf, size);
	buf[len] = 0;

    l = fwrite(buf, 1, len, fp);
    if (l != len)
		TR069ErrorOut("fwrite %d / %d!\n", l, len);

Err:
    pthread_mutex_unlock(&gMutex);
    if (fp)
        fclose(fp);
	if (buf)
	    free(buf);
}
