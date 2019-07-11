
#ifndef _KEYBOARDLINUX_H_
#define _KEYBOARDLINUX_H_
#pragma once

#include "Keyboard.h"

namespace Hippo {
class KeyboardLinux : public Keyboard::Delegate {
public:
    KeyboardLinux();
    virtual ~KeyboardLinux();
    virtual bool Open(Rect& r);
    virtual void Close();
    virtual bool HandleKey(void* event);
};

} // namespace Hippo;

#endif

