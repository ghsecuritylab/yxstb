
#include "ProgramChannel.h"


namespace Hippo {

ProgramChannel::ProgramChannel()
{
    m_HStringChanID.clear();
    m_HStringChanName.clear();
    m_HStringChanURL.clear();
    m_HStringTimeShiftURL.clear();
    m_HStringChanLogURL.clear();
    m_HStringLogoURL.clear();

    m_eChanType = PCT_VEDIO;
    m_nUserChanID = 0;
    m_nTimeShift = 0;
#if defined(Jiangsu)
    m_nTimeShiftLength = -1;
#else
    m_nTimeShiftLength = 3600;
#endif
    m_nIsHDChan = 0;
    m_nPrevEnable = 0;
    m_nPrevNum = 0;

    m_nFCC_Enable = 0;
    m_nFEC_Enable = 0;
    m_nSQACode = 0;
    m_nRetCode = 0;
    m_nFCCServerValidTime = 0;

    m_nLogoXPos = 0;
    m_nLogoYPos = 0;

    /*******   not sure      *************/
    m_HStringMiniChanURL.clear();
    m_HStringMulticastSrcIP.clear();


    m_HStringPIP_URL.clear();
    m_HStringFCC_Server.clear();			//RRS-FCC
    m_HStringFCC_ServerPIP.clear();

    m_nChanKey = 0;

#ifdef INCLUDE_DVBS
    m_uTP_Freq = 0;
    m_uDVB_Symb = 0;

    m_nDVB_Polarity = 0;
    m_nDVB_ProgNum = 0;
    m_nDVB_PMT_PID = 0;
    m_nDVB_OrigNetID = 0;

    m_nDVB_SatLocation = 0;
	m_HStringDVB_SatName.clear();
#endif
    m_nChanDomain = 0;
    m_nHDCP_Enable = -1;
    m_nMacrovEnable = -1;
    m_nCGMSAEnable = -1;

	m_HStringChanCacheTime.clear();
    m_MediaCode.clear();
    m_EntryID.clear();
}

ProgramChannel::~ProgramChannel()
{
}

int
ProgramChannel::getNumberID()
{
    return GetUserChanID();
}

const char *
ProgramChannel::getStringID()
{
    return m_HStringChanID.c_str();
}

void ProgramChannel::SetChanType(int iarg)
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
