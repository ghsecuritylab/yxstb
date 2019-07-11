#ifndef _CPVRAUXTOOLS_H
#define _CPVRAUXTOOLS_H

#include <time.h>
#include <pthread.h>

#include "json/json_object.h"
#include "json/json_public.h"

namespace Hippo {
/* z_time.h */
/*{{{*/
time_t getLocalTime();
time_t timeStringToSecNum(char *timeStr);
void timeSecNnumToString(time_t secNum, char *strBuf, char insert);
/*}}}*/

/* z_mem.h */
/*{{{*/
void *mem_alloc (long nbytes, const char *file, int line);
void *mem_calloc(long count, long nbytes, const char *file, int line);
void *mem_resize(void *ptr, long nbytes, const char *file, int line);
void mem_free(void *ptr, const char *file, int line);
void show_memory();
#define MEM_ALLOC(nbytes) \
    mem_alloc((nbytes), __FILE__, __LINE__)

#define MEM_CALLOC(count, nbytes) \
    mem_calloc((count), (nbytes), __FILE__, __LINE__)

#define MEM_RESIZE(ptr, nbytes)  \
    ((ptr) = mem_resize((ptr),(nbytes), __FILE__, __LINE__))

#define MEM_FREE(ptr)  \
    ((void)(mem_free((ptr), __FILE__, __LINE__), (ptr) = 0))

#define MEM_NEW(p) ((p) = MEM_CALLOC(1, (long)sizeof *(p)))
/*}}}*/

/* z_array.h */
/*{{{*/
typedef struct {
    int length;
    int size;
    char *array;
    pthread_mutex_t mutex;
}array_t;

array_t *array_new(int length, int size);
void array_free(array_t *p);
void *array_get(array_t *p, int i);
void *array_put(array_t *p, int i, void *elem);
int array_get_length(array_t *p);
int array_get_size(array_t *p);
void array_resize(array_t *p, int length);
array_t *array_copy(array_t *p, int length);
/*}}}*/

/* json_tools.h */
/*{{{*/
void json_add_str(char *dst_str, const char *src_name, const char *src_value);
void json_add_int(char *dst_str, const char *src_name, int src_value);
void json_add_rightbrace(char *dst_str);
void backspace_comma(char *str);
int param_int_get(struct json_object* object, const char *key, int *ret_value);
int param_to_array(char *param, array_t *p_array);
int param_string_get(struct json_object* object, const char *key, char *ret_buf, int ret_buf_len);
/*}}}*/

}
#endif

