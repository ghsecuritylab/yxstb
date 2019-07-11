
#include "ITC.h"

#include "Synchronized.h"


namespace Hippo {

static Result gSyncChannel;

ITC::ITC(Result *syncChannel)
	: mSyncChannel(syncChannel)
{
    if (mSyncChannel == NULL)
        mSyncChannel = &gSyncChannel;
}

ITC::~ITC()
{
}

int 
ITC::call(int what)
{
    Message *msg = obtainMessage(what);
    {
        Synchronized tSync(mSyncChannel);
        sendMessage(msg);
        tSync.wait(mSyncChannel);
    }
    return -1;
}

int 
ITC::call(int what, Object *obj)
{
    Message *msg = obtainMessage(what, obj);
    {
        Synchronized tSync(mSyncChannel);
        sendMessage(msg);
        tSync.wait(mSyncChannel);
    }
    return -1;
}

int 
ITC::call(int what, int arg1, int arg2)
{
    Message *msg = obtainMessage(what, arg1, arg2);
    {
        Synchronized tSync(mSyncChannel);
        sendMessage(msg);
        tSync.wait(mSyncChannel);
    }
    return -1;
}

int 
ITC::call(int what, int arg1, int arg2, Object *obj)
{
    Message *msg = obtainMessage(what, arg1, arg2, obj);
    {
        Synchronized tSync(mSyncChannel);
        sendMessage(msg);
        tSync.wait(mSyncChannel);
    }
    return -1;
}

void 
ITC::handleMessage(Message *msg)
{
}

void 
ITC::wakeUp()
{
    Synchronized::notify(mSyncChannel);
}

} // namespace Hippo
