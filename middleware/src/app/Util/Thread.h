#ifndef _THREAD_H_
#define _THREAD_H_

#include "Runnable.h"
#include "MessageQueue.h"

#include <pthread.h>

#ifdef __cplusplus

namespace Hippo {

class Thread : public Runnable {
public:
    Thread();
    Thread(Runnable *r);
    Thread(pthread_t id);
    ~Thread();

    //void setName(String *name);
    void start();

    virtual void run();

    MessageQueue *getMessageQueue() { return &mMsgQueue; }

    static Thread *currentThread();

private:
    //String *nName;
    Runnable *mRunnable;
    pthread_t mID;
    MessageQueue mMsgQueue;

    static void *threadEntry(void *r);

};

} /* namespace Hippo */

#endif // __cplusplus

#endif /* _THREAD_H_ */
