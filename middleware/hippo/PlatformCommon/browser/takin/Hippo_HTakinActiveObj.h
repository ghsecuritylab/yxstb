#ifndef __HIPPO_HTakinActiveObj_H
#define __HIPPO_HTakinActiveObj_H

#include "Hippo_HBrowserActiveObj.h"

namespace Hippo{

class HTakinActiveObj : public HBrowserActiveObj{
	typedef HBrowserActiveObj Base;
	public:		
		HTakinActiveObj( );
		~HTakinActiveObj( );

		virtual void InitActiveObject( );		
		virtual HEventDispatcher::event_status_e HandleEvent( HEvent& aEvent );
    private:
};

}

#endif

