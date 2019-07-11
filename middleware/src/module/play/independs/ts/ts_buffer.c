
/*
    2013-3-19 1:03:44 因OTT解扰需求而增加 reload 相关函数以及成员
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app/Assertions.h"
#include "ind_ts.h"
#include "ind_string.h"
#include "ind_mem.h"

struct ts_buf {
    int     size;

    int     read;

    int     write;

    int     reload;

    char*   buffer;
};

ts_buf_t ts_buf_create(int size)
{
    ts_buf_t sb = NULL;

    PRINTF("size = %d\n", size);

    sb = (ts_buf_t)IND_MALLOC(sizeof(struct ts_buf));
    if (sb == NULL)
        ERR_OUT("malloc\n");
    IND_MEMSET(sb, 0, sizeof(struct ts_buf));

    sb->reload = -1;
    sb->buffer = (char *)IND_MALLOC(size);
    if (NULL == sb->buffer)
        ERR_OUT("malloc %d\n", size);

    sb->size = size;

    return sb;
Err:
    if (sb)
        IND_FREE(sb);

    return NULL;
}

ts_buf_t ts_buf_reload(char* buffer, int size)
{
    ts_buf_t sb = NULL;

    PRINTF("buffer = %p, size = %d\n", buffer, size);

    sb = (ts_buf_t)IND_MALLOC(sizeof(struct ts_buf));
    if (sb == NULL)
        ERR_OUT("malloc\n");
    IND_MEMSET(sb, 0, sizeof(struct ts_buf));

    sb->buffer = buffer;

    sb->size = size;

    return sb;
Err:
    if (sb)
        IND_FREE(sb);

    return NULL;
}

void ts_buf_delete(ts_buf_t sb)
{
    if (sb == NULL)
        return;

    if (-1 == sb->reload && sb->buffer)
        IND_FREE(sb->buffer);

    IND_FREE(sb);
}

int ts_buf_size(ts_buf_t sb)
{
    return sb->size;
}

void ts_buf_reset(ts_buf_t sb)
{
    sb->write = 0;

    sb->read = 0;

    if (sb->reload > 0)
        sb->reload = 0;
}

void ts_buf_print(ts_buf_t sb)
{
    PRINTF("write = %d, read = %d, size = %d\n", sb->write, sb->read, sb->size);
}

int ts_buf_length(ts_buf_t sb)
{
    int length;

    if (sb->buffer == NULL)
        return 0;

    length = sb->write - sb->read;

    if (length < 0)
        WARN_PRN("write = %d, read = %d\n", sb->write, sb->read);

    return length;
}

static int int_buf_write_len(ts_buf_t sb)
{
    int len;

    if (sb->write >= sb->size)
        len = sb->size + sb->read - sb->write;
    else
        len = sb->size - sb->write;

    if (len < 0)
        ERR_OUT("write = %d, read = %d\n", sb->write, sb->read);

    return len;
Err:
    return 0;
}

void ts_buf_write_get(ts_buf_t sb, char **buf, int *len)
{
    int length = int_buf_write_len(sb);

    if (sb->buffer == NULL || length <= 0) {
        *len = 0;
        *buf = NULL;
        WARN_PRN("buffer = %p, write = %d, read = %d\n", sb->buffer, sb->write, sb->read);
    } else {
        *len = length;
        if (sb->write >= sb->size)
            *buf = sb->buffer + (sb->write - sb->size);
        else
            *buf = sb->buffer + sb->write;
    }
}

int ts_buf_write_put(ts_buf_t sb, int len)
{
    int length = int_buf_write_len(sb);

    if (len <= 0)
        ERR_OUT("len = %d\n", len);

    if (len > length)
        ERR_OUT("len = %d, length = %d\n", len, length);

    sb->write += len;

    return 0;
Err:
    return -1;
}

static int int_buf_reload_len(ts_buf_t sb)
{
    int len;

    if (sb->reload < sb->size && sb->write > sb->size)
        len = sb->size - sb->reload;
    else
        len = sb->write - sb->reload;

    if (len < 0)
        ERR_OUT("write = %d, reload = %d\n", sb->write, sb->reload);

    return len;
Err:
    return 0;
}

void ts_buf_reload_get(ts_buf_t sb, char **pbuf, int *plen)
{
    int length;

    if (NULL == sb->buffer || -1 == sb->reload)
        goto Err;

    length = int_buf_reload_len(sb);

    *plen = length;
    if (pbuf) {
        if (sb->reload >= sb->size)
            *pbuf = sb->buffer + (sb->reload - sb->size);
        else
            *pbuf = sb->buffer + sb->reload;
    }
    return;
Err:
    *plen = 0;
    if (pbuf)
        *pbuf = NULL;
}

int ts_buf_reload_mark(ts_buf_t sb, int len)
{
    int length;

    if (len <= 0)
        ERR_OUT("len = %d\n", len);

    length = int_buf_reload_len(sb);
    if (len > length)
        ERR_OUT("len = %d, length = %d\n", len, length);

    sb->reload += len;

    return 0;
Err:
    return -1;
}

static int int_buf_read_len(ts_buf_t sb)
{
    int len;

    if (sb->reload >= 0) {
		if (sb->reload > sb->size)
            len = sb->size - sb->read;
        else
            len = sb->reload - sb->read;
    } else {
        if (sb->write > sb->size)
            len = sb->size - sb->read;
        else
            len = sb->write - sb->read;
    }

    if (len < 0)
        ERR_OUT("write = %d, read = %d, reload = %d\n", sb->write, sb->read, sb->reload);

    return len;
Err:
    return 0;
}

void ts_buf_read_get(ts_buf_t sb, char **pbuf, int *plen)
{
    int length;

    if (NULL == sb->buffer)
        goto Err;

    length = int_buf_read_len(sb);

    *plen = length;
    *pbuf = sb->buffer + sb->read;
    return;
Err:
    *plen = 0;
    *pbuf = NULL;
}

int ts_buf_read_pop(ts_buf_t sb, int len)
{
    int length;

    if (len <= 0)
        ERR_OUT("len = %d\n", len);

    length = int_buf_read_len(sb);
    if (len > length)
        ERR_OUT("len = %d, length = %d\n", len, length);

    sb->read += len;

    if (sb->read >= sb->size) {
        sb->write -= sb->size;
        sb->read = 0;

        if (sb->reload >= 0)
            sb->reload -= sb->size;
    }

    return 0;
Err:
    return -1;
}

int ts_buf_read(ts_buf_t sb, char *buffer, int size)
{
    int len, bytes;
    char *buf;

    if (NULL == sb->buffer || size <= 0)
        return 0;

    bytes = 0;
    while(size > 0) {
        len = 0;
        ts_buf_read_get(sb, &buf, &len);
        if (len <= 0)
            break;
        if (len > size)
            len = size;
        if (buffer) {
            IND_MEMCPY(buffer, buf, len);
            buffer += len;
        }
        size -= len;

        bytes += len;
        ts_buf_read_pop(sb, len);
    }

    return bytes;
}

int ts_buf_peek(ts_buf_t sb, int off, char *buf, int size)
{
    int len;

    if (NULL == sb->buffer || size <= 0 || sb->write <= off)
        return 0;

    len = sb->write - off;
    if (len > size)
        len = size;
    if (len > 0)
        IND_MEMCPY(buf, sb->buffer + off, len);

    return len;
}

int ts_buf_memstr(ts_buf_t sb, char* str)
{
    int off, len, len0, len1;
    char *p, *buf0, *buf1;

    len = strlen(str);
    if (NULL == sb->buffer || len <= 0)
        return -1;

    buf0 = sb->buffer + sb->read;
    if (sb->write > sb->size)
        len0 = sb->size - sb->read;
    else
        len0 = sb->write - sb->read;

    if (len0 >= len) {
        p = ind_memstr(buf0, len0, str);//命令结束符
        if (p)
            return (int)(p - buf0);

        off = len0 - (len - 1);
        buf0 += off;
        len0 -= off;
    } else {
        off = 0;
    }

    if (sb->write <= sb->size)
        return -1;

    buf1 = sb->buffer;
    len1 = sb->write - sb->size;

    while (len0 > 0) {
        if (buf0[0] == str[0]) {
            if (len > len0 + len1)
                return -1;
            if (0 == memcmp(buf0, str, len0) && 0 == memcmp(buf1, str + len0, len - len0))
                return off;
        }
        off++;
        buf0++;
        len0--;
    }

    p = ind_memstr(buf1, len1, str);//命令结束符
    if (p)
        return (off + (int)(p - buf1));

    return -1;
}
