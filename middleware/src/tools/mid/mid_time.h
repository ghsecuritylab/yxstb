
#ifndef __MID_OS_TIME_H_2007_3_25__
#define __MID_OS_TIME_H_2007_3_25__


#ifdef __cplusplus
extern "C" {
#endif

typedef long long mid_msec_t;

mid_msec_t mid_clock(void);
unsigned int mid_time(void);

unsigned int mid_10ms(void);
unsigned int mid_ms(void);
void mid_set_timezone(unsigned int sec);
unsigned int mid_get_times_sec();
unsigned int mid_tvms_time(void);

int set_local_time_zone(void);
int set_saving_time_sec(void);
int get_local_time_zone(void);
int get_saving_time_sec(void);

/* 1 means support utc time, 0 means do not support */
#ifndef MID_UTC_SUPPORT
#ifdef VIETTEL_HD
#define MID_UTC_SUPPORT 0
#else
#define MID_UTC_SUPPORT 1
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif /*__MID_OS_TIME_H_2007_3_25__*/
