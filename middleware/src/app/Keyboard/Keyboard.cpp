

#include "Keyboard.h"
#include "TAKIN_setting_type.h"
#include "TAKIN_interface.h"
#include "TAKIN_mid_porting.h"

namespace Hippo {

static Keyboard* g_instance = NULL;

Keyboard::Keyboard()
    : m_implement(Delegate::Create())
{
}

Keyboard::~Keyboard()
{
    if (m_implement)
        delete m_implement;
}

Keyboard* Keyboard::GetInstance()
{
    if (!g_instance)
        g_instance = new Keyboard();
    return g_instance;
}

bool Keyboard::Open(Rect& r)
{
    if (m_implement)
        return m_implement->Open(r);
    return false;
}

void Keyboard::Close()
{
    if (m_implement)
        m_implement->Close();
}

bool Keyboard::HandleKey(void* event)
{
    if (m_implement)
        return m_implement->HandleKey(event);
    return false;
}

};










