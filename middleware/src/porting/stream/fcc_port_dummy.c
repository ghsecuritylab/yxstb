#include "app_include.h"
#include "fcc_port.h"
#include "stream_port.h"

/*
	申请并打开fcc
 */
void *fcc_port_open(struct FCCArg* arg)
{
	return NULL;
}

/*
	关闭并释放fcc
 */
void fcc_port_close(void* handle)
{
	return;
}

/*
	rtsp主线程会100毫秒钟调用一次
 */
void fcc_port_100ms(void* handle)
{
}

/*
	RET
 */
void *ret_port_open(struct RETArg* arg)
{
	return NULL;
}

void ret_port_close(void* handle)
{
}

void ret_port_reset(void* handle)
{
}

void ret_port_push(void* handle, char *buf, int len)
{
}

void ret_port_cache(void* handle, int on)
{
}

int ret_port_pop(void* handle)
{
	return 0;
}

void ret_port_100ms(void* handle)
{
}

void *arq_port_open(struct RETArg* arg)
{
	return NULL;
}

void arq_port_close(void* handle)
{
}

void arq_port_reset(void* handle)
{
}

void arq_port_push(void* handle, char *buf, int len)
{
}

void arq_port_100ms(void* handle)
{
}

