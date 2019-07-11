
#include <sys/stat.h>
#include <stdio.h>

#include "mid_stream.h"
#include "stream_port.h"
#include "Assertions.h"
#include "ind_mem.h"



#ifdef HUAWEI_C10
static char *g_buffer = NULL;
static int g_buffer_len = 0;
static int g_buffer_point = 0;
#endif

int stream_port_fsize(char* filepath)
{
    struct stat info;

    if(filepath == NULL)
        ERR_OUT("filepath is NULL\n");
    if(stat(filepath, &info))
        ERR_OUT("stat %s\n", filepath);

    return (int)info.st_size;
Err:
    return -1;
}

void *stream_port_fopen(char *name, char *op)
{
    if(name == NULL || op == NULL || op[0] != 'r')
        ERR_OUT("name = %p, op = %p\n", name, op);

    FILE *fp;
    fp = fopen(name, "rb");
    if(fp == NULL)
        ERR_OUT("fopen\n");

    return fp;
Err:
    return NULL;
}

int stream_port_fread(void *file, char *buf, int len)
{
#ifdef HUAWEI_C10
    char *buffer = (char *)file;

    if(buffer == NULL || buf == NULL || len <= 0)
        ERR_OUT("buffer = %p, buf = %p, len = %d\n", buffer, buf, len);
#ifdef	PLAY_BGMUSIC
//	PRINTF("stream_port_fread = len =%d=bufflen =%d\n",len,g_buffer_len);
    if(g_buffer_point + len >= g_buffer_len)
        len = g_buffer_len - g_buffer_point;
    if(len <= 0)
        return 0;
    if(g_buffer)
        IND_MEMCPY(buf, g_buffer + g_buffer_point, len);//
    g_buffer_point += len;
#endif
    if(len < 0)
        ERR_OUT("fread\n");
    if(len == 0)
        return 0;

    return len;

#elif defined (HUAWEI_C20)
    int ret;
    FILE *fp = (FILE *)file;

    if(fp == NULL || buf == NULL || len <= 0)
        ERR_OUT("fp = %p, buf = %p, len = %d\n", fp, buf, len);

    ret = fread(buf, 1, len, fp);
    if(ret < 0)
        ERR_OUT("fread\n");
    if(len == 0) {
        if(fseek(fp, 0, SEEK_SET))
            ERR_OUT("fseek\n");
        ret = fread(buf, 1, len, fp);
    }

    return ret;
#endif
Err:
    return -1;
}

int stream_port_fseek(void *file, int offset)
{
#ifdef HUAWEI_C10
    char *buffer = (char *)file;

    if(buffer == NULL || offset != 0)
        ERR_OUT("buffer = %p, offset = %d\n", buffer, offset);
#ifdef	PLAY_BGMUSIC
    g_buffer_point = 0;
#endif

#elif defined (HUAWEI_C20)
    FILE *fp = (FILE *)file;

    if(fp == NULL || offset != 0)
        ERR_OUT("fp = %p, offset = %d\n", fp, offset);

    if(fseek(fp, 0, SEEK_SET))
        ERR_OUT("fseek\n");
#endif
    return 0;
Err:
    return -1;
}

void stream_port_fclose(void *file)
{
    FILE *fp = (FILE *)file;

    if(fp == NULL)
        ERR_OUT("fp is NULL\n");

    fclose(fp);

Err:
    return;
}

