
#include "ProgramChannelC10.h"


namespace Hippo {

ProgramChannelC10::ProgramChannelC10()
{
    m_HStringChanSDP.clear();

    m_nChanPurchased = 0;
    m_nChanLocked = 0;
    
    m_nBeginTime = 0;
    m_nInterval = 0;
    m_nLasting = 0;

    m_nActionType = 0;
    m_nChannelFECPort = 0;
    m_nCanUnicast = 0;
}

ProgramChannelC10::~ProgramChannelC10()
{
}

void ProgramChannelC10::SetChanType(int iarg)
{
	switch(iarg){
		case 1:	m_eChanType = PCT_VEDIO;	break;
		case 2:	m_eChanType = PCT_AUDIO;	break;
		case 3:	m_eChanType = PCT_WEBPAGE;	break;
		default: m_eChanType = PCT_VEDIO;	break;		
	}
	return;
}

} // namespace Hippo
