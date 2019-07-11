/*******************************************************************************
	¹«Ë¾£º
			Yuxing software
	¼ÍÂ¼£º
			2008-1-26 21:12:26 create by Liu Jianhua
	Ä£¿é£º
			tr069
	¼òÊö£º
			ÍøÂçÕï¶Ï
 *******************************************************************************/

#ifndef __TR069_PING_H__
#define __TR069_PING_H__

#define DIAG_ERROR_NONE				0
#define DIAG_ERROR_RESOLVE			1
#define DIAG_ERROR_HOPEXCEEDED		2
#define DIAG_ERROR_INTERNAL			3
#define DIAG_ERROR_OTHER			4

#define DIAG_HOST_LEN				256
#define DIAG_HOST_NUM				64

struct Ping {
	char host[DIAG_HOST_LEN];
	u_int count;
	u_int timeout;
	u_int datalen;
	u_int dscp;
	u_int successcount;
	u_int failurecount;
	u_int averagetime;
	u_int minimumtime;
	u_int maximumtime;
};

struct RouteHop {
	char hophost[DIAG_HOST_LEN];
	int minimumResponseTime;
	int averageResponseTime;
	int maximumResponseTime;
};

struct Trace {
	char host[DIAG_HOST_LEN];
	u_int timeout;
	u_int datalen;
	u_int dscp;

	u_int hopmax;
	u_int resptime;
	u_int hopnum;

	struct RouteHop routehop[DIAG_HOST_NUM];
};

int tr069_diag_ping(struct Ping *ping);
int tr069_diag_trace(struct Trace *trace);

#endif //__TR069_PING_H__
