#ifndef _ProgramChannel_H_
#define _ProgramChannel_H_

#include "Program.h"

#ifdef __cplusplus

namespace Hippo {

class ProgramChannel : public Program {
public:
    enum ProgramChannelType {
    	PCT_VEDIO = 1,
    	PCT_AUDIO,
    	PCT_WEBPAGE,
    	PCT_DVB,
    	PCT_INVALI
    };

    ProgramChannel();
    ~ProgramChannel();

    virtual ProgramType getType() { return PT_CHANNEL; }

    virtual int getNumberID();
    virtual const char *getStringID();

	std::string& GetChanID(){return m_HStringChanID;};
	void SetChanID(std::string& CStr){m_HStringChanID = CStr;};
	std::string& GetChanName(){return m_HStringChanName;};
	void SetChanName(std::string& CStr){m_HStringChanName = CStr;};
	int	GetUserChanID(){return m_nUserChanID;};
	void SetUserChanID(int iarg){m_nUserChanID = iarg;};
	std::string& GetChanURL(){return m_HStringChanURL;};
	void SetChanURL(std::string& CStr){m_HStringChanURL = CStr;};
	int	GetTimeShift(){return m_nTimeShift;};
	void SetTimeShift(int iarg){m_nTimeShift = iarg;};
	std::string& GetTimeShiftURL(){return m_HStringTimeShiftURL;};
	void SetTimeShiftURL(std::string& CStr){m_HStringTimeShiftURL = CStr;};
	ProgramChannelType GetChanType(){return m_eChanType;};
	virtual void SetChanType(int iarg);
	std::string& GetChanLogURL(){return m_HStringChanLogURL;};
	void SetChanLogURL(std::string& CStr){m_HStringChanLogURL = CStr;};
	std::string& GetLogoURL(){return m_HStringLogoURL;};
	void SetLogoURL(std::string& CStr){m_HStringLogoURL = CStr;};
	int GetLogoXPos(){return m_nLogoXPos;};
	void SetLogoXPos(int iarg){m_nLogoXPos = iarg;};
	int GetLogoYPos(){return m_nLogoYPos;};
	void SetLogoYPos(int iarg){m_nLogoYPos = iarg;};
	int GetTimeShiftLength(){return m_nTimeShiftLength;};
	void SetTimeShiftLength(int iarg){m_nTimeShiftLength = iarg;};
	int GetPrevEnable(){return m_nPrevEnable;};
	void SetPrevEnable(int iarg){m_nPrevEnable = iarg;};
	int GetIsHD_Chan(){return m_nIsHDChan;};
	void SetIsHD_Chan(int iarg){m_nIsHDChan = iarg;};
	int GetPrevNum(){return m_nPrevNum;};
	void SetPrevNum(int iarg){m_nPrevNum = iarg;};
	int GetFCC_Enable(){return m_nFCC_Enable;};
	void SetFCC_Enable(int iarg){m_nFCC_Enable = iarg;};
	int GetFEC_Enable(){return m_nFEC_Enable;};
	void SetFEC_Enable(int iarg){m_nFEC_Enable = iarg;};
	int GetSqaCode(){return m_nSQACode;};
	void SetSqaCode(int iarg){m_nSQACode = iarg;};
	int GetRetCode(){return m_nRetCode;};
	void SetRetCode(int iarg){m_nRetCode = iarg;};
    int GetFCCServerValidTime(){return m_nFCCServerValidTime;};
	void SetFCCServerValidTime(int iarg){m_nFCCServerValidTime = iarg;};





