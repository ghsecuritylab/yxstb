
#include "ProgramChannelCustomer.h"


namespace Hippo {

ProgramChannelCustomer::ProgramChannelCustomer()
{
}

ProgramChannelCustomer::~ProgramChannelCustomer()
{
}

void ProgramChannelCustomer::SetChanType(int iarg)
{
	switch(iarg){
		case 1:	m_eChanType = PCT_VEDIO;	break;
		case 2:	m_eChanType = PCT_WEBPAGE;	break;
		case 3:	m_eChanType = PCT_AUDIO;	break;
		default: m_eChanType = PCT_VEDIO;	break;		
	}
	return;
}

} // namespace Hippo
