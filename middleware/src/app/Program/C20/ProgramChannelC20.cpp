
#include "ProgramChannelC20.h"
#include <stdlib.h>

namespace Hippo {

ProgramChannelC20::ProgramChannelC20()
{
    m_HStringPIPMulticastIP.clear();
    m_HStringPIPMulticastPort.clear();
    m_HStringPIPMulticastSrcIP.clear();
	m_HStringID.clear();

    m_nChanStatus = 0;

    m_nLogoWidth = 0;
    m_nLogoHeight = 0;
    m_nLogoDisplay = 0;
    m_nLogoHide = 0;

    m_nHasPIP = 0;
    m_nChanBandwith = 0;
    m_nChanEncrypt = 0;

    m_nPrevLength = 0;
    m_nPrevCount = 0;
    m_nUpgrade = 0;
}

ProgramChannelC20::~ProgramChannelC20()
{
}

int
ProgramChannelC20::getNumberID()
{
	return atoi(m_HStringChanID.c_str());
}

const char *
ProgramChannelC20::getStringID()
{
	m_HStringID	+= "MULTIPLE";
	m_HStringID += m_nChanKey;

	return m_HStringID.c_str();

}

} // namespace Hippo
