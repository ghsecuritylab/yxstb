

#include "KeyboardLinux.h"
#include "TAKIN_setting_type.h"
#include "TAKIN_interface.h"
#include "TAKIN_mid_porting.h"
#include "BrowserAgent.h"

#include <stdlib.h>


namespace Hippo {

Keyboard::Delegate* Keyboard::Delegate::Create()
{
    return new KeyboardLinux();
}

KeyboardLinux::KeyboardLinux()
{
}

KeyboardLinux::~KeyboardLinux()
{
}

bool KeyboardLinux::Open(Rect&)
{
    int support_chinese = 0;
#ifdef HUAWEI_C10
    support_chinese = 1;
#endif
    int vkeyboard_handle = TAKIN_browser_getKeyboardHandle();
    return (Takin_browser_InputMethod(&vkeyboard_handle, 0, support_chinese) == 0);
}

void KeyboardLinux::Close()
{
    int handler = TAKIN_browser_getKeyboardHandle();
    if (handler)
        TAKIN_browser_closeKeyboard(handler);
}

bool KeyboardLinux::HandleKey(void * event)
{
    int handler = TAKIN_browser_getKeyboardHandle();
    if (handler == 0)
        return false;

    YX_INPUTEVENT* e = static_cast<YX_INPUTEVENT*>(event);

    if (e->eventkind == YX_EVENT_KEYPRESS || e->eventkind == YX_EVENT_KEYUP) {
        return true;
    }
    if (e->vkey == 0 && e->unicode == 0) {
        return true;
    }

    switch (e->vkey) {
    case VK_UP:
    case VK_DOWN:
    case VK_LEFT:
    case VK_RIGHT:
    case VK_PRIOR:
    case VK_RETURN:
    case VK_BACK:
    case VK_0:
    case VK_1:
    case VK_2:
    case VK_3:
    case VK_4:
    case VK_5:
    case VK_6:
    case VK_7:
    case VK_8:
    case VK_9:
        return (TAKIN_browser_hitKeyboard(handler, e) != 0);
    default:
        break;
    }

    return false;
}


} // namespace Hippo





