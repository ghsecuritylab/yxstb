
#ifndef __MID_DNS_H__
#define __MID_DNS_H__

//最大超时时间
#define MID_DNS_TIMEOUT		60

#ifdef __cplusplus
extern "C" {
#endif

enum {
	DNS_MSG_NOTFOUND = -3,//与dns服务器交互成功，但域名不存在
	DNS_MSG_TIMEOUT = -2,//与DNS服务器连接超时（服务器不可达）
	DNS_MSG_ERROR = -1,//其它错误
	DNS_MSG_OK,//成功
};

typedef void (*mid_dns_f)(int arg, int dnsmsg, unsigned int hostip);
/*
	DNS异步解析
	返回值：
			-1 函数调用失败
			0  异步解析
			其它值表示 已经解析出来过 直接返回IP地址
 */
int mid_dns_resolve(const char* hostname, mid_dns_f callback, int arg, int timeout);
/*
	举例：
	to_array[0] = 2;//
	to_array[1] = 4;//第二次与第三次间隔
	to_array[2] = 8;
	to_count = 3;//数值大小

	如果只有一个dns0，那么就只发3次解析
		dns0 间隔2秒 dns0 间隔4秒 dns0

	如果有两个dns，分别为dns0和dns1，那么会发送6次
		dns0 间隔2秒 dns1 间隔2秒 dns0 间隔4秒 dns1 间隔4秒 dns0 间隔8秒 dns1

 */
int mid_dns_resolve_to(const char* hostname, mid_dns_f callfunc, int callarg, int timeout, int* to_array, int to_count);

//DNS同步解析
int mid_dns_gethost(const char* hostname, unsigned int *hostip, int timeout);
int mid_dns_gethost_to(const char* hostname, unsigned int *hostip, int timeout, int* to_array, int to_count);

void mid_dns_clean(const char* hostname);
unsigned int mid_dns_cache(const char* hostname);

void mid_dns_setsrv(char* dns1, char* dns2);
void mid_dns_init(void);

#ifdef __cplusplus
}
#endif

#endif//__MID_DNS_H__
