#include "Hippo_Debug.h"
#include "Hippo_Context.h"
#include "Hippo_HBrowserActiveObj.h"
#include "base/Hippo_HEventDispatcher.h"

#include "takin/Hippo_HTakinActiveObj.h"


namespace Hippo{

HBrowserActiveObj* HBrowserActiveObj::createBrowserObj( )
{
	HTakinActiveObj* p;
    p = new HTakinActiveObj( );

	if( !p ){
		HIPPO_DEBUG( "create Browser error.\n" );
	}
	return static_cast<HBrowserActiveObj*>( p );
}
HBrowserActiveObj* HBrowserActiveObj::createWidget( )
{
	HBrowserActiveObj* p = HBrowserActiveObj::createBrowserObj( );
	HBrowserActiveObj* mainObj = HippoContext::getContextInstance( )->getBrowserActiveObj( );

	if( this != mainObj ){
		HIPPO_DEBUG( "why not main BrowserActiveObj????\n" );
	}
	p->InitWidget( );
	return static_cast<HBrowserActiveObj*>( p );
}
HBrowserActiveObj::HBrowserActiveObj( )
    :m_dispatcher( 0 ), mType(ActiveOjbType_eUnknown)
{
}
HBrowserActiveObj::~HBrowserActiveObj( )
{
}

void HBrowserActiveObj::InitWidget( )
{
}
void HBrowserActiveObj::InitBrowser( )
{

}
void HBrowserActiveObj::setCurrentActiveObj( const HBrowserActiveObj* aObj )
{
}
void HBrowserActiveObj::setDefaultActiveObj( const HBrowserActiveObj* aObj )
{
}

HActiveObject* HBrowserActiveObj::getCurrentActiveObj( )
{
	return static_cast<HActiveObject*>(this);
}
void HBrowserActiveObj::releaseCurrentActiveObj( )
{
    if( !getCurrentActiveObj( ) )
        setCurrentActiveObj( HippoContext::getContextInstance( )->getBrowserActiveObj( ) );
}
HEventDispatcher::event_status_e HBrowserActiveObj::HandleEvent( HEvent& aEvent )
{
	HEventDispatcher::event_status_e eStatus;

	return HEventDispatcher::EventStatus_eContinue;
}

}

