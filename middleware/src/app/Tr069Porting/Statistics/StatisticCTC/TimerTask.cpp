
#include "TimerTask.h"

#include "TR069Assertions.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>

void TimerTask::timerInsert(struct Timer* timer)
{
    struct Timer *tmr, *prev;

    prev = NULL;
    tmr = m_timer;

    while (tmr && tmr->clock <= timer->clock) {
        prev = tmr;
        tmr = tmr->next;
    }

    timer->next = tmr;
    if (prev)
        prev->next = timer;
    else
       m_timer = timer;
}

struct TimerTask::Timer* TimerTask::timerRemove(OnTimer onTimer, int arg)
{
    struct Timer *tmr, *prev;

    prev = NULL;
    tmr = m_timer;

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
            m_timer = tmr->next;
    }

    return tmr;
}

unsigned int TimerTask::taskClock(void)
{
    unsigned int clk;
    struct timespec tp;

    clock_gettime(CLOCK_MONOTONIC, &tp);

    clk = (unsigned int)tp.tv_sec * 100 + (unsigned int)tp.tv_nsec / 10000000;

    return clk;
}

unsigned int TimerTask::taskSecond(void)
{
    unsigned int sec;
    struct timespec tp;

    clock_gettime(CLOCK_MONOTONIC, &tp);

    sec = (unsigned int)tp.tv_sec;

    return sec;
}

void TimerTask::startThread(void)
{
    unsigned int    clock, diff;
    char            buf[4];
    int             fd, arg;

    fd_set          rset;
    struct timeval  tv;
 
    struct Timer     *timer;

    OnTimer onTimer;

    fd = m_fds[0];
    while (1) {
        clock = taskClock( );
        onTimer = NULL;
        arg = 0;
        diff = 360000;
        pthread_mutex_lock(&m_mutex);
        timer = m_timer;
        if (timer) {
            if (timer->clock <= clock) {
                m_timer = timer->next;
                timer->next = NULL;

                onTimer = timer->onTimer;
                arg = timer->arg;

                timer->clock += timer->interval;
                timerInsert(timer);
            } else {
                diff = timer->clock - clock;
            }
        }
        pthread_mutex_unlock(&m_mutex);
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
}

void* TimerTask::startRoutine(void *arg)
{
    TimerTask *timerTask = (TimerTask *)arg;
    timerTask->startThread( );

    return NULL;
}

TimerTask::TimerTask(void) : m_timer(NULL)
{
    pipe(m_fds);

    pthread_mutex_init(&m_mutex, NULL);
    pthread_create(&m_thread, NULL, startRoutine, this);
}

TimerTask::~TimerTask(void)
{
}

void TimerTask::timerTaskRegist(unsigned int sec, OnTimer onTimer, int arg)
{
    struct Timer *timer;

    if (sec <= 0 || sec > 3600*24) {
        printf("%s:%d sec = %d\n", __FILE__, __LINE__, sec);
        LogTr069Error("sec = %d\n", sec);
        return;
    }

    pthread_mutex_lock(&m_mutex);
    timer = timerRemove(onTimer, arg);
    if (!timer) {
        timer = (Timer *)calloc(sizeof(Timer), 1);
        timer->onTimer = onTimer;
        timer->arg = arg;
    }
    timer->interval = sec * 100;
    timer->clock = taskClock( ) + timer->interval;

    timerInsert(timer);
    pthread_mutex_unlock(&m_mutex);

    {
        char buf[4];
        write(m_fds[1], buf, 1);
    }
}

void TimerTask::timerTaskUnRegist(OnTimer onTimer, int arg)
{
    struct Timer *timer;

    pthread_mutex_lock(&m_mutex);
    timer = timerRemove(onTimer, arg);
    if (timer)
        free(timer);
    pthread_mutex_unlock(&m_mutex);

    {
        char buf[4];
        write(m_fds[1], buf, 1);
    }
}

