
#include "ProgramParserC20.h"

#include "Program.h"
#include "ProgramAssertions.h"
#include "ProgramList.h"
#include "ProgramChannelC20.h"
#include "SystemManager.h"
#include "ProgramChannel.h"
#ifdef INCLUDE_cPVR
#include "CpvrJsCall.h"
#endif
#ifdef INCLUDE_DVBS
#include "DvbManager.h"
#endif
#include "ProgramVOD.h"
#include "AppSetting.h"
#include "SysSetting.h"

#include "Assertions.h"
#include "app_heartbit.h"
#include "sys_basic_macro.h"
#include "json/json_public.h"

using namespace std;
namespace Hippo {

static ProgramParserC20 gProgramParser;
static int mUpgradeManage = 0;
ProgramParserC20::ProgramParserC20()
{

}

ProgramParserC20::~ProgramParserC20()
{
}

Program *ProgramParserC20::playurlParse( std::string &sUrl )
{
	PROGRAM_LOG("ProgramParserC20::playurlParse channelUrl(%s)\n", sUrl.c_str());
	return 0;
}


ProgramParser &programParser()
{
    return gProgramParser;
}

int ProgramParserC20::searchNodeFromProgramList(VodSource *source, Program **program)
{
	int mediaType = 0;

	SystemManager &sysManager = systemManager();
	mediaType = source->GetMediaType();

	PROGRAM_LOG("ProgramParserC20::searchNodeFromProgramList mediaType(%d)\n", mediaType);
	if(mediaType == ProgramVOD::TYPE_CHANNEL || mediaType == ProgramVOD::TYPE_DVB_CHANNEL){
		std::string channelUrl;
		std::string channelID;
		std::string stringID;

		channelUrl.clear();
		stringID.clear();
		channelID.clear();

		channelUrl = source->GetUrl();
		if(!strncasecmp(channelUrl.c_str(), "igmp://", 7)){
			return 0;
		}
		if(!strncasecmp(channelUrl.c_str(), "channel://", 10 )){
			const char *p = NULL;
#if defined(DEBUG_BUILD)
			PROGRAM_LOG("ProgramParserC20::searchNodeFromProgramList channel url(%s)\n", channelUrl.c_str());
#endif
			p = channelUrl.c_str();
			p = p + 10;
			if(p == NULL){
				PROGRAM_LOG_ERROR("ProgramParserC20::searchNodeFromProgramList channel id is NULL\n");
				return -1;
			}
			channelID = channelUrl.substr(10, channelUrl.length() - 10).c_str();
		} else {
			channelID = channelUrl;
		}

		if(mediaType == ProgramVOD::TYPE_CHANNEL){
			stringID += "MULTIPLE";
			stringID += channelID.c_str();
			*program = sysManager.channelList().getProgramByStringID(stringID.c_str());
			if(*program == NULL){
				PROGRAM_LOG_ERROR("ProgramParserC20::searchNodeFromProgramList can't find program(%s)\n", stringID.c_str());
				return -1;
			}
		} else {
			stringID += "DVB";
			stringID += channelID.c_str();
			*program = sysManager.channelList().getProgramByStringID(stringID.c_str());
			if(*program == NULL){
				PROGRAM_LOG_ERROR("ProgramParserC20::searchNodeFromProgramList can't find program(%s)\n", stringID.c_str());
				return -1;
			}
		}
		((ProgramChannel *)(*program))->SetMediaCode(source->GetMediaCode());
		return 1;
	} else {
		return 0;
	}
}

Program *ProgramParserC20::parseSingleMedia(const char *media)
{
	if (NULL == media) {
		PROGRAM_LOG_ERROR("ProgramParserC20::parseSingleMedia the media is NULL !\n");
		return NULL;
	}
	struct json_object* list = NULL;
	struct json_object* object = NULL;
//	struct json_object* obj = NULL;
	char *param = (char *)media;
	int	obj_num = 0;

	ProgramVOD *m_vod = NULL;
#if defined(DEBUG_BUILD)
	PROGRAM_LOG("ProgramParserC20::parseSingleMedia channel info(%s)\n", param);
#endif
	if('[' == *param){	//json array, eg. [{}] or [{},{}]
		int i = 0, count = 0 ;
		list = json_tokener_parse_string(param);

		if(list == NULL){
			PROGRAM_LOG_ERROR("ProgramParserC20::parseSingleMedia invalid jsonstings(%s)\n", param);
			return NULL;
		}

		obj_num = json_get_object_array_length(list);
		PROGRAM_LOG("ProgramParserC20::parseSingleMedia get json object(%d)\n", obj_num);

		m_vod = new ProgramVOD();

		for(i = 0; i< obj_num; i++){
			object = json_object_get_array_idx(list, i);
			if(NULL == object){
				PROGRAM_LOG_WARNING("ProgramParserC20::parseSingleMedia invalid json(%s)\n", param);
				count++;
				continue;
			}
			m_vod->addVODSource(getVodSourceByJsonObj(object));
		}

		if(count == obj_num){
			if(m_vod)
				delete m_vod;
			json_object_delete(list);
			return NULL;
		}
		json_object_delete(list);

	}else if('{' == *param){ // json object , eg. {}
		object = json_tokener_parse_string(param);
		if(object == NULL){
			PROGRAM_LOG_ERROR("ProgramParserC20::parseSingleMedia invalid json(%s)\n", param);
			return NULL;
		}
		VodSource *source = NULL;
		source = ProgramParser::getVodSourceByJsonObj(object);
		Program *program = NULL;
		int ret = searchNodeFromProgramList(source, &program);
		if(ret != 0){
			json_object_delete(object);
			if(source != NULL){
				delete source;
			}
			if(ret == 1){
				PROGRAM_LOG("ProgramParserC20::parseSingleMedia program(%p)\n", program);
				return program;
			} else {
				return NULL;
			}
		}
		m_vod = new ProgramVOD();
		m_vod->addVODSource(source);
		json_object_delete(object);
	}else{ // unvalid json
		PROGRAM_LOG_ERROR("ProgramParserC20::parseSingleMedia invalid json(%s)\n", param );
		return NULL;
	}

	return m_vod;
}

int
ProgramParserC20::setOpenSqaCode(ProgramChannelC20*pNode)
{
//FEC+RET+FEC
//华为文档中没有fec 开启的标志位，默认情况下只要是组播就要开启sqa 模块
	int FEC_Enable = pNode->GetFEC_Enable();
	int FCC_Enable = pNode->GetFCC_Enable();
	int sqa_code = 0;
	int ret_code = 1;
	int sqa_fcc = 0, sqa_ret = 0, sqa_fec = 0;
	int fccSwitch =0;

	sysSettingGetInt("sqa_fcc", &sqa_fcc, 0);
	sysSettingGetInt("sqa_ret", &sqa_ret, 0);
	sysSettingGetInt("sqa_fec", &sqa_fec, 0);

    appSettingGetInt("fcc_switch", &fccSwitch, 0);
	if(FEC_Enable ==0)
		ret_code = 0;
	if(fccSwitch == 1){
		switch(FCC_Enable ){
			case 0://no fcc and ret
				ret_code = 0;
				break;
			case 1://open fcc + ret
				if(1 == sqa_fcc){
					sqa_code |= SQA_FCC_CODE;
					PROGRAM_LOG("open FCC in fccmode 1\n");
				}
				if(1 == sqa_ret){
					sqa_code |= SQA_RET_CODE;
					PROGRAM_LOG("open RET in fccmode 1\n");
				}
				ret_code = 1;
				break;
			case 2://open fcc
				if(1 == sqa_fcc){
					sqa_code |= SQA_FCC_CODE;
					PROGRAM_LOG("open FCC in fccmode 1\n");
				}
				ret_code = 0;
				break;
			case 3://open ret
				if(1 == sqa_ret){
					sqa_code |= SQA_RET_CODE;
					PROGRAM_LOG("open RET in fccmode 1\n");
				}
				ret_code = 1;
				break;
			default:
				break;
		}
		if(1 == sqa_fec){//open fec
			sqa_code |= SQA_FEC_CODE;
			PROGRAM_LOG("open FEC in fccmode 1\n");
		}
	} else {
		sqa_code = 0;
		ret_code = 0;
	}
	if(0 == sqa_code){
		PROGRAM_LOG("FCC RET FEC IS CLOSED!\n");
	}
	pNode->SetSqaCode(sqa_code);
	pNode->SetRetCode(ret_code);
	return 0;
}

int
ProgramParserC20::parseChannelList(ProgramList *list, const char *mediaList)
{
	SystemManager &sysManager = systemManager();
	ProgramChannelC20 *pChannelInfoNode = NULL;
	vector<std::string> vLine;
	vector<std::string>::iterator iteratorIt, iteratorKey, iteratorValue;
	string::size_type s1, s2;
	std::string m_channelList;
	int index = 0;

	if(mUpgradeManage == 1){
		PROGRAM_LOG("ProgramParserC20::parseChannelList channel count(%d)\n", sysManager.channelList().getProgramCount());
		for(index = 0; index < sysManager.channelList().getProgramCount(); index++){
			ProgramChannelC20 *program = (ProgramChannelC20 *)(sysManager.channelList().getProgramByIndex(index));
#ifdef INCLUDE_DVBS
            int flag = 0;
            sysSettingGetInt("DVBChannelManageFlag", &flag, 0);
			if(flag == 0){
				if(program->GetChanDomain() != 1){
					program->SetUpgradeFlag(1);
				}
			} else
#endif
			{
				program->SetUpgradeFlag(1);
			}
		}
	}
	m_channelList.clear();
	m_channelList = mediaList;
    m_parseChannellistOK = 1;
	if(m_channelList.empty()){
		PROGRAM_LOG_ERROR("ProgramParserC20::parseChannelList channellist is empty !!\n");
		return 0;
	}
	ChannelListTokenize(m_channelList, vLine);
	iteratorIt = vLine.begin();

	while(iteratorIt < vLine.end()){
		iteratorKey = iteratorIt;
		if (iteratorIt + 1 < vLine.end()) {
			//PROGRAM_LOG("ProgramParserC20::parseChannelList channel value = %s \n", iteratorKey->c_str());
			if(!iteratorKey->compare("END")) {
				if(pChannelInfoNode != NULL){
					if(pChannelInfoNode->GetChanDomain() == 1){
                        /* user class ProgramChannelDvb */
#ifdef INCLUDE_DVBS
                        DvbChannelParseAdd(pChannelInfoNode);
                        delete (pChannelInfoNode);
                        pChannelInfoNode = 0;
#endif
					} else {
						setOpenSqaCode(pChannelInfoNode);
						upgradeProgram(pChannelInfoNode);
					}
				}
				break;
			}
			else if (!iteratorKey->compare("VideoChannel")) {
				iteratorIt++;
				continue;
			}
			else if (!iteratorKey->compare("AudioChannel")) {
				iteratorIt++;
				continue;
			}
			else if (!iteratorKey->compare("WebChannel")) {
				iteratorIt++;
				continue;
			}
			else if (!iteratorKey->compare("DVBChannel")) {
				iteratorIt++;
				continue;
			}
			else if (!iteratorKey->compare("ChannelVer")) {
				char channelVer[32] = {0};
				iteratorValue = iteratorIt + 1;
				if (iteratorValue->length() > 31)
				    strncpy(channelVer, iteratorValue->c_str(), 31);
				else
				    strcpy(channelVer, iteratorValue->c_str());
				channel_array_set_version(channelVer, 0);
			}
			else if (!iteratorKey->compare("ChannelNum")) {
				iteratorIt += 2;
				continue;
			}
			else if (!iteratorKey->compare("ChanID")) {
				if(pChannelInfoNode != NULL){
					if(pChannelInfoNode->GetChanDomain() == 1){
#ifdef INCLUDE_DVBS
                        DvbChannelParseAdd(pChannelInfoNode);
                        delete (pChannelInfoNode);
                        pChannelInfoNode = 0;
#endif
					} else {
						setOpenSqaCode(pChannelInfoNode);
						upgradeProgram(pChannelInfoNode);
					}
				}
				pChannelInfoNode = new ProgramChannelC20();
				iteratorValue = iteratorIt + 1;
				pChannelInfoNode->SetChanID(*iteratorValue);
			}
			else if (!iteratorKey->compare("ChanStatus")) {
				iteratorValue = iteratorIt + 1;
				pChannelInfoNode->SetChanStatus(atoi(iteratorValue->c_str()));
			}
			else if (!iteratorKey->compare("PreviewEnable")) {
				iteratorValue = iteratorIt + 1;
				pChannelInfoNode->SetPrevEnable(atoi(iteratorValue->c_str()));
			}
			else if (!iteratorKey->compare("ChanKey")) {
				iteratorValue = iteratorIt + 1;
				pChannelInfoNode->SetChanKey(atoi(iteratorValue->c_str()));
				pChannelInfoNode->SetUserChanID(atoi(iteratorValue->c_str()));
			}
			else if (!iteratorKey->compare("PLTVEnable")) {
				iteratorValue = iteratorIt + 1;
				pChannelInfoNode->SetTimeShift(atoi(iteratorValue->c_str()));
			}
			else if (!iteratorKey->compare("PauseLength")) {
				iteratorValue = iteratorIt + 1;
				pChannelInfoNode->SetTimeShiftLength(atoi(iteratorValue->c_str()));
			}
			else if (!iteratorKey->compare("ChanName"))	{
				iteratorValue = iteratorIt + 1;
				pChannelInfoNode->SetChanName(*iteratorValue);
			}
			else if (!iteratorKey->compare("ChanLogoUrl")) {
				iteratorValue = iteratorIt + 1;
				pChannelInfoNode->SetChanLogURL(*iteratorValue);
			}
			else if (!iteratorKey->compare("ChanURL"))	{
				iteratorValue = iteratorIt + 1;
				if((s1 = iteratorValue->find("igmp://")) != string::npos) {
					if((s2 = iteratorValue->find("|rtsp://")) != string::npos) {
						std::string tmpStr = iteratorValue->substr(s1, s2);
						pChannelInfoNode->SetChanURL(tmpStr);
						tmpStr = iteratorValue->substr(s2 + 1,iteratorValue->length());
						pChannelInfoNode->SetTimeShiftURL(tmpStr);
					} else {
						pChannelInfoNode->SetChanURL(*iteratorValue);
					}
				} else {
				       if (iteratorValue->find("http://") != string::npos) {
                                       pChannelInfoNode->SetChanURL(*iteratorValue);
					} else {
					    pChannelInfoNode->SetTimeShiftURL(*iteratorValue);
					}
				}
			}
			else if (!iteratorKey->compare("LogoURL")) {
				iteratorValue = iteratorIt + 1;
				pChannelInfoNode->SetLogoURL(*iteratorValue);
			}
			else if (!iteratorKey->compare("IsHDChannel"))	{
				iteratorValue = iteratorIt + 1;
				pChannelInfoNode->SetIsHD_Chan(atoi(iteratorValue->c_str()));
			}
			else if (!iteratorKey->compare("ChanEncrypt")) {
				iteratorValue = iteratorIt + 1;
				pChannelInfoNode->SetChanEncrypt(atoi(iteratorValue->c_str()));
			}
			else if (!iteratorKey->compare("hasPip"))	{
				iteratorValue = iteratorIt + 1;
				pChannelInfoNode->SetHasPIP(atoi(iteratorValue->c_str()));
			}
			else if (!iteratorKey->compare("pipMulticastIP")) {
				iteratorValue = iteratorIt + 1;
				pChannelInfoNode->SetPIPMulticastIP(*iteratorValue);
			}
			else if (!iteratorKey->compare("pipMulticastPort"))	{
				iteratorValue = iteratorIt + 1;
				pChannelInfoNode->SetMulticastPort(*iteratorValue);
				if(1 == pChannelInfoNode->GetHasPIP()){
					std::string str_tmp;
					str_tmp.clear();
					str_tmp += "igmp://";
					str_tmp += pChannelInfoNode->GetPIPMulticastIP();
					str_tmp += ":";
					str_tmp += pChannelInfoNode->GetMulticastPort();
					pChannelInfoNode->SetMiniChanURL(str_tmp);

				}
			}
			else if (!iteratorKey->compare("ChanType")) {
				iteratorValue = iteratorIt + 1;
				pChannelInfoNode->SetChanType(atoi(iteratorValue->c_str()));
			}
#ifdef INCLUDE_DVBS
			else if (!iteratorKey->compare("TP_frequency"))	{
				iteratorValue = iteratorIt + 1;
				pChannelInfoNode->SetDVB_TpFreq(strtol(iteratorValue->c_str(),NULL,10));
			}
			else if (!iteratorKey->compare("DVB_Symborate")) {
				iteratorValue = iteratorIt + 1;
				pChannelInfoNode->SetDVB_Symb(atoi(iteratorValue->c_str()));
			}
			else if (!iteratorKey->compare("DVB_Polarity"))	{
				iteratorValue = iteratorIt + 1;
				pChannelInfoNode->SetDVB_Polarity(atoi(iteratorValue->c_str()));
			}
			else if (!iteratorKey->compare("DVB_transportStreamId")) {
				iteratorValue = iteratorIt + 1;
				pChannelInfoNode->SetDVB_PMT_PID(atoi(iteratorValue->c_str()));
			}
			else if (!iteratorKey->compare("DVB_Original_network_id"))	{
				iteratorValue = iteratorIt + 1;
				pChannelInfoNode->SetDVB_OrigNetID(atoi(iteratorValue->c_str()));
			}
			else if (!iteratorKey->compare("DVB_serviceID")) {
				iteratorValue = iteratorIt + 1;
				pChannelInfoNode->SetDVB_ProgNum(atoi(iteratorValue->c_str()));
			}
			else if (!iteratorKey->compare("satName")) {
				iteratorValue = iteratorIt + 1;
				pChannelInfoNode->SetDVB_SatName(*iteratorValue);
			}
			else if(!iteratorKey->compare("satLongitude")) {
				iteratorValue = iteratorIt + 1;
				pChannelInfoNode->SetDVB_SatLocation(atoi(iteratorValue->c_str()));
			}
#endif
			else if (!iteratorKey->compare("ChanBandwith"))	{
				iteratorValue = iteratorIt + 1;
				pChannelInfoNode->SetChanBandwith(atoi(iteratorValue->c_str()));
			}
			else if (!iteratorKey->compare("FCCEnable")) {
				iteratorValue = iteratorIt + 1;
				pChannelInfoNode->SetFCC_Enable(atoi(iteratorValue->c_str()));
			}
			else if (!iteratorKey->compare("PreviewLength"))	{
				iteratorValue = iteratorIt + 1;
				pChannelInfoNode->SetPrevLength(atoi(iteratorValue->c_str()));
			}
			else if (!iteratorKey->compare("PreviewCount")) {
				iteratorValue = iteratorIt + 1;
				pChannelInfoNode->SetPrevCount(atoi(iteratorValue->c_str()));
			}
			else if (!iteratorKey->compare("LogoSize"))	{
				iteratorValue = iteratorIt + 1;
				if((s1 = iteratorValue->find(",")) == string::npos){
					std::string first, second;
					first.clear();
					second.clear();
					first = iteratorValue->substr(0, s1);
					second = iteratorValue->substr(s1 + 1, iteratorValue->length());
		      pChannelInfoNode->SetLogoWidth(atoi(first.c_str()));
		      pChannelInfoNode->SetLogoHeight(atoi(second.c_str()));
				}
			}
			else if (!iteratorKey->compare("LogoLocation")) {
				iteratorValue = iteratorIt + 1;
				if((s1 = iteratorValue->find(",")) != string::npos){
					std::string first, second;
					first.clear();
					second.clear();
					first = iteratorValue->substr(0, s1);
					second = iteratorValue->substr(s1 + 1, iteratorValue->length());
		      pChannelInfoNode->SetLogoXPos(atoi(first.c_str()));
		      pChannelInfoNode->SetLogoYPos(atoi(second.c_str()));
				}
			}
			else if (!iteratorKey->compare("LogoDisplay"))	{
				iteratorValue = iteratorIt + 1;
				if((s1 = iteratorKey->find(",")) != string::npos){
					std::string first, second;
					first.clear();
					second.clear();
					first = iteratorKey->substr(0, s1 - 1);
					second = iteratorKey->substr(s1 + 1, iteratorKey->length());
					pChannelInfoNode->SetLogoDisplay(atoi(first.c_str()));
					pChannelInfoNode->SetLogoHide(atoi(second.c_str()));
				}
			}
			else if (!iteratorKey->compare("HDCPEnable")) {
				iteratorValue = iteratorIt + 1;
				pChannelInfoNode->SetHDCP_Enable(atoi(iteratorValue->c_str()));
			}
			else if (!iteratorKey->compare("CGMSAEnable"))	{
				iteratorValue = iteratorIt + 1;
				pChannelInfoNode->SetCGMSAEnable(atoi(iteratorValue->c_str()));
			}
			else if (!iteratorKey->compare("MacrovisionEnable")) {
				iteratorValue = iteratorIt + 1;
				pChannelInfoNode->SetMacrovEnable(atoi(iteratorValue->c_str()));
			}
			else if (!iteratorKey->compare("FECEnable"))	{
				iteratorValue = iteratorIt + 1;
				pChannelInfoNode->SetFEC_Enable(atoi(iteratorValue->c_str()));
			}
			else if (!iteratorKey->compare("ChanCacheTime")) {
				iteratorValue = iteratorIt + 1;
				pChannelInfoNode->SetChanCacheTime(*iteratorValue);
			}
			else if(!iteratorKey->compare("chanDomain")){
				int chanDomain = 0;
				iteratorValue = iteratorIt + 1;
				if(!iteratorValue->compare("IPTV")){
					chanDomain = 0;
				} else if(!iteratorValue->compare("DVB")){
					chanDomain = 1;
				} else if(!iteratorValue->compare("OTT")){
					chanDomain = 2;
				} else {
					chanDomain = 0;
				}
				pChannelInfoNode->SetChanDomain(chanDomain);
			}
			else if (!iteratorKey->compare("Checkcode"))	{
				iteratorValue = iteratorIt + 1;
				//pChannelInfoNode->SetCheckcode(*iteratorValue);
			} else {
				iteratorIt++;
				continue;
			}
			//PROGRAM_LOG("ProgramParserC20::parseChannelList %s = %s\n", iteratorKey->c_str(), iteratorValue->c_str());
			iteratorIt = iteratorIt + 2;
		}

	}
	if(mUpgradeManage == 1){
		for(index = sysManager.channelList().getProgramCount(); index > 0 ; index--){
			ProgramChannelC20 *program = (ProgramChannelC20 *)(sysManager.channelList().getProgramByIndex(index - 1));

			PROGRAM_LOG("ProgramParserC20::parseChannelList channelKey(%d), UpgradeFlag(%d)\n",program->GetChanKey(),program->GetUpgradeFlag());
			if(program->GetUpgradeFlag() == 1){
				sysManager.channelList().removeProgramByIndex(index - 1);
			}
		}
	} else {
			mUpgradeManage = 1;
	}
#ifdef INCLUDE_cPVR
    cpvrTaskResumeAfterReboot(2);
#endif
   	return 0;
}

int
ProgramParserC20::ChannelListTokenize( /*in*/ std::string& store, /*out*/ vector<std::string>& line)
{
	token_state_e estate = TokenState_eInKey;
	std::string HStringTmp;

	string::const_iterator iteratorIt = store.begin();

  	HStringTmp.erase(0,HStringTmp.length());

	for( ; iteratorIt < store.end(); iteratorIt++ ){
		if('[' == *iteratorIt){
			if(estate == TokenState_eInKey){
				;
			}
			else{
				HStringTmp += *iteratorIt;
			}
		}
		else if(']' == *iteratorIt){
			if(estate == TokenState_eInKey){
				line.push_back( HStringTmp );
				HStringTmp.erase(0,HStringTmp.length());
			}
			else{
				HStringTmp += *iteratorIt;
			}
		}
		else if(('\r' == *iteratorIt)||('\n' == *iteratorIt)){
				line.push_back( HStringTmp );
				HStringTmp.erase(0,HStringTmp.length());
			if(estate == TokenState_eInValue){
				estate = TokenState_eInKey;
			}
			else if(estate == TokenState_eInTag){
				estate = TokenState_eInKey;
			}
			else{
				estate = TokenState_eInKey;
			}
		}
		else if(' ' == *iteratorIt){
			continue;
		}
		else if(!isprint( *iteratorIt ))
			continue;
		else if(*iteratorIt == '='){
			if(estate == TokenState_eInKey){
				line.push_back( HStringTmp );
				HStringTmp.erase(0,HStringTmp.length());
				estate = TokenState_eInTag;
			}
			else{
				HStringTmp += *iteratorIt;
			}
		}
		else{
			if(estate == TokenState_eInTag){
				estate = TokenState_eInValue;
			}
			HStringTmp += *iteratorIt;
		}
	}
		line.push_back( HStringTmp );
		return 0;
}


void
ProgramParserC20::upgradeProgram(ProgramChannelC20 *pNode)
{
	SystemManager &sysManager = systemManager();
	std::string m_stringID;
	char channelKey[16];

	if(mUpgradeManage == 0){
		sysManager.channelList().addProgram(pNode);
		return;
	}

	m_stringID.clear();
	memset(channelKey, 0, sizeof(channelKey));
	sprintf(channelKey, "%d", pNode->GetChanKey());
	m_stringID += "MULTIPLE";
	m_stringID += channelKey;

	PROGRAM_LOG("ProgramParserC20::upgradeProgram m_stringId(%s)\n", m_stringID.c_str());
	ProgramChannelC20 *program = (ProgramChannelC20 *)(sysManager.channelList().getProgramByStringID(m_stringID.c_str()));
	if(program != NULL){
		program->SetChanID(pNode->GetChanID());
		program->SetChanStatus(pNode->GetChanStatus());
		program->SetPrevEnable(pNode->GetPrevEnable());
		program->SetChanKey(pNode->GetChanKey());
		program->SetUserChanID(pNode->GetUserChanID());
		program->SetTimeShift(pNode->GetTimeShift());
		program->SetTimeShiftLength(pNode->GetTimeShiftLength());
		program->SetChanName(pNode->GetChanName());
		program->SetChanLogURL(pNode->GetChanLogURL());
		program->SetChanURL(pNode->GetChanURL());
		program->SetTimeShiftURL(pNode->GetTimeShiftURL());
		program->SetLogoURL(pNode->GetLogoURL());
		program->SetIsHD_Chan(pNode->GetIsHD_Chan());
		program->SetChanEncrypt(pNode->GetChanEncrypt());
		program->SetHasPIP(pNode->GetHasPIP());
		program->SetPIPMulticastIP(pNode->GetPIPMulticastIP());
		program->SetMulticastPort(pNode->GetMulticastPort());
		program->SetMiniChanURL(pNode->GetMiniChanURL());
		program->SetChanType(pNode->GetChanType());
#ifdef INCLUDE_DVBS
		program->SetDVB_TpFreq(pNode->GetDVB_TpFreq());
		program->SetDVB_Symb(pNode->GetDVB_Symb());
		program->SetDVB_Polarity(pNode->GetDVB_Polarity());
		program->SetDVB_PMT_PID(pNode->GetDVB_PMT_PID());
		program->SetDVB_OrigNetID(pNode->GetDVB_OrigNetID());
		program->SetDVB_ProgNum(pNode->GetDVB_ProgNum());
		program->SetDVB_SatLocation(pNode->GetDVB_SatLocation());
		program->SetDVB_SatName(pNode->GetDVB_SatName());
#endif
		program->SetChanBandwith(pNode->GetChanBandwith());
		program->SetFCC_Enable(pNode->GetFCC_Enable());
		program->SetPrevLength(pNode->GetPrevLength());
		program->SetPrevCount(pNode->GetPrevCount());
		program->SetLogoWidth(pNode->GetLogoWidth());
		program->SetLogoHeight(pNode->GetLogoHeight());
		program->SetLogoXPos(pNode->GetLogoXPos());
		program->SetLogoDisplay(pNode->GetLogoDisplay());
		program->SetHDCP_Enable(pNode->GetHDCP_Enable());
		program->SetCGMSAEnable(pNode->GetCGMSAEnable());
		program->SetMacrovEnable(pNode->GetMacrovEnable());
		program->SetFEC_Enable(pNode->GetFEC_Enable());
		program->SetSqaCode(pNode->GetSqaCode());
		program->SetRetCode(pNode->GetRetCode());
		program->SetChanCacheTime(pNode->GetChanCacheTime());
		program->SetChanDomain(pNode->GetChanDomain());
		//program->SetCheckcode(pNode->GetCheckcode());
		program->SetUpgradeFlag(0);
		delete pNode;
	} else {
		sysManager.channelList().addProgram(pNode);
	}

}

} // namespace Hippo


