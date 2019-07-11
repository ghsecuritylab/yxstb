
#include "tmerTask.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/select.h>


typedef struct tagTimer Timer;
struct tagTimer {
    Timer *next;

    OnTimer onTimer;
    int arg;
    unsigned int clock;
    unsigned int interval;
};

struct tagTimerTask {
    pthread_t thread;
    pthread_mutex_t mutex;

    int fds[2];
    Timer *timer;
};

void timerInsert(TimerTask *task, Timer *timer)
{
    Timer *tmr, *prev;

    prev = NULL;
    tmr = task->timer;

    while (tmr && tmr->clock <= timer->clock) {
        prev = tmr;
        tmr = tmr->next;
    }

    timer->next = tmr;
    if (prev)
        prev->next = timer;
    else
        task->timer = timer;
}

Timer *timerRemove(TimerTask *task, OnTimer onTimer, int arg)
{
    Timer *tmr, *prev;

    prev = NULL;
    tmr = task->timer;

    while (tmr) {
        if (onTimer == tmr->onTimer && arg == tmr->arg)
            break;
        prev = tmr;
        tmr = tmr->next;
    }
    if (tmr) {
        if (prev)
            prev->next = tmr->next;
        else
            task->timer = tmr->next;
    }

    return tmr;
}

unsigned int taskClock(void)
{
    unsigned int clk;
    struct timespec tp;

    clock_gettime(CLOCK_MONOTONIC, &tp);

    clk = (unsigned int)tp.tv_sec * 100 + (unsigned int)tp.tv_nsec / 10000000;

    return clk;
}

static void *taskLoop(void *taskArg)
{
    unsigned int    clock, diff;
    char            buf[4];
    int             fd, arg;

    fd_set          rset;
    struct timeval  tv;
 
    TimerTask      *task;
    Timer          *timer;

    OnTimer onTimer;

    task = (TimerTask*)taskArg;
    fd = task->fds[0];
    while (1) {
        clock = taskClock( );
        onTimer = NULL;
        arg = 0;
        diff = 360000;
        pthread_mutex_lock(&task->mutex);
        timer = task->timer;
        if (timer) {
            if (timer->clock <= clock) {
                task->timer = timer->next;
                timer->next = NULL;

                onTimer = timer->onTimer;
                arg = timer->arg;

                timer->clock += timer->interval;
                timerInsert(task, timer);
            } else {
                diff = timer->clock - clock;
            }
        }
        pthread_mutex_unlock(&task->mutex);
        if (onTimer) {
            onTimer(arg);
            continue;
        }

        tv.tv_sec = diff / 100;
        tv.tv_usec = diff % 100 * 10000;

        FD_ZERO(&rset);
        FD_SET(fd, &rset);

        if (select(fd + 1, &rset , NULL,  NULL, &tv) <= 0)
            continue;

        if (FD_ISSET(fd, &rset)) {
            read(fd, buf, 1);
            continue;
        }
    }

    return NULL;
}

TimerTask *timerTaskCreate(void)
{
    TimerTask *task;

    task = (TimerTask*)calloc(sizeof(TimerTask), 1);
    if (!task)
        return NULL;

    pipe(task->fds);

    pthread_mutex_init(&task->mutex, NULL);
    pthread_create(&task->thread, NULL, taskLoop, task);

    return task;
}

void timerTaskRegist(TimerTask *task, unsigned int sec, OnTimer onTimer, int arg)
{
    Timer *timer;

    if (sec <= 0 || sec > 3600*24)
        return;

    pthread_mutex_lock(&task->mutex);
    timer = timerRemove(task, onTimer, arg);
    if (!timer) {
        timer = (Timer *)calloc(sizeof(Timer), 1);
        timer->onTimer = onTimer;
        timer->arg = arg;
        timer->interval = sec * 100;
        timer->clock = taskClock( ) + timer->interval;
    }
    timerInsert(task, timer);
    pthread_mutex_unlock(&task->mutex);

    {
        char buf[4];
        write(task->fds[1], buf, 1);
    }
}

void timerTaskUnRegist(TimerTask *task, OnTimer onTimer, int arg)
{
    Timer *timer;

    pthread_mutex_lock(&task->mutex);
    timer = timerRemove(task, onTimer, arg);
    if (timer)
        free(timer);
    pthread_mutex_unlock(&task->mutex);

    {
        char buf[4];
        write(task->fds[1], buf, 1);
    }
}
