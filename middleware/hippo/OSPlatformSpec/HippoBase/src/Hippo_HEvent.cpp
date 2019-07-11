#include "Hippo_HString.h"
#include "base/Hippo_HEvent.h"

namespace Hippo {

HEventDispatcher::event_status_e HEvent::handleBySelf( )
{
    if( m_target )
        return m_target->HandleEvent( *this );

    return HEventDispatcher::EventStatus_eContinue;
}

HActiveObject::HActiveObject( HActiveObject* parent ):
	parent_(parent), m_client(NULL)
{
}

HEventDispatcher::event_status_e HActiveObject::HandleEvent( HEvent& event )
{
	return HEventDispatcher::EventStatus_eContinue;
}

}

