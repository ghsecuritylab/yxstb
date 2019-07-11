
#ifndef TimerTask_h
#define TimerTask_h

#include <pthread.h>

class TimerTask {
private:
    pthread_t m_thread;
    pthread_mutex_t m_mutex;

    int m_fds[2];

    typedef void (*OnTimer)(int arg);

    struct Timer {
        struct Timer *next;
    
        OnTimer onTimer;
        int arg;
        unsigned int clock;
        unsigned int interval;
    };
    struct Timer* m_timer;

    void startThread(void);
    static void* startRoutine(void *arg);

    void timerInsert(struct Timer *timer);
    struct Timer* timerRemove(OnTimer onTimer, int arg);

public:
    TimerTask();
    ~TimerTask();

    void timerTaskRegist(unsigned int sec, OnTimer onTimer, int arg);
    void timerTaskUnRegist(OnTimer onTimer, int arg);

    unsigned int taskClock(void);
    unsigned int taskSecond(void);
};

#endif//TimerTask_h