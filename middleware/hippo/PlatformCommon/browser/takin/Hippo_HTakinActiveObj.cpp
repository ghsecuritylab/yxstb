#include "Hippo_HTakinActiveObj.h"
#include "base/Hippo_HEventDispatcher.h"

namespace Hippo{

HTakinActiveObj::HTakinActiveObj( )
{
}
HTakinActiveObj::~HTakinActiveObj( )
{
}

void HTakinActiveObj::InitActiveObject( )
{
	//TODO: Init Browser like iPanel3.0/Takin here.
}
HEventDispatcher::event_status_e HTakinActiveObj::HandleEvent( HEvent& aEvent )
{
	if( m_dispatcher )
		return Base::HandleEvent( aEvent );

	//TODO: send to Browser.
	
	return HEventDispatcher::EventStatus_eContinue;
}

}
