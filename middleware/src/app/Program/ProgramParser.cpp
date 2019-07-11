
#include "ProgramParser.h"

#include "Program.h"
#include "ProgramVOD.h"
#include "ProgramList.h"
#include "ProgramAssertions.h"

#include "AdvertisementList.h"
#include "string.h"
#ifdef TVMS_OPEN
#include "tvms_define.h"
#include "tvms_setting.h"
#endif

#include <ctype.h>
using namespace std;
namespace Hippo {

bool ProgramParser::m_parseChannellistOK = false;
ProgramParser::ProgramParser()
{
}

ProgramParser::~ProgramParser()
{
}

int ProgramParser::tokenize(std::string& str,/*out*/ vector<std::string>& aKey, /*out*/ vector<std::string>& aValue, int aTag)
{
    //int point = true;
    token_state_e state = TokenState_eBeforeKey;
    std::string tmp;
    string::const_iterator it = str.begin();
    bool singalQuoteOpened = false;
    bool doubleQuotedOpend = false;
    for(; it < str.end(); it++) {
        if(!singalQuoteOpened && !doubleQuotedOpend && *it == aTag) {
            state = TokenState_eBeforeValue;
            aKey.push_back(tmp);
            tmp.clear();
            continue;
        } else if(!isprint(*it)) {
            //continue; //此代码先注掉，因为会影响频道列表ChannelName乱码
        }

        if(*it == ' ') {
            switch(state) {
            case TokenState_eInKey:
                state = TokenState_eAfterKey;
                break;
            case TokenState_eInValue:
                state = TokenState_eBeforeKey;
                break;
            default:
                ;
            }
        } else if(*it == '\'') {
            if(!singalQuoteOpened) {
                switch(state) {
                case TokenState_eInValue:
                    tmp += *it;
                    break;
                case TokenState_eAfterKey:
                default:
                    break;
                }
                singalQuoteOpened = true;
            } else {
                if(doubleQuotedOpend)
                    tmp += *it;
                singalQuoteOpened = false;
            }
        } else if(*it == '\"') {
            if(!doubleQuotedOpend) {
                switch(state) {
                case TokenState_eInValue:
                    tmp += *it;
                    break;
                case TokenState_eAfterKey:
                default:
                    break;
                }
                doubleQuotedOpend = true;
            } else {
                if(singalQuoteOpened)
                    tmp += *it;
                doubleQuotedOpend = false;
            }
        } else if(*it == ',') {
            if(state == TokenState_eInValue && (singalQuoteOpened == true || doubleQuotedOpend == true))
                tmp += *it;
            else {
                aValue.push_back(tmp);
                tmp.clear();
                state = TokenState_eBeforeKey;
            }
        } else if(*it == '\r' || *it == '\n') {
            aValue.push_back(tmp);
            tmp.clear();
            state = TokenState_eBeforeKey;
        } else {
            tmp += *it;
            switch(state) {
            case TokenState_eBeforeKey:
                state = TokenState_eInKey;
                break;
            case TokenState_eBeforeValue:
                state = TokenState_eInValue;
                break;
            default:
                break;
            }
        }
    }
    aValue.push_back(tmp);
    return 0;
}

VodSource*
ProgramParser::getVodSourceByStr(std::string& sDiscription)
{
    vector<std::string> key, value;
    //int founded = 0;
    unsigned int i = 0;
    char mediacode[1024] = {0};
//    vector<std::string>::iterator keyIt, valueIt;
    VodSource *m_sourceNode = new VodSource();

    PROGRAM_LOG("url:%s\n", sDiscription.c_str());

    ProgramParser::tokenize(sDiscription, key, value );
//    keyIt = key.begin( );
//    valueIt = value.begin( );

    for( i=0; i<key.size() && i<value.size(); ++i ) {
        if(!strncasecmp(key[i].c_str(), "mediaURL", 8 )) {
#ifdef TVMS_OPEN
    tvms_mediaurl_mediacode(mediacode, value[i].c_str());
    tvms_mediacode_write(mediacode);
#endif

            m_sourceNode->SetUrl(value[i]);
        }
        else if(!strncasecmp(key[i].c_str(), "mediaCode", 9 )) {
            m_sourceNode->SetMediaCode(value[i]);
        }
        else if(!strncasecmp(key[i].c_str(), "allowTrickmode", 14 ) ) {
            m_sourceNode->SetAllowTrickmode(atoi(value[i].c_str()));
        }
        else if(!strncasecmp(key[i].c_str(), "insertTime", 10 )) {
            m_sourceNode->SetInsertTime(atoi(value[i].c_str()));
        }

    }

    return m_sourceNode;
}

VodSource*
ProgramParser::getVodSourceByJsonObj(struct json_object *object)
{
	struct json_object* obj = NULL;
	VodSource *m_sourceNode = new VodSource();
	std::string sValue;
       char mediacode[1024] = {0};

	if(NULL == object){
		if(m_sourceNode)
			delete m_sourceNode;
		return NULL;
	}
	//mediaUrl
	obj = json_object_get_object_bykey(object,"mediaUrl");
	if(obj==NULL){
		obj = json_object_get_object_bykey(object,"mediaURL");
			if(obj == NULL){
			    PROGRAM_LOG_ERROR( "no mediaUrl\n");
			if(m_sourceNode)
				delete m_sourceNode;
			return NULL;
		}
	}
	sValue = json_object_get_string(obj);
	m_sourceNode->SetUrl(sValue);
#ifdef TVMS_OPEN
    tvms_mediaurl_mediacode(mediacode, sValue.c_str());
    tvms_mediacode_write(mediacode);
#endif

	//mediaCode
	obj = json_object_get_object_bykey(object,"mediaCode");
	if(obj==NULL)
		PROGRAM_LOG("no mediaCode\n");
	else{
		sValue = json_object_get_string(obj);
		PROGRAM_LOG("mediaCode:%s\n", sValue.c_str());
		m_sourceNode->SetMediaCode(sValue);
	}
	//entryID
	obj = json_object_get_object_bykey(object,"entryID");
	if(obj==NULL)
		PROGRAM_LOG("no entryID\n");
	else{
		sValue = json_object_get_string(obj);
		PROGRAM_LOG("entryID:%s\n", sValue.c_str());
		m_sourceNode->SetEntryID(sValue);
	}
	//mediaType
	obj = json_object_get_object_bykey(object,"mediaType");
	if(obj==NULL)
		PROGRAM_LOG("no mediaType\n");
	else{
		sValue = json_object_get_string(obj);
		PROGRAM_LOG("mediaType:%s\n", sValue.c_str());
		m_sourceNode->SetMediaType(atoi(sValue.c_str()));
	}
	//audioType
	obj = json_object_get_object_bykey(object,"audioType");
	if(obj==NULL)
		PROGRAM_LOG("no audioType\n");
	else{
		sValue = json_object_get_string(obj);
		PROGRAM_LOG("audioType:%s\n", sValue.c_str());
		m_sourceNode->SetAudioType(atoi(sValue.c_str()));
	}
	//videoType
	obj = json_object_get_object_bykey(object,"videoType");
	if(obj==NULL)
		PROGRAM_LOG("no videoType\n");
	else{
		sValue = json_object_get_string(obj);
		PROGRAM_LOG("videoType:%s\n", sValue.c_str());
		m_sourceNode->SetVideoType(atoi(sValue.c_str()));
	}
	//streamType
	obj = json_object_get_object_bykey(object,"streamType");
	if(obj==NULL)
		PROGRAM_LOG("no streamType\n");
	else{
		sValue = json_object_get_string(obj);
		PROGRAM_LOG("streamType:%s\n", sValue.c_str());
		m_sourceNode->SetStreamType(atoi(sValue.c_str()));
	}
	//drmType
	obj = json_object_get_object_bykey(object,"drmType");
	if(obj==NULL)
		PROGRAM_LOG("no drmType\n");
	else{
		sValue = json_object_get_string(obj);
		PROGRAM_LOG("drmType:%s\n", sValue.c_str());
		m_sourceNode->SetDrmType(atoi(sValue.c_str()));
	}
	//fingerPrint
	obj = json_object_get_object_bykey(object,"fingerPrint");
	if(obj==NULL)
		PROGRAM_LOG("no fingerPrint\n");
	else{
		sValue = json_object_get_string(obj);
		PROGRAM_LOG("fingerPrint:%s\n", sValue.c_str());
		m_sourceNode->SetFingerPrint(atoi(sValue.c_str()));
	}
	//copyProtection
	obj = json_object_get_object_bykey(object,"copyProtection");
	if(obj==NULL)
		PROGRAM_LOG("no copyProtection\n");
	else{
		sValue = json_object_get_string(obj);
		PROGRAM_LOG("copyProtection:%s\n", sValue.c_str());
		m_sourceNode->SetCopyProtection(atoi(sValue.c_str()));
	}
	//allowTrickmode
	obj = json_object_get_object_bykey(object,"allowTrickmode");
	if(obj==NULL)
		PROGRAM_LOG("no allowTrickmode\n");
	else{
		sValue = json_object_get_string(obj);
		PROGRAM_LOG("allowTrickmode:%s\n", sValue.c_str());
		m_sourceNode->SetAllowTrickmode(atoi(sValue.c_str()));
	}
	//startTime
	obj = json_object_get_object_bykey(object,"startTime");
	if(obj==NULL)
		PROGRAM_LOG("no startTime\n");
	else{
		sValue = json_object_get_string(obj);
		m_sourceNode->SetStartTime(sValue);
		PROGRAM_LOG("startTime:%s\n", m_sourceNode->GetStartTime().c_str());
	}
	//endTime
	obj = json_object_get_object_bykey(object,"endTime");
	if(obj==NULL)
		PROGRAM_LOG("no endTime\n");
	else{
		sValue = json_object_get_string(obj);
		if(sValue.empty()
		   || !strncasecmp(sValue.c_str(), "20000", 5)
		   || !strncasecmp(sValue.c_str(), "null", 4)
		   || strchr(sValue.c_str(), '.')){
            std::string temp("0");
			m_sourceNode->SetEndTime(temp);
		}
		else
			m_sourceNode->SetEndTime(sValue);
		PROGRAM_LOG("endTime:%s\n", m_sourceNode->GetEndTime().c_str());
	}
	//insertTime
	obj = json_object_get_object_bykey(object,"insertTime");
	if(obj==NULL)
		PROGRAM_LOG("no insertTime\n");
	else{
		m_sourceNode->SetInsertTime(atoi(json_object_get_string(obj)));
		PROGRAM_LOG("insertTime:%d\n", m_sourceNode->GetInsertTime());
	}
	//isPostive
	obj = json_object_get_object_bykey(object,"isPostive");
	if(obj==NULL)
		PROGRAM_LOG("no isPostive\n");
	else{
		sValue = json_object_get_string(obj);
		PROGRAM_LOG("isPostive:%s\n", sValue.c_str());
		m_sourceNode->SetIsPostive(atoi(sValue.c_str()));
	}
	//bandwidth
	obj = json_object_get_object_bykey(object,"bandwidth");
	if(obj==NULL)
		PROGRAM_LOG("no bandwidth\n");
	else{
		sValue = json_object_get_string(obj);
		PROGRAM_LOG("bandwidth:%s\n", sValue.c_str());
		m_sourceNode->SetBandwidth(atoi(sValue.c_str()));
	}
	return m_sourceNode;
}

/*******************************************************
*function
*input: sUrl  C10 eg. [{mediaUrl:"rtsp://110.1.1.164/88888888/16/20120206/268435469/268435469.ts?rrsip=110.1.1.164&icpid=&accounttype=1&limitflux=-1&limitdur=-1&accountinfo=:20120224173239,019,110.1.1.59,20120224173239,10000100000000010000000000000012,8A93F0B8FC460A07C2D5655FFCF74443,,1,0,-1,1,1,btv,59d7d9c5dfb546609f39,9,1,END",
*                     mediaCode: "jsoncode1",mediaType:2,audioType:1,videoType:1,streamType:1,drmType:1,fingerPrint:0,
*                     copyProtection:1,allowTrickmode:1,startTime:0,endTime:20000,entryID:"jsonentry1"}]
*
*******************************************************/
Program *ProgramParser::parseSingleMedia(const char *media)
{
	struct json_object* list = NULL;
	struct json_object* object = NULL;
//	struct json_object* obj = NULL;
	char *param = (char *)media;
	int	obj_num = 0;

	ProgramVOD *m_vod = NULL;

	PROGRAM_LOG( "channel: %s\n", param );

	if('[' == *param){	//json array, eg. [{}] or [{},{}]
		int i = 0, count = 0 ;
		list = json_tokener_parse_string(param);

		if(list == NULL){
			PROGRAM_LOG( "invalid json: %s\n", param );
			return NULL;
		}

		obj_num = json_get_object_array_length(list);
		PROGRAM_LOG( "get json object[%d] . \n", obj_num);

		m_vod = new ProgramVOD();

		for(i = 0; i< obj_num; i++){
			object = json_object_get_array_idx(list, i);
			if(NULL == object){
				PROGRAM_LOG( "invalid json: %s\n", param );
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

	}else if('{' == *param){	// json object , eg. {}
		object = json_tokener_parse_string(param);
		if(object == NULL){
			PROGRAM_LOG( "invalid json: %s\n", param );
			return NULL;
		}
		m_vod = new ProgramVOD();
		m_vod->addVODSource(getVodSourceByJsonObj(object));
		json_object_delete(object);
	}else{	// unvalid json
		PROGRAM_LOG( "invalid json: %s\n", param );
		return NULL;
	}

	//广告插播
	advertisementList().advertisingSpots(m_vod);

	return m_vod;
}

/************************************************************
* function
* input: list
*        mediaList  [{mediaUrl:"rtsp://110.1.1.164/88888888/16/20120202/268435465/268435465.ts?rrsip=110.1.1.164&icpid=&accounttype=1&limitflux=-1&limitdur=-1&accountinfo=:20120322120857,112,110.1.1.67,20120322120857,10000100000000010000000000000003,37C4616875648641EDD054255764EAF0,,1,10,,6,1,0,0,9,1,END",
                      mediaCode:"code1",mediaType:2,audioType:1,videoType:1,streamType:1,drmType:1,fingerPrint:0,
                      copyProtection:1,allowTrickmode:1,startTime:10,endTime:110,entryID:"entry0"},
                     {mediaUrl:"rtsp://110.1.1.164/88888888/16/20120202/268435462/268435462.ts?rrsip=110.1.1.164&icpid=&accounttype=1&limitflux=-1&limitdur=-1&accountinfo=:20120322120857,112,110.1.1.67,20120322120857,10000100000000010000000000000005,37C4616875648641EDD054255764EAF0,,1,10,,5,1,0,0,9,1,END",
                      mediaCode:"code1",mediaType:2,audioType:1,videoType:1,streamType:1,drmType:1,fingerPrint:0,
                      copyProtection:1,allowTrickmode:1,startTime:20,endTime:120,entryID:"entry1"}]
************************************************************/
int
ProgramParser::parseMediaList(ProgramList *pProgramList, const char *mediaList)
{
    struct json_object* list = NULL;
    struct json_object* object = NULL;
//    struct json_object* obj = NULL;
    int	obj_num = 0;

    ProgramVOD *m_vod = NULL;

    if(NULL == pProgramList || NULL == mediaList) {
        PROGRAM_LOG("ERROR : null pointer list:%p, mediaList:%p\n", list, mediaList);
        return -1;
    }

    PROGRAM_LOG( "medialist: %s\n", mediaList );

    if('[' == *mediaList){	//json array, eg. [{}] or [{},{}]
        int i = 0, count = 0 ;
        list = json_tokener_parse_string(mediaList);

        if(list == NULL) {
            PROGRAM_LOG( "invalid json: %s\n", mediaList );
            return -1;
        }

        obj_num = json_get_object_array_length(list);
        PROGRAM_LOG( "get json object[%d] . \n", obj_num);

        for(i = 0; i< obj_num; i++) {
            object = json_object_get_array_idx(list, i);
            if(NULL == object) {
                 PROGRAM_LOG( "invalid json: %s\n", mediaList );
                 count++;
                 continue;
            }
            m_vod = new ProgramVOD();
            m_vod->addVODSource(getVodSourceByJsonObj(object));
            pProgramList->addProgram(m_vod);
        }
        json_object_delete(list);

    }else if('{' == *mediaList) {    // json object , eg. {}

        object = json_tokener_parse_string(mediaList);
        if(object == NULL){
            PROGRAM_LOG( "invalid json: %s\n", mediaList );
            return -1;
        }

        m_vod = new ProgramVOD();
        m_vod->addVODSource(getVodSourceByJsonObj(object));
        pProgramList->addProgram(m_vod);
        json_object_delete(object);

    }else {    // unvalid json
        PROGRAM_LOG( "invalid json: %s\n", mediaList );
        return -1;
    }
    return 0;
}

Program *
ProgramParser::parseSingleChannel(const char *media)
{
    return 0;
}

int
ProgramParser::parseChannelList(ProgramList *list, const char *mediaList)
{
    return -1;
}

} // namespace Hippo
