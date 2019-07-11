#ifndef _ProgramChannelDvb_H_
#define _ProgramChannelDvb_H_

#include "ProgramChannel.h"
#include "DvbEnum.h"
#include <iostream>

#include "libzebra.h"

#ifdef __cplusplus

namespace Hippo {

#define DVBS_DOMAIN 1

class ProgramChannelDvb : public ProgramChannel {
public:
    ProgramChannelDvb();
    ~ProgramChannelDvb();

    virtual int getNumberID() ;
    virtual const char *getStringID();


	void SetDVB_ServiceType(ServiceType_e ser_type ) { _serviceType = ser_type; }
    ServiceType_e GetDVB_ServiceType( void ) { return _serviceType; }

    void SetDVB_Modulation(Modulation_e modulation ) { _modulation = modulation; }
    Modulation_e GetDVB_Modulation( void ) { return _modulation; }

    void SetDVB_TpID( int tpID ) { _tpID = tpID; }
    int GetDVB_TpID( void ) { return _tpID; }

	void SetDVB_Switch22k( int b22k ) { _switch_22k = b22k; }
	int GetDVB_Switch22k ( void ) { return _switch_22k; }

	void SetDVB_DiseqPort( int port ) { _diseq_port = port; }
	int GetDVB_DiseqPort( void ) { return _diseq_port; }

	void SetDVB_BandWidth( int band ) { _band_width = band; }
	int GetDVB_BandWidth( void ) { return _band_width; }

    void SetDVB_TsID( int tsID ) { _tsID = tsID; }
    int GetDVB_TsID( void ) { return _tsID; }

    void SetDVB_Scrambled( bool scrambled ) { _scrambled = scrambled; }
    bool GetDVB_Scrambled( void ) { return _scrambled; }

	ProgramChannelDvb& operator =(ProgramChannelDvb &rSide)
	{
        SetChanName(rSide.GetChanName());
		SetChanKey(rSide.GetChanKey());
		SetDVB_TpFreq(rSide.GetDVB_TpFreq());
		SetDVB_Symb(rSide.GetDVB_Symb());
		SetDVB_Polarity(rSide.GetDVB_Polarity());
		SetDVB_ProgNum(rSide.GetDVB_ProgNum());
		SetDVB_OrigNetID(rSide.GetDVB_OrigNetID());
		SetDVB_PMT_PID(rSide.GetDVB_PMT_PID());
        SetHDCP_Enable(rSide.GetHDCP_Enable());
        SetMacrovEnable(GetMacrovEnable());
        SetCGMSAEnable(GetCGMSAEnable());
		_tpID        = rSide._tpID;
		_tunerID     = rSide._tunerID;
		_switch_22k  = rSide._switch_22k;
		_diseq_port  = rSide._diseq_port;
        _tsID        = rSide._tsID;
		_serviceType = rSide._serviceType;
        _band_width  = rSide._band_width;
        _scrambled   = rSide._scrambled;
        _modulation  = rSide._modulation;
		return *this;
	}

private:
	int _tpID;
	int _tunerID;
    int _tsID;
	int _switch_22k;
	int _diseq_port;
	int _band_width;
    bool _scrambled;

	ServiceType_e _serviceType;
    Modulation_e _modulation;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _ProgramInfo_H_

