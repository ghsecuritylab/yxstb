
#include "ProgramChannelDvb.h"
#include "DvbUtils.h"


namespace Hippo {

ProgramChannelDvb::ProgramChannelDvb()
{
	_tunerID     = 1;
	_switch_22k  = 0;
	_diseq_port  = 0;
    _band_width  = 0;
	_scrambled   = false;
    SetChanDomain(DVBS_DOMAIN);
}

ProgramChannelDvb::~ProgramChannelDvb()
{
}

int  ProgramChannelDvb::getNumberID()
{
	return GetChanKey();
}

/*-----------------------------------------------------------------------------
 *  Be carefull when change the prefix "DVB", used in ManageProgramDvbs.cpp
 *-----------------------------------------------------------------------------*/
const char *ProgramChannelDvb::getStringID()
{
    std::string strID = "DVB" + UtilsInt2Str(GetChanKey());
	return strID.c_str();
}

} // namespace Hippo

