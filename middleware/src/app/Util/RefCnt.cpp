
#include "RefCnt.h"


namespace Hippo {

AutoUnref::~AutoUnref()
{
    if (mObj) {
        mObj->unref();
    }
}

bool 
AutoUnref::ref()
{
    if (mObj) {
        mObj->ref();
        return true;
    }
    return false;
}

bool 
AutoUnref::unref()
{
    if (mObj) {
        mObj->unref();
        mObj = NULL;
        return true;
    }
    return false;
}

RefCnt* 
AutoUnref::detach()
{
    RefCnt* obj = mObj;
    mObj = NULL;
    return obj;
}

} // namespace Hippo
