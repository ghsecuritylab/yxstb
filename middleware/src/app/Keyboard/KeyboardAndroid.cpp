

#include "KeyboardAndroid.h"
#include "IPTVMiddleware.h"

#include <stdlib.h>

#ifdef EC6106V8_TEST
extern void AndroidSurface_showIme(int top);
extern void AndroidSurface_hideIme();
#endif

namespace Hippo {

Keyboard::Delegate* Keyboard::Delegate::Create()
{
    return new KeyboardAndroid();
}

KeyboardAndroid::KeyboardAndroid()
{
}

KeyboardAndroid::~KeyboardAndroid()
{
}

bool KeyboardAndroid::Open(Rect& r)
{
#ifdef EC6106V8_TEST //TODO:此宏目前只有6V8的才有
    AndroidSurface_showIme(static_cast<int>(r.fTop));
#else
    IPTVMiddleware_PostEvent(15, 1, static_cast<int>(r.fTop), 0);
#endif
    return true;
}

void KeyboardAndroid::Close()
{
 #ifdef EC6106V8_TEST
     AndroidSurface_hideIme();
 #else
    IPTVMiddleware_PostEvent(15, 0, 0, 0);
 #endif
}

bool KeyboardAndroid::HandleKey(void * event)
{
    return false;
}


} // namespace Hippo





