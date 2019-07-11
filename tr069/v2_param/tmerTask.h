
#ifndef __timerTask_h__
#define __timerTask_h__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tagTimerTask TimerTask;

typedef void (*OnTimer)(int arg);

TimerTask *timerTaskCreate(void);
void timerTaskRegist(TimerTask *task, unsigned int sec, OnTimer onTimer, int arg);
void timerTaskUnRegist(TimerTask *task, OnTimer onTimer, int arg);

#ifdef __cplusplus
}
#endif

#endif//__timerTask_h__