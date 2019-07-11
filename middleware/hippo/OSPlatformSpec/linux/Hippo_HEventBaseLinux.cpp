#include "Hippo_OS.h"
#include "Hippo_HString.h"
//#include "base/Hippo_HEventBase.h"

namespace Hippo {

HEventBase::HEventBase( )
{
	m_eProperty = EventProperty_eNormal;
}

HEventBase::~HEventBase( )
{
	//destory sync lock.
	if( EventProperty_eSync == m_eProperty ){
	}
}

}

