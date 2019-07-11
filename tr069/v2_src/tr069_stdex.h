/*******************************************************************************
	公司：
			Yuxing software
	纪录：
			2008-1-26 21:12:26 create by Liu Jianhua
	模块：
			tr069
	简述：
			标准C语言函数扩展
 *******************************************************************************/

#ifndef __TR069_STDEX_H__
#define __TR069_STDEX_H__

u_int tr069_atoui(const char *str);
int tr069_strdup(char **pp, const char *str);
void tr069_strncpy(const char *src, char *dst, int len);
void tr069_linencpy(const char *src, char *dst, int len);
int tr069_isblank(char ch);
int tr069_str2time(const char* str, u_int *pVal);
int tr069_time2str(uint32_t sec, char* buf);
int tr069_str2int(const char *strval, int *pVal);
int tr069_str2uint(const char *strval, u_int *pVal);

int tr069_checkurl(char* url, char *uri, int uri_len, char *host, int host_len, unsigned short *port);

int base64_encode(const u_char *inbuf, u_int inlen, u_char *outbuf, u_int outsize);
int base64_decode(const u_char *inbuf, u_int inlen, u_char *outbuf, u_int outsize);

unsigned int tr069_ipAddrDot2Int(char *src);
int tr069_ipAddrInt2Dot(unsigned int src, char *dst, int dstLen);

unsigned int tr069_10ms(void);
unsigned int tr069_msec(void);
unsigned int tr069_sec(void);

void tr069_logPrefix(const char *file, const int line, const char *func);

struct TR069Msgq;
typedef struct TR069Msgq* TR069Msgq_t;

TR069Msgq_t tr069_msgq_create(int msg_num, int msg_size);
void tr069_msgq_delete(TR069Msgq_t msgq);
int tr069_msgq_fd(TR069Msgq_t msgq);
int tr069_msgq_putmsg(TR069Msgq_t msgq, char *msg);
int tr069_msgq_getmsg(TR069Msgq_t msgq, char *msg);

unsigned int tr069_get_TotalBytesSent(void);
unsigned int tr069_get_TotalBytesReceived(void);
unsigned int tr069_get_TotalPacketsSent(void);
unsigned int tr069_get_TotalPacketsReceived(void);

ssize_t tr069_sendto(int socket, const void *message, size_t length, int flags, const struct sockaddr_in *address);
int tr069_connect(int socket, const struct sockaddr_in *address);

#endif //__TR069_STDEX_H__
