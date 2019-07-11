#ifndef _ProgramChannelC10_H_
#define _ProgramChannelC10_H_

#include "ProgramChannel.h"

#ifdef __cplusplus

namespace Hippo {

class ProgramChannelC10 : public ProgramChannel {
public:
    ProgramChannelC10();
    ~ProgramChannelC10();

    std::string& GetChanSDP(){return m_HStringChanSDP;};
    void SetChanSDP(std::string& CStr){m_HStringChanSDP = CStr;};
    int GetChanPurchased(){return m_nChanPurchased;};
    void SetChanPurchased(int iarg){m_nChanPurchased = iarg;};
    int GetBeginTime(){return m_nBeginTime;};
    void SetBeginTime(int iarg){m_nBeginTime = iarg;};
    int GetInterval(){return m_nInterval;};
    void SetInterval(int iarg){m_nInterval = iarg;};
    int GetLasting(){return m_nLasting;};
    void SetLasting(int iarg){m_nLasting = iarg;};
    int GetChanLocked(){return m_nChanLocked;};
    void SetChanLocked(int iarg){m_nChanLocked = iarg;};
    int GetActionType(){return m_nActionType;};
    void SetActionType(int iarg){m_nActionType = iarg;};
    unsigned int GetChannelFECPort(){return m_nChannelFECPort;};
    void SetChannelFECPort(unsigned int iarg){m_nChannelFECPort = iarg;};
    int GetCanUnicast(){return m_nCanUnicast;};
    void SetCanUnicast(int iarg){m_nCanUnicast = iarg;};
	virtual void SetChanType(int iarg);

protected:

    std::string         m_HStringChanSDP;

    int             m_nChanPurchased;
    int             m_nChanLocked;

    int             m_nBeginTime;
    int             m_nInterval;
    int             m_nLasting;

    int             m_nActionType;
    unsigned int    m_nChannelFECPort;
    int             m_nCanUnicast;

};

} // namespace Hippo

#endif // __cplusplus

#endif // _ProgramInfo_H_
