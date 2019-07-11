
#ifndef __IND_TIMER_H__
#define __IND_TIMER_H__

#ifdef __cplusplus
extern "C" {
#endif

struct ind_time
{
  int sec;			/* Seconds.	[0-60] (1 leap second) */
  int min;			/* Minutes.	[0-59] */
  int hour;			/* Hours.	[0-23] */
  int day;			/* Day.		[1-31] */
  int mon;			/* Month.	[0-11] */
  int year;			/* Year. */
  int week;			/* Week.	[0-6] */
};

typedef struct ind_tlink* ind_tlink_t;
typedef void (*Ontimer)(void *arg);

/*
	��ʱ��������
 */
ind_tlink_t ind_tlink_create(int num);
void ind_tlink_delete(ind_tlink_t tlink);

unsigned int ind_timer_clock(ind_tlink_t tlink);
void ind_timer_deal(ind_tlink_t tlink, unsigned int clock);
void ind_timer_print(ind_tlink_t tlink);

/*
	clock ��ʱ��ִ��ʱ��
	interval 0����ʱ��ִֻ��һ�Σ�> 0����ʱ���ظ�ִ��ʱ��ʱ����
 */
int ind_timer_create(ind_tlink_t tlink, unsigned int clock, int interval, Ontimer ontimer, void *arg);
void ind_timer_delete(ind_tlink_t tlink, Ontimer ontimer, void *arg);
void ind_timer_delete_all(ind_tlink_t tlink);



unsigned int ind_time_make(struct ind_time *tp);
int ind_time_local (unsigned int t, struct ind_time *tp);


#ifdef __cplusplus
}
#endif

#endif //__IND_TIMER_H__
