
#ifndef _KEYBOARDANDROID_H_
#define _KEYBOARDANDROID_H_
#pragma once

#include "Keyboard.h"

namespace Hippo {
class KeyboardAndroid : public Keyboard::Delegate {
public:
    KeyboardAndroid();
    virtual ~KeyboardAndroid();
    virtual bool Open(Rect& r);
    virtual void Close();
    virtual bool HandleKey(void* event);
};

} // namespace Hippo;

#endif

