#ifndef _Synchronized_H_
#define _Synchronized_H_

#include <stdint.h>
#include <pthread.h>

#ifdef __cplusplus

namespace Hippo {

class Synchronized {
public:
    Synchronized(void *sync);
    ~Synchronized();

    static int wait(void *sync);
    static int wait(void *sync, uint32_t when);
    static void notify(void *sync);
private:
    static pthread_mutex_t mSyncMutex;
    void *mSync;
};

} // namespace Hippo

#endif // __cplusplus

#endif /* _Synchronized_H_ */