	/*******   not sure      *************/
	int GetChanKey(){return m_nChanKey;};
	void SetChanKey(int iarg){	m_nChanKey = iarg;};
	std::string& GetMiniChanURL(){return m_HStringMiniChanURL;};
	void SetMiniChanURL(std::string& CStr){m_HStringMiniChanURL = CStr;};
	std::string& GetMulticastSrcIP(){return m_HStringMulticastSrcIP;};
	void SetMulticastSrcIP(std::string& CStr){m_HStringMulticastSrcIP = CStr;};
	std::string& GetPIP_URL(){return m_HStringPIP_URL;};
	void SetPIP_URL(std::string& CStr){m_HStringPIP_URL = CStr;};
	std::string& GetFCC_Server(){return m_HStringFCC_Server;};
	void SetFCC_Server(std::string& CStr){m_HStringFCC_Server = CStr;};
	std::string& GetFCC_ServerPIP(){return m_HStringFCC_ServerPIP;};
	void SetFCC_ServerPIP(std::string& CStr){m_HStringFCC_ServerPIP = CStr;};

#ifdef INCLUDE_DVBS
	unsigned int GetDVB_TpFreq(){return m_uTP_Freq;};
	void SetDVB_TpFreq(unsigned int uiarg){m_uTP_Freq = uiarg;};
	unsigned int GetDVB_Symb(){return m_uDVB_Symb;};
	void SetDVB_Symb(int iarg){m_uDVB_Symb = iarg;};
	int GetDVB_Polarity(){return m_nDVB_Polarity;};
	void SetDVB_Polarity(int iarg){m_nDVB_Polarity = iarg;};
	int GetDVB_ProgNum(){return m_nDVB_ProgNum;};
	void SetDVB_ProgNum(int iarg){m_nDVB_ProgNum = iarg;};
	int GetDVB_PMT_PID(){	return m_nDVB_PMT_PID;};
	void SetDVB_PMT_PID(int iarg){m_nDVB_PMT_PID = iarg;};
	int GetDVB_OrigNetID(){return m_nDVB_OrigNetID;};
	void SetDVB_OrigNetID(int iarg){m_nDVB_OrigNetID = iarg;};
	int GetDVB_SatLocation(){ return m_nDVB_SatLocation; }
	void SetDVB_SatLocation(int iarg){ m_nDVB_SatLocation = iarg; }
	std::string& GetDVB_SatName(){ return m_HStringDVB_SatName; }
	void SetDVB_SatName(std::string&CStr){ m_HStringDVB_SatName = CStr; }
#endif
	std::string& GetChanCacheTime() { return m_HStringChanCacheTime;}
	void SetChanCacheTime(std::string& CStr) { m_HStringChanCacheTime = CStr;}
	int GetChanDomain() {return m_nChanDomain;}
	void SetChanDomain(int iarg) {m_nChanDomain = iarg;}
	int SetMediaCode(std::string& CStr) {m_MediaCode = CStr; return 0;}
	std::string& GetMediaCode() {return m_MediaCode;}
	std::string& GetEntryID() {return m_EntryID;}
	int GetHDCP_Enable(){return m_nHDCP_Enable;};
	void SetHDCP_Enable(int iarg){m_nHDCP_Enable = iarg;};
	int GetMacrovEnable(){return m_nMacrovEnable;};
	void SetMacrovEnable(int iarg){m_nMacrovEnable = iarg;};
	int GetCGMSAEnable() {return m_nCGMSAEnable;}
	void SetCGMSAEnable(int iarg) {m_nCGMSAEnable = iarg;}

protected:
	std::string m_HStringChanID;
	std::string m_HStringChanName;
	std::string m_HStringChanURL;
	std::string m_HStringTimeShiftURL;
	std::string m_HStringChanLogURL;	//频道 海报地址
	std::string m_HStringLogoURL;	//台标 地址

	ProgramChannelType m_eChanType;
	int m_nUserChanID;
	int m_nTimeShift;
	int m_nTimeShiftLength;
	int m_nIsHDChan;
	int m_nPrevEnable;
	int m_nPrevNum;	                  //预览计数

	int m_nFCC_Enable;	// 0：不支持FCC和RET；1：支持FCC和RET；2：仅支持FCC；3：仅支持RET
	int m_nFEC_Enable;		//频道是否支持FEC (0：不支持；1：支持）
	int m_nSQACode;			// 内部使用，临时保存
	int m_nRetCode;			//内部使用，临时保存
    int m_nFCCServerValidTime;

	int m_nLogoXPos;
	int m_nLogoYPos;

	/*******   not sure      *************/
	std::string m_HStringMiniChanURL;
	std::string m_HStringMulticastSrcIP;


	std::string m_HStringPIP_URL;
	std::string m_HStringFCC_Server;			//RRS-FCC
	std::string m_HStringFCC_ServerPIP;

	int m_nChanKey;

#ifdef INCLUDE_DVBS
	unsigned int m_uTP_Freq;
	unsigned int m_uDVB_Symb;
	int m_nDVB_Polarity;
	int m_nDVB_ProgNum;
	int m_nDVB_PMT_PID;
	int m_nDVB_OrigNetID;
	int m_nDVB_SatLocation;
	std::string m_HStringDVB_SatName;
#endif

	std::string m_HStringChanCacheTime;
	std::string m_MediaCode;
	std::string m_EntryID;
	int m_nChanDomain;
	int m_nHDCP_Enable;
	int m_nMacrovEnable;
	int m_nCGMSAEnable;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _ProgramChannel_H_
