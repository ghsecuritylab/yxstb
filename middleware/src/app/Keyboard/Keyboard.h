

#ifndef _IPTVMIDDLEWARE_KEYBOARD_H_
#define _IPTVMIDDLEWARE_KEYBOARD_H_
#pragma once

#include "Rect.h"

namespace Hippo {

class Keyboard {
public:
    ~Keyboard();

    static Keyboard* GetInstance();

    // true: ok
    // false: shouldn't open or open failed.
    bool Open(Rect& r);

    void Close();

    bool HandleKey(void* event);
    

    // 存储自己的数据结构
    class Delegate {
    public:
        virtual bool Open(Rect& r) = 0;
        virtual void Close() = 0;
        virtual bool HandleKey(void* event) = 0;

    protected:
        friend class Keyboard;
        static Delegate* Create();

        Delegate() {}
        virtual ~Delegate() {}
    };

private:
    Keyboard();

    Delegate * m_implement;
};


} // namespace Hippo
#endif

