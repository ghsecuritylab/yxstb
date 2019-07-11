#include "CpvrAssertions.h"
#include "CpvrAuxTools.h"

#include "mid/mid_time.h"
#include "mid/mid_tools.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

typedef unsigned char   u_int8;

namespace Hippo {
/* z_time.c */
/*{{{*/
time_t getLocalTime()
{
    time_t timeNow = 0;
    time_t secTZAndDST = 0;

    if (1 == MID_UTC_SUPPORT) {
        secTZAndDST = mid_get_times_sec();
    }
    timeNow = time(NULL) + secTZAndDST;
    LogCpvrDebug("now time : %ld\n", timeNow);
    return timeNow;
}

time_t timeStringToSecNum(char *timeStr)
{
    time_t secNum = 0;
    time_t secTZAndDST = 0;

    if (timeStr) {
        if (1 == MID_UTC_SUPPORT) {
            secTZAndDST = mid_get_times_sec();
        }
        secNum = mid_tool_string2time(timeStr);
        secNum += secTZAndDST;
    }

    LogCpvrDebug("seconds : %ld\n", secNum);
    return secNum;
}

void timeSecNnumToString(time_t secNum, char *strBuf, char insert)
{
    time_t secTZAndDST = 0;

    if (strBuf) {
        if (1 == MID_UTC_SUPPORT) {
            secTZAndDST = mid_get_times_sec();
        }
        secNum -= secTZAndDST;
        mid_tool_time2string(secNum, strBuf, insert);
    }
}
/*}}}*/

/* z_mem.c */
/*{{{*/
typedef struct block_info{
    u_int8 *first_addr;
    size_t block_size;
    char fname[512];
    int line;
    struct block_info *next;
}block_info_st;

static block_info_st *phead = NULL;

static pthread_mutex_t block_mux_mutex = PTHREAD_MUTEX_INITIALIZER;

/* local function declarations */
static int create_block_info(u_int8 *pb, size_t size, char *file, int line);
static void delete_block_info(u_int8 *pb);
static void update_block_info(u_int8 *old_addr, u_int8 *new_addr, size_t new_size, char *file, int line);

static int create_block_info(u_int8 *pb, size_t size, char *file, int line)
{
    block_info_st *p = NULL;

    pthread_mutex_lock(&block_mux_mutex);

    if (pb != NULL && size > 0) {
        p = (block_info_st*)malloc(sizeof(*p));
        if (p != NULL) {
            p->first_addr = pb;
            p->block_size = size;
            strcpy(p->fname, file);
            p->line = line;
            p->next = phead;
            phead = p;

            pthread_mutex_unlock(&block_mux_mutex);
            return 0;
        }
    }

    pthread_mutex_unlock(&block_mux_mutex);
    return -1;
}

static void delete_block_info(u_int8 *pb)
{
    block_info_st *p = NULL, *prev = NULL;

    pthread_mutex_lock(&block_mux_mutex);

    if (pb) {
        for (p=phead; p!=NULL; p=p->next) {
            if (pb == p->first_addr) {
                if (prev == NULL) {
                    phead = p->next;
                } else {
                    prev->next = p->next;
                }
                break;
            }
            prev = p;
        }

        if (p) {
            memset(p, 0, sizeof(*p));
            free(p);
        }
    }

    pthread_mutex_unlock(&block_mux_mutex);
}

static void update_block_info(u_int8 *old_addr, u_int8 *new_addr, size_t new_size, char *file, int line)
{
    block_info_st *p = NULL;

    pthread_mutex_lock(&block_mux_mutex);

    for (p=phead; p!=NULL; p=p->next) {
        if (old_addr == p->first_addr) {
            p->first_addr = new_addr;
            p->block_size = new_size;
            strcpy(p->fname, file);
            p->line = line;
            break;
        }
    }

    pthread_mutex_unlock(&block_mux_mutex);
}

void show_memory()
{
    block_info_st *p;

    pthread_mutex_lock(&block_mux_mutex);

    printf("\n");
    printf("\tfirst addr\tblock size\tline\tfile name\n");
    printf("--------------------------------------------------------------------------------------------\n");
    for (p=phead; p!=NULL; p=p->next) {
        printf("  %p\t %d\t %d\t %s\n", p->first_addr, p->block_size, p->line, p->fname);
    }
    pthread_mutex_unlock(&block_mux_mutex);
}

void *mem_alloc(long nbytes, const char *file, int line)
{
    void *ptr = NULL;

    if (nbytes > 0) {
        ptr = (void*)malloc(nbytes);
        if (ptr) {
            memset(ptr,0, nbytes);
#ifdef Z_MEMORY_DEBUG
            create_block_info(ptr, nbytes, file, line);
#endif
        }
    }

    return ptr;
}

void *mem_calloc(long count, long nbytes, const char *file, int line)
{
    void *ptr = NULL;

    if (nbytes > 0 && count > 0) {
        ptr = (void*)calloc(count, nbytes);
        if (ptr) {
#ifdef Z_MEMORY_DEBUG
            create_block_info(ptr, nbytes*count, file, line);
#endif
        }
    }

    return ptr;
}

void *mem_resize(void *ptr, long nbytes, const char *file, int line)
{
    void *ptr_bak = NULL;

    if(nbytes > 0 && ptr != NULL) {
        ptr_bak = ptr;
        ptr = (void*)realloc(ptr, nbytes);
        if (ptr != NULL) {
#ifdef Z_MEMORY_DEBUG
            update_block_info(ptr_bak, ptr, nbytes, file, line);
#endif
            return ptr;
        }
    }
    return NULL;
}

void mem_free(void *ptr, const char *file, int line)
{
    if (ptr) {
#ifdef Z_MEMORY_DEBUG
        delete_block_info(ptr);
#endif
        free(ptr);
    }
}
/*}}}*/

/* z_array.c */
/*{{{*/
array_t *array_new(int length, int size)
{
    array_t *p = NULL;

    if (size > 0) {
        p = (array_t*) MEM_CALLOC(1, (long)sizeof(array_t));
        if (p) {
            p->length = length;
            p->size   = size;
            if (length > 0) {
                p->array = (char*)MEM_CALLOC(length, size);
            } else {
                p->array = NULL;
            }
            pthread_mutex_init (&(p->mutex),NULL);
        }
    }

    return p;
}

void array_free(array_t *p)
{
    if (p != NULL) {
        MEM_FREE(p->array);
        MEM_FREE(p);
    }
}

void *array_get(array_t *p, int i)
{
    void *ptr = NULL;

    if (p != NULL) {
        pthread_mutex_lock( &p->mutex );

        if (i >= 0 && i < p->length)
            ptr = p->array + i*p->size;

        pthread_mutex_unlock( &p->mutex );
    }

    return ptr;
}

void *array_put(array_t *p, int i, void *elem)
{
    void *ptr = NULL;

    if (p != NULL) {
        pthread_mutex_lock( &p->mutex );

        if (i >= 0 && i < p->length && elem != NULL) {
            memcpy(p->array + i*p->size, elem, p->size);
            ptr = elem;
        }

        pthread_mutex_unlock( &p->mutex );
    }

    return ptr;
}

int array_get_length(array_t *p)
{
    int result = -1;

    if (p != NULL) {
        pthread_mutex_lock( &p->mutex );

        result = p->length;

        pthread_mutex_unlock( &p->mutex );
    }

    return result;
}

int array_get_size(array_t *p)
{
    int result = -1;

    if (p != NULL) {
        pthread_mutex_lock( &p->mutex );

        result = p->size;

        pthread_mutex_unlock( &p->mutex );
    }

    return result;
}

void array_resize(array_t *p, int length)
{
    if (p != NULL && length >= 0) {
        pthread_mutex_lock( &p->mutex );

        if (length == 0)
            MEM_FREE(p->array);
        else if (p->length == 0)
            p->array = (char*)MEM_ALLOC(length*p->size);
        else
            p->array =(char*) mem_resize(p->array, length*p->size, __FILE__, __LINE__);

        p->length = length;

        pthread_mutex_unlock( &p->mutex );
    }
}

array_t *array_copy(array_t *p, int length)
{
    array_t *copy = NULL;

    if (p != NULL && length >= 0) {
        pthread_mutex_lock( &p->mutex );

        copy = array_new(length, p->size);
        if (copy != NULL) {
            if (copy->length >= p->length && p->length > 0)
                memcpy(copy->array, p->array, p->length);
            else if (p->length > copy->length && copy->length > 0)
                memcpy(copy->array, p->array, copy->length);
        }

        pthread_mutex_unlock( &p->mutex );
    }

    return copy;
}
/*}}}*/

/* json_tools.c */
/*{{{*/
void json_add_str(char *dst_str,const char *src_name, const char *src_value)
{
    char *tmp = NULL;

    if (dst_str && src_name && src_value) {
        char *p_src = NULL, *p = NULL;

        p_src = (char *)MEM_ALLOC(strlen(src_value)*2 + 1);
        if (p_src) {
            int i = 0;

            p = p_src;
            while (i < strlen(src_value)) {
                if (src_value[i] == '"' && (i == 0 || src_value[i-1] != '\\')) {
                    *p = '\\';
                    p++;
                } else if (src_value[i] == '\\' && src_value[i+1] == '\'') {
                    i++;
                }

                *p = src_value[i];
                p++;
                i++;
            }
            *p = 0;

            tmp = (char *)MEM_ALLOC(strlen(src_name) + strlen(p_src) + 6  +1);

            sprintf(tmp, "\"%s\":\"%s\",", src_name, p_src);
            strcat(dst_str, tmp);
            if (p_src) MEM_FREE(p_src);
            if (tmp) MEM_FREE(tmp);
        }
    }
}

void json_add_int(char *dst_str, const char *src_name, int src_value)
{
    char *tmp = NULL;

    if (dst_str && src_name) {
        tmp = (char *)MEM_ALLOC(strlen(src_name) + 12 + 3  +1);
        sprintf(tmp, "\"%s\":%d,", src_name, src_value);
        strcat(dst_str, tmp);
        if (tmp)
            MEM_FREE(tmp);
    }
}

void backspace_comma(char *str)
{
    if (str[strlen(str) - 1] == ','){
        str[strlen(str) - 1] = 0;
    }
}

void json_add_rightbrace(char *dst_str)
{
    int len;

    if (dst_str) {
        len = strlen(dst_str);
        if (dst_str[len - 1] == ',') {
            dst_str[len - 1] = '}';
        } else {
            dst_str[len] = '}';
            dst_str[len + 1] = 0;
        }
    }
}

int param_string_get(struct json_object* object, const char *key, char *ret_buf, int ret_buf_len)
{
    char *value = NULL;
    struct json_object *sub_object = NULL;

    sub_object = (struct json_object*)json_object_get_object_bykey(object, key);
    if (sub_object) {
        value = (char *)json_get_object_string(sub_object);
        if (value)  {
            int i = 0, j = 0;
            char *p_dst = NULL, *p_src = NULL;

            p_dst = ret_buf;
            p_src = value;
            while (j < ret_buf_len && i < strlen(value)) {
                if (p_src[i] == '\'' && (i == 0 || p_src[i-1] != '\\')) {
                    if (j+2 >= ret_buf_len)
                        break;
                    p_dst[j++] = '\\';
                    p_dst[j++] = '\'';
                    i++;

                } else
                    p_dst[j++] = p_src[i++];
            }
            p_dst[j] = 0;

            LogCpvrDebug( "%s is %s\n", key, ret_buf);
            return 0;
        }
    }
    return -1;
}

int param_int_get(struct json_object* object, const char *key, int *ret_value)
{
    int value = 0;
    struct json_object* sub_object = NULL;

    sub_object = (struct json_object*)json_object_get_object_bykey(object, key);
    if (sub_object) {
        value = json_get_object_int(sub_object);
        if (ret_value)
            *ret_value = value;
        LogCpvrDebug( "%s is %d\n", key, value);
        return 0;
    }

    return -1;
}

int param_to_array(char *param, array_t *p_array)
{
    int index = 0, cp_len = 0;

    if (param && p_array) {
        char *p_temp, *p_start, *p_end, *p_buf;

        p_temp = param;
        p_buf = (char*)MEM_ALLOC(p_array->size);
        p_start = strchr(p_temp, '"');
        while(p_buf && p_start != NULL) {
            p_start ++;
            p_temp = p_start;
            p_end = strchr(p_temp, '"');
            if (p_end == NULL)
                break;

            cp_len = (p_end - p_start) > (p_array->size - 1) ? (p_array->size - 1) : (p_end - p_start);
            strncpy(p_buf, p_start, cp_len);
            *(p_buf + cp_len) = '\0';

            if (index >= p_array->length)
                array_resize(p_array, p_array->length * 2);
            array_put(p_array, index, p_buf);
            index ++;
            p_temp = p_end + 1;
            p_start = strchr(p_temp, '"');
        }

        if (p_buf) MEM_FREE(p_buf);
    }
}
/*}}}*/
}
