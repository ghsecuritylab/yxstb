
#include "Synchronized.h"

#include <map>


namespace Hippo {

struct SyncChannel {
    pthread_cond_t mCond;
    pthread_mutex_t mMutex;
    int mCount;

    SyncChannel();
    ~SyncChannel();
};

SyncChannel::SyncChannel()
{
    pthread_cond_init(&mCond, NULL);
    pthread_mutex_init(&mMutex, NULL);
    mCount = 1;
}

SyncChannel::~SyncChannel()
{
    pthread_mutex_destroy(&mMutex);
    pthread_cond_destroy(&mCond);
}


static std::map<void *, SyncChannel *> gSyncChannels;

pthread_mutex_t Synchronized::mSyncMutex = PTHREAD_MUTEX_INITIALIZER;


Synchronized::Synchronized(void *sync)
	: mSync(sync)
{
    SyncChannel *channel;

    pthread_mutex_lock(&mSyncMutex);
    std::map<void *, SyncChannel *>::iterator it;
    it = gSyncChannels.find(sync);
    if (it == gSyncChannels.end()) {
        channel = new SyncChannel();
        gSyncChannels.insert(std::make_pair(sync, channel));
    }
    else {
        channel = it->second;
        channel->mCount++;
    }
    pthread_mutex_unlock(&mSyncMutex);

    pthread_mutex_lock(&channel->mMutex);
}

Synchronized::~Synchronized()
{
    SyncChannel *channel;

    pthread_mutex_lock(&mSyncMutex);
    std::map<void *, SyncChannel *>::iterator it;
    it = gSyncChannels.find(mSync);
    if (it == gSyncChannels.end()) {
        ;
    }
    else {
        channel = it->second;
        channel->mCount--;
        pthread_mutex_unlock(&channel->mMutex);
        if (channel->mCount <= 0) {
            gSyncChannels.erase(it);
            delete channel;
        }
    }
    pthread_mutex_unlock(&mSyncMutex);
}

int Synchronized::wait(void *sync)
{
    SyncChannel *channel;

    pthread_mutex_lock(&mSyncMutex);
    std::map<void *, SyncChannel *>::iterator it;
    it = gSyncChannels.find(sync);
    if (it == gSyncChannels.end()) {
        pthread_mutex_unlock(&mSyncMutex);
        return -1;
    }
    else {
        channel = it->second;
    }
    pthread_mutex_unlock(&mSyncMutex);

    pthread_cond_wait(&channel->mCond, &channel->mMutex);
    return 0;
}

int Synchronized::wait(void *sync, uint32_t when)
{
    SyncChannel *channel;
    struct timespec ts;

    pthread_mutex_lock(&mSyncMutex);
    std::map<void *, SyncChannel *>::iterator it;
    it = gSyncChannels.find(sync);
    if (it == gSyncChannels.end()) {
        pthread_mutex_unlock(&mSyncMutex);
        return -1;
    }
    else {
        channel = it->second;
    }
    pthread_mutex_unlock(&mSyncMutex);

    ts.tv_sec = when / 1000;
    ts.tv_nsec = (when % 1000) * 1000 * 1000;
    pthread_cond_timedwait(&channel->mCond, &channel->mMutex, &ts);
    return 0;
}

void Synchronized::notify(void *sync)
{
    SyncChannel *channel = 0;

    pthread_mutex_lock(&mSyncMutex);
    std::map<void *, SyncChannel *>::iterator it;
    it = gSyncChannels.find(sync);
    if (it == gSyncChannels.end()) {
        ;
    }
    else {
        channel = it->second;
    }
    pthread_mutex_unlock(&mSyncMutex);

    if (channel) {
        pthread_mutex_lock(&channel->mMutex);
        pthread_cond_signal(&channel->mCond);
        pthread_mutex_unlock(&channel->mMutex);
    }
}

} /* namespace Hippo */
