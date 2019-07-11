#ifndef _ProgramChannelC20_H_
#define _ProgramChannelC20_H_

#include "ProgramChannel.h"

#ifdef __cplusplus

namespace Hippo {

class ProgramChannelC20 : public ProgramChannel {
public:
    ProgramChannelC20();
    ~ProgramChannelC20();
	virtual int getNumberID();
	virtual const char *getStringID();
	int GetLogoWidth(){return m_nLogoWidth;};
	void SetLogoWidth(int iarg){m_nLogoWidth = iarg;};
	int GetLogoHeight(){return m_nLogoHeight;};
	void SetLogoHeight(int iarg){m_nLogoHeight = iarg;};
	int GetLogoDisplay(){return m_nLogoDisplay;};
	void SetLogoDisplay(int iarg){m_nLogoDisplay = iarg;};
	int GetLogoHide(){return m_nLogoHide;};
	void SetLogoHide(int iarg){m_nLogoHide = iarg;};
	int GetChanStatus(){return m_nChanStatus;};
	void SetChanStatus(int iarg){m_nChanStatus = iarg;};
	int GetPrevLength(){return m_nPrevLength;};
	void SetPrevLength(int iarg){m_nPrevLength = iarg;};
	int GetPrevCount(){return m_nPrevCount;};
	void SetPrevCount(int iarg){m_nPrevCount = iarg;};
	int GetHasPIP(){return m_nHasPIP;};
	void SetHasPIP(int iarg){m_nHasPIP = iarg;};
	std::string& GetPIPMulticastIP(){return m_HStringPIPMulticastIP;};
	void SetPIPMulticastIP(std::string& CStr){m_HStringPIPMulticastIP = CStr;};
	std::string& GetMulticastPort(){return m_HStringPIPMulticastPort;};
	void SetMulticastPort(std::string& CStr){m_HStringPIPMulticastPort = CStr;};
	std::string& GetPIPMulticastSrcIP(){return m_HStringPIPMulticastSrcIP;};
	void SetPIPMulticastSrcIP(std::string& CStr){m_HStringPIPMulticastSrcIP = CStr;};
	int GetChanBandwith(){return m_nChanBandwith;};
	void SetChanBandwith(int iarg){m_nChanBandwith = iarg;};
	int GetChanEncrypt(){return m_nChanEncrypt;};
	void SetChanEncrypt(int iarg){m_nChanEncrypt = iarg;};
	virtual void SetChanType(int iarg){m_eChanType = (ProgramChannelType)iarg;}
	void SetUpgradeFlag(int iarg) {m_nUpgrade = iarg;}
	int  GetUpgradeFlag() {return m_nUpgrade;}


protected:
	std::string			m_HStringID;
	std::string 		m_HStringPIPMulticastIP;	//PIPС��Ƶ���鲥IP
	std::string			m_HStringPIPMulticastPort;	//PIPС��Ƶ���鲥�˿�
	std::string			m_HStringPIPMulticastSrcIP;	//Ƶ��ΪIGMP V3ʱ���鲥Դ��ַ

	int				m_nChanStatus;	// 1������Ȩδ������2������Ȩ�������� 3��δ��Ȩ

	int   			m_nLogoWidth;
	int   			m_nLogoHeight;
	int				m_nLogoDisplay;
	int				m_nLogoHide;

	int 			m_nHasPIP;
	int				m_nChanBandwith;
	int 			m_nChanEncrypt;		//Ƶ���Ƿ���ţ�0��δ���ţ�1�����ţ�

	int     	    m_nPrevLength;		//Ƶ������Ԥ��ʱ�Ŀ�Ԥ��ʱ��,��λΪ��
	int				m_nPrevCount;		//Ƶ��Ԥ������
	int				m_nUpgrade;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _ProgramInfo_H_
