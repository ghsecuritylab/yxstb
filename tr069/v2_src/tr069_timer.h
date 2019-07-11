/*******************************************************************************
	��˾��
			Yuxing software
	��¼��
			2008-1-26 21:12:26 create by Liu Jianhua
	ģ�飺
			tr069
	������
			tr069 ��ʱ��
 *******************************************************************************/

#ifndef __TR069_TIMER_H__
#define __TR069_TIMER_H__

typedef void (*Ontimer)(struct TR069 *tr069);
void tr069_timer_deal(struct TR069 *tr069, unsigned int current);
int tr069_timer_time(const struct TR069 *tr069);
int tr069_timer_create(struct TR069 *tr069, unsigned int datetime, Ontimer ontimer);
void tr069_timer_delete(struct TR069 *tr069, Ontimer ontimer);
void tr069_timer_init(struct TR069 *tr069);

#endif //__TR069_TIMER_H__
