#ifndef __MID_NTP_H__
#define __MID_NTP_H__

#ifdef  __cplusplus
extern "C" {
#endif

/* If sync time failed by NTP or DVBS, set this value to system time. */
#define DEFAULT_SYSTEM_TIME "20120101000100"

int mid_ntp_init( void);

int mid_ntp_status(void);

void mid_ntp_time_sync(void);
void mid_ntp_time_contrl(int flags);

int mid_ntp_status(void);
void tv_tvms_setstate(int state);

int get_ntp_error(void);

int set_ntp_error(int errorstate);

unsigned int mid_ntp_firstSyncTimeGet( void );
#ifdef Chongqing
int set_nowtime_for_reboot(int sec );
#endif


#endif


#ifndef __SNTPC_H__
#define __SNTPC_H__

enum {
	NTP_MSG_ERROR = -1,
	NTP_MSG_OK,
	NTP_MSG_TIMEOUT
};

typedef void (*mid_ntp_f)(int arg, int ntpmsg, struct timespec *pts);

//timeout 以秒为单位超时时间
int mid_ntp_gettime(char* addr, mid_ntp_f callback, int arg, unsigned int timeout);

#ifdef  __cplusplus
}
#endif


#endif//__SNTPC_H__
