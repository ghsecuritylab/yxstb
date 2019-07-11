
/*******************************************************************************
    Copyright (c) 2011-2012, Hybroad Vision Electronic Technology Corp
    All Rights Reserved
    Confidential Property of Hybroad Vision Electronic Technology Corp

    Revision History:

    Created: 2012-6-27 14:09:05 by liujianhua

 ******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <stdint.h>

#include "ind_mem.h"

static ind_malloc_f g_malloc_f = NULL;
static ind_realloc_f g_realloc_f = NULL;
static ind_free_f g_free_f = NULL;


typedef struct _Alloc    Alloc;
struct _Alloc {
    Alloc*   q_prev;
    Alloc*   q_next;
    Alloc*   h_next;
    int      sec;
    int      num;
    int      line;
    char*    file;
    int      check;
};

typedef struct _Handle    Handle;
struct _Handle {
    void*   ptr;
    int     size;
    Alloc*  alloc;
};

#define HASH_SIZE       256
#define ALLOC_NUM       4096
#define HANDLE_MUM      (256*1024)
#define BUFFER_SIZE     (64*1024)
#define MAX_SIZE        (128 * 1024 * 1024)

static Alloc **g_alloc_hash = NULL;
static Alloc **g_alloc_print = NULL;

static Alloc *g_alloc_pool = NULL;
static Alloc *g_alloc_queue = NULL;

static char*    g_file_buffer = NULL;
static int      g_file_off = 0;

static int g_alloc_num = 0;

static Handle *g_handle_array = NULL;
static int g_handle_num = 0;
static int g_handle_max = 0;
static int g_handle_size = 0;

static int g_index_used = 0;
static int g_index_free = 0;
static int g_debug_type = 0;

static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

static void int_chk_exit(void)
{
#if 1
        char *p = NULL;
        p[0] = 0;
#else
        int a, b = 0;
        a = 5 / b;
        printf("a = %d\n", a);
#endif
    //kill(getpid( ), SIGSEGV);
}

static int int_clock(void)
{
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    return tp.tv_sec;
}

static int int_chk_check(char *ptr, int verbose)
{
    char *p;
    uint32_t i, chk;
    int idx;

    p = ptr - 4;
    memcpy(&i, p, 4);
    idx = (int)((uint32_t)i);
    if (idx < 0 || idx > g_handle_max) {
        if (verbose)
            printf("------------------ %s:%d ERROR! ptr = %p index = %d / %d\n", __FUNCTION__, __LINE__, ptr, idx, g_handle_max);
        return -1;
    }

    if (ptr != g_handle_array[idx].ptr) {
        if (verbose)
            printf("------------------ %s:%d ERROR! ptr = %p / %p\n", __FUNCTION__, __LINE__, ptr, g_handle_array[idx].ptr);
        return -2;
    }

    p = ptr + g_handle_array[idx].size;
    memcpy(&chk, p, 4);
    if (0xffffffff != i + chk) {
        printf("------------------ %s:%d ERROR! ptr = %p index = %d\n", __FUNCTION__, __LINE__, ptr, idx);
        return -3;
    }

    return idx;
}

static int int_chk_verify(char *ptr)
{
    char* p;
    int i, idx, prev;

    idx = int_chk_check(ptr, 1);
    if (idx >= 0)
        return idx;

    if (-3 == idx) {
        printf("------------------ %s:%d ERROR! ptr = %p write outside!\n", __FUNCTION__, __LINE__, ptr);
        return -1;
    }

    prev = -1;
    for (i = 0; i <= g_handle_max; i ++) {
        p = g_handle_array[i].ptr;
        if (NULL == p)
            continue;
        if (p == ptr)
            idx = i;
        else if (p < ptr && (-1 == prev || p > (char*)g_handle_array[prev].ptr))
            prev = i;
    }

    if (idx < 0) {
        printf("------------------ %s:%d ERROR! ptr = %p not alloc!\n", __FUNCTION__, __LINE__, ptr);
        return -1;
    }

    if (-1 == prev) {
        printf("------------------ %s:%d ERROR! ptr = %p prev not exist!\n", __FUNCTION__, __LINE__, ptr);
    } else {
        Handle *handle = &g_handle_array[prev];
        printf("------------------ %s:%d ERROR! ptr = %p prev.ptr = %p, prev.size = %d, prev.alloc = %s:%d!\n", __FUNCTION__, __LINE__, ptr, handle->ptr, handle->size, handle->alloc->file, handle->alloc->line);
    }

    return -1;
}

static void* int_chk_malloc(int size, char* file, int line, int check)
{
    int idx, len;
    uint32_t chk;
    char *p, *ptr = NULL;

    p = strrchr(file, '/');
    if (p)
        file = p + 1;

    len = strlen(file);
    pthread_mutex_lock(&g_mutex);
    idx = g_index_used;

    if (size <= 0 || size >= MAX_SIZE) {
        printf("------------------ %s:%d ERROR! size = %d %s:%d\n", __FUNCTION__, __LINE__, size, file, line);
        int_chk_exit( );
        goto End;
    }

    if (NULL == g_alloc_pool || HANDLE_MUM <= g_handle_num || BUFFER_SIZE <= g_file_off + len) {
        printf("------------------ %s:%d ERROR! pool = %p, num = %d, off = %d\n", __FUNCTION__, __LINE__, g_alloc_pool, g_handle_num, g_file_off);
        int_chk_exit( );
        goto End;
    }

    p = (char *)malloc(size + 8);
    if (NULL == p) {
        printf("------------------ %s:%d ERROR! malloc size = %d %s:%d\n", __FUNCTION__, __LINE__, size, file, line);
        goto End;
    }

    while(g_index_used < HANDLE_MUM && g_handle_array[g_index_used].ptr != NULL)
        g_index_used ++;

    if (g_index_used >= HANDLE_MUM) {
        g_index_used = g_index_free;

        while(g_index_used < HANDLE_MUM && g_handle_array[g_index_used].ptr != NULL)
            g_index_used ++;

        if (g_index_used >= HANDLE_MUM) {
            printf("------------------ %s:%d ERROR! index = %d, num = %d\n", __FUNCTION__, __LINE__, g_index_used, g_handle_num);
            int_chk_exit( );
            goto End;
        }
    }

    if (idx > g_handle_max)
        g_handle_max = idx;
    if (g_index_free == g_index_used)
        g_index_free ++;
    g_index_used ++;

    ptr = p + 4;
    g_handle_array[idx].ptr = ptr;
    g_handle_array[idx].size = size;

    {
        int i;
        Alloc *alloc;

        i = (uint32_t)idx;
        chk = 0xffffffff - i;
        memcpy(p, &i, 4);
        memcpy(ptr + size, &chk, 4);

        i = line % HASH_SIZE;
        alloc = g_alloc_hash[i];
        while (alloc) {
            if (line == alloc->line && strncmp(file, alloc->file, strlen(file)) == 0)
                break;
            alloc = alloc->h_next;
        }
        if (alloc) {
            alloc->num ++;
            alloc->sec = int_clock( );

            if (alloc != g_alloc_queue) {
                Alloc *prev, *next;

                prev = alloc->q_prev;
                next = alloc->q_next;

                prev->q_next = next;
                if (next)
                    next->q_prev = prev;

                alloc->q_prev = NULL;

                g_alloc_queue->q_prev = alloc;
                alloc->q_next = g_alloc_queue;

                g_alloc_queue = alloc;
            }
        } else {
            alloc = g_alloc_pool;
            g_alloc_pool = alloc->q_next;
            g_alloc_num ++;

            alloc->num = 1;
            alloc->check = check;
            alloc->sec = int_clock( );

            alloc->line = line;
            alloc->file = g_file_buffer + g_file_off;
            g_file_off += len;
            g_file_off ++;
            strcpy(alloc->file, file);

            alloc->h_next = g_alloc_hash[i];
            g_alloc_hash[i] = alloc;

            alloc->q_next = g_alloc_queue;
            if (g_alloc_queue)
                g_alloc_queue->q_prev = alloc;
            g_alloc_queue = alloc;
        }

        g_handle_array[idx].alloc = alloc;
    }

    g_handle_size += size;
    g_handle_num ++;

End:
    pthread_mutex_unlock(&g_mutex);

    if (g_debug_type)
        printf("------------------ %s:%d ptr = %p, size = %d, index = %d %s:%d\n", __FUNCTION__, __LINE__, ptr, size, idx, file, line);
    return ptr;
}

static void int_chk_free(void* ptr, char* file, int line)
{
    char *p;
    int idx;

    if (NULL == ptr)
        return;

    p = strrchr(file, '/');
    if (p)
        file = p + 1;

    pthread_mutex_lock(&g_mutex);

    idx = int_chk_verify(ptr);
    if (idx < 0) {
        printf("------------------ %s:%d ERROR! %p %s:%d\n", __FUNCTION__, __LINE__, ptr, file, line);
        int_chk_exit( );
    }

    g_handle_size -= g_handle_array[idx].size;

    g_handle_array[idx].ptr = NULL;
    g_handle_array[idx].alloc->num --;
    if (idx < g_index_free)
        g_index_free = idx;
    g_handle_num --;

    pthread_mutex_unlock(&g_mutex);

    if (g_debug_type)
        printf("------------------ %s:%d ptr = %p %s:%d\n", __FUNCTION__, __LINE__, ptr, file, line);
    p = (char*)ptr - 4;
    free(p);
}

static void* int_chk_realloc(void* oldptr, int size, char* file, int line)
{
    char *p, *ptr;
    int idx;

    ptr = oldptr;
    if (size <= 0 || NULL == ptr) {
        if (NULL == ptr)
            return int_chk_malloc(size, file, line, 1);
        if (0 == size)
            int_chk_free(ptr, file, line);
        return NULL;
    }

    pthread_mutex_lock(&g_mutex);
    idx = int_chk_verify(ptr);
    if (idx < 0) {
        printf("------------------ %s:%d ERROR! %p %s:%d\n", __FUNCTION__, __LINE__, ptr, file, line);
        int_chk_exit( );
    }

    p = realloc(ptr - 4, size + 8);
    if (NULL == p) {
        ptr = NULL;
    } else {
        uint32_t i, chk;

        g_handle_size -= g_handle_array[idx].size;

        ptr = p + 4;
        g_handle_array[idx].ptr = ptr;
        g_handle_array[idx].size = size;

        i = (uint32_t)idx;
        chk = 0xffffffff - i;
        memcpy(p, &i, 4);
        memcpy(ptr + size, &chk, 4);

        g_handle_size += size;
    }

    pthread_mutex_unlock(&g_mutex);
    if (g_debug_type)
        printf("------------------ %s:%d ptr = %p / %p, size = %d, index = %d %s:%d\n", __FUNCTION__, __LINE__, ptr, oldptr, size, idx, file, line);
    return ptr;
}

void *_ind_malloc(int size, char *file, int line)
{
    void *p;

    if (g_malloc_f)
        p = g_malloc_f(size, file, line);
    else if (g_handle_array)
        p = int_chk_malloc(size, file, line, 1);
    else
        p = malloc(size);

    return p;
}

void *_ind_dalloc(int size, char *file, int line)
{
    void *p;

    if (g_malloc_f)
        p = g_malloc_f(size, file, line);
    else if (g_handle_array)
        p = int_chk_malloc(size, file, line, 0);
    else
        p = malloc(size);

    return p;
}

void *_ind_calloc(int size, char *file, int line)
{
    void *p = _ind_malloc(size, file, line);

    if (p)
        memset(p, 0, size);

    return p;
}

char* _ind_strdup(const char* str, char *file, int line)
{
    if(str) {
        int len = strlen(str);
        char* s = (char*)_ind_malloc(sizeof(char) * (len + 1), file, line);

        if(s) {
            memcpy(s, str, len);
            s[len] = '\0';
        }

        return s;
    }

    return NULL;
}

void* _ind_realloc(void* ptr, int size, char *file, int line)
{
    void *p;

    if (g_realloc_f)
        p = g_realloc_f(ptr, size, file, line);
    else if (g_handle_array)
        p = int_chk_realloc(ptr, size, file, line);
    else
        p = realloc(ptr, size);

    return p;
}

void _ind_verify(void* ptr, char* file, int line)
{
    char *p;
    int idx;

    if (!ptr || !g_handle_array)
        return;

    p = strrchr(file, '/');
    if (p)
        file = p + 1;

    pthread_mutex_lock(&g_mutex);

    idx = int_chk_verify(ptr);
    if (idx < 0) {
        printf("------------------ %s:%d ERROR! %p %s:%d\n", __FUNCTION__, __LINE__, ptr, file, line);
        int_chk_exit( );
    }

    pthread_mutex_unlock(&g_mutex);
}

void _ind_free(void *ptr, char *file, int line)
{
    if (g_free_f)
        g_free_f(ptr, file, line);
    else if (g_handle_array)
        int_chk_free(ptr, file, line);
    else
        free(ptr);
}

void* _ind_memcpy(void *dest, const void *src, int n, char *file, int line)
{
    int idx;

    if (g_handle_array) {
        pthread_mutex_lock(&g_mutex);

        idx = int_chk_check(dest, 0);
        if (idx > 0 && n > g_handle_array[idx].size) {
            printf("------------------ %s:%d ERROR! %p, n = %d/%d %s:%d\n", __FUNCTION__, __LINE__, dest, n, g_handle_array[idx].size, file, line);
            int_chk_exit( );
        }
    
        pthread_mutex_unlock(&g_mutex);
    }

    return memcpy(dest, src, n);
}

void* _ind_memset(void *s, int ch, int n, char *file, int line)
{
    int idx;

    if (g_handle_array) {
        pthread_mutex_lock(&g_mutex);
    
        idx = int_chk_check(s, 0);
        if (idx > 0 && n > g_handle_array[idx].size) {
            printf("------------------ %s:%d ERROR! %p, n = %d/%d %s:%d\n", __FUNCTION__, __LINE__, s, n, g_handle_array[idx].size, file, line);
            int_chk_exit( );
        }

        pthread_mutex_unlock(&g_mutex);
    }

    return memset(s, ch, n);
}

char* _ind_strcpy(char *dest, const char *src, char *file, int line)
{
    int len, idx;

    if (g_handle_array) {
        len = strlen(src);

        pthread_mutex_lock(&g_mutex);
    
        idx = int_chk_check(dest, 0);
        if (idx > 0 && len >= g_handle_array[idx].size) {
            printf("------------------ %s:%d ERROR! %p, len = %d/%d %s:%d\n", __FUNCTION__, __LINE__, dest, len, g_handle_array[idx].size, file, line);
            int_chk_exit( );
        }
    
        pthread_mutex_unlock(&g_mutex);
    }

    return strcpy(dest, src);
}

char * _ind_strncpy(char* dest, const char* src, int n, char *file, int line)
{
    int idx, len;

    if (g_handle_array) {
        len = strlen(src);
        if (len > n)
            len = n;

        pthread_mutex_lock(&g_mutex);

        idx = int_chk_check(dest, 0);
        if (idx > 0 && len >= g_handle_array[idx].size) {
            printf("------------------ %s:%d ERROR! %p, len = %d/%d %s:%d\n", __FUNCTION__, __LINE__, dest, len, g_handle_array[idx].size, file, line);
            int_chk_exit( );
        }
    
        pthread_mutex_unlock(&g_mutex);
    }

    return strncpy(dest, src, n);
}

void ind_mem_ctrl(ind_malloc_f _malloc, ind_realloc_f _realloc, ind_free_f _free)
{
    g_malloc_f = _malloc;
    g_realloc_f = _realloc;
    g_free_f = _free;
}

void ind_mem_init(int type)
{
    int i;
    Alloc* alloc;

    if (g_handle_array)
        return;
    g_debug_type = type;

    g_handle_array = (Handle*)calloc(sizeof(Handle) * HANDLE_MUM, 1);

    g_alloc_hash = (Alloc**)calloc(sizeof(Alloc*) * HASH_SIZE, 1);

    for (i = 0; i < ALLOC_NUM; i ++) {
        alloc = (Alloc*)calloc(sizeof(Alloc), 1);
        if (alloc) {
            alloc->q_next = g_alloc_pool;
            g_alloc_pool = alloc;
        }
    }

    g_alloc_print = (Alloc**)calloc(sizeof(Alloc*) * ALLOC_NUM, 1);

    g_file_buffer = (char*)calloc(BUFFER_SIZE, 1);
}

void ind_mem_chk(void)
{
    int i;
    char *ptr;
    Alloc *alloc;

    pthread_mutex_lock(&g_mutex);
    printf("------------------ %s:%d SIZE = %d, NUM = %d, num = %d, off = %d\n", __FUNCTION__, __LINE__, g_handle_size, g_handle_num, g_alloc_num, g_file_off);
    for (i = 0; i <= g_handle_max; i ++) {
        ptr = g_handle_array[i].ptr;
        if (ptr)  {
            alloc = g_handle_array[i].alloc;
            if (alloc->check && int_chk_verify(ptr) < 0) {
                printf("------------------ %s:%d ERROR! ptr = %p num = %d %s:%d\n", __FUNCTION__, __LINE__, ptr, alloc->num, alloc->file, alloc->line);
                int_chk_exit( );
            }
        }
    }

    pthread_mutex_unlock(&g_mutex);
}

void ind_mem_print(int num)
{
    int i, n, h, m, s;
    Alloc *alloc;

    if (num <= 0) {
        printf("------------------ %s:%d ERROR! num = %d\n", __FUNCTION__, __LINE__, num);
        return;
    }

    pthread_mutex_lock(&g_mutex);

    for (n = 0, alloc = g_alloc_queue; n < num && alloc; alloc = alloc->q_next) {
        if (alloc->num > 0) {
            g_alloc_print[n] = alloc;
            n ++;
        }
    }

    pthread_mutex_unlock(&g_mutex);

    s = int_clock( );
    h = s / 3600;
    m = s % 3600 / 60;
    s = s % 60;
    printf("------------------ %s:%d %03d:%02d:%02d xxx\n", __FUNCTION__, __LINE__, h, m, s);
    for (i = 0; i < n; i ++) {
        alloc = g_alloc_print[i];
        h = alloc->sec / 3600;
        m = alloc->sec % 3600 / 60;
        s = alloc->sec % 60;
        printf("------------------ %s:%d %03d:%02d:%02d num = %d %s:%d\n", __FUNCTION__, __LINE__, h, m, s, alloc->num, alloc->file, alloc->line);
    }
}

int ind_mem_show(char *file, int line, ind_show_t show)
{
    int i, num;
    Alloc *alloc;
    Handle *handle, **array = NULL;

    array = (Handle**)malloc(sizeof(Handle*) * ALLOC_NUM);
    if (!array)
        return -1;

    pthread_mutex_lock(&g_mutex);

    i = line % HASH_SIZE;
    alloc = g_alloc_hash[i];
    while (alloc) {
        if (line == alloc->line && strncmp(file, alloc->file, strlen(file)) == 0)
            break;
        alloc = alloc->h_next;
    }

    num = 0;
    for (i = 0; i <= g_handle_max && num < ALLOC_NUM; i ++) {
        handle = &g_handle_array[i];
        if (NULL != handle->ptr && alloc == handle->alloc) {
            array[num] = handle;
            num ++;
        }
    }

    pthread_mutex_unlock(&g_mutex);
    for (i = 0; i < num; i ++) {
        handle = array[i];
        if (alloc == handle->alloc && handle->ptr)
            show(i, handle->ptr);
    }

    if (array)
        free(array);

    return num;
}
