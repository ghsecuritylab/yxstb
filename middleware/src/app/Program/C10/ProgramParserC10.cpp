
#include "ProgramParserC10.h"
#include "ProgramChannelC10.h"
#include "ProgramAssertions.h"

#include "AppSetting.h"
#include "SysSetting.h"
#include "sys_basic_macro.h"
#include "app_sys.h"
#include "Session.h"

extern "C" {
#include "mid/mid_tools.h"
#include "mid_stream.h"
}

#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;
namespace Hippo {

static ProgramParserC10 gProgramParser;


ProgramParserC10::ProgramParserC10()
{
}

ProgramParserC10::~ProgramParserC10()
{
}
/*******************************************************************************************
function
input aFieldValue: eg. ChannelID="5",ChannelName="CCTV1",UserChannelID="5",ChannelURL="igmp://238.255.100.9:10099|rtsp://110.1.1.164/PLTV/88888888/224/3221225481/10000100000000060000000000000015_0.smil?rrsip=110.1.1.164&icpid=&accounttype=1&limitflux=-1&limitdur=-1&accountinfo=:20120222161818,019,110.1.1.59,20120222161818,10000100000000050000000000000016,C1C9F46A91702BEB0ACD5191442DCDE0,-1,0,3,-1,,2,,,,2,END",
                        TimeShift="0",TimeShiftLength="0",ChannelSDP="igmp://238.255.100.9:10099|rtsp://110.1.1.164/PLTV/88888888/224/3221225481/10000100000000060000000000000015_0.smil?rrsip=110.1.1.164&icpid=&accounttype=1&limitflux=-1&limitdur=-1&accountinfo=:20120222161818,019,110.1.1.59,20120222161818,10000100000000050000000000000016,C1C9F46A91702BEB0ACD5191442DCDE0,-1,0,3,-1,,2,,,,2,END",
                        TimeShiftURL="rtsp://110.1.1.164/PLTV/88888888/224/3221225481/10000100000000060000000000000015_0.smil?rrsip=110.1.1.164&icpid=&accounttype=1&limitflux=-1&limitdur=-1&accountinfo=:20120222161818,019,110.1.1.59,20120222161818,10000100000000050000000000000016,C1C9F46A91702BEB0ACD5191442DCDE0,-1,0,3,-1,,7,,,,4,END",
                        ChannelType="1",IsHDChannel="2",PreviewEnable="1",ChannelPurchased="1",ChannelLocked="0",ChannelLogURL="",
                        PositionX="null",PositionY="null",BeginTime="null",Interval="null",Lasting="null",ActionType="1",FCCEnable="0"

********************************************************************************************/
Program *
ProgramParserC10::parseSingleChannel(const char *media)
{
    static bool rrsFlag = false;
	
    if (NULL == media) {
        PROGRAM_LOG_ERROR("Media is NULL\n");
        return NULL;
    }

    ProgramChannelC10 *pNode = NULL;
    pNode = new ProgramChannelC10();

    if(NULL == pNode) {
        PROGRAM_LOG_ERROR("Can't create ProgramChannelC10 instant \n");
        return NULL;
    }
    std::string sMediaInfo = media;
    vector<std::string> key, value;
    unsigned int i = 0;
    vector<std::string>::iterator keyIt, valueIt;

    //PROGRAM_LOG( "channel: %s\n", media);

    ProgramParser::tokenize(sMediaInfo, key, value );
    keyIt = key.begin( );
    valueIt = value.begin( );

    for( i=0; i<key.size() && i<value.size(); ++i ) {
        if(!strncasecmp(key[i].c_str(), "ChannelID", 9 ) ) {
            pNode->SetChanID(value[i]);
        }
        else if(!strncasecmp(key[i].c_str(), "ChannelName", 11 ) ) {
            pNode->SetChanName(value[i]);
        }
        else if(!strncasecmp(key[i].c_str(), "ChannelURL", 10 ) ) {
            pNode->SetChanURL(value[i]);
        }
        else if(!strncasecmp(key[i].c_str(), "TimeShiftURL", 12 ) ) {
            pNode->SetTimeShiftURL(value[i]);
            if (!rrsFlag) {				
                char *p = NULL; 
                int port = 0;
                unsigned int rrsAddr =0;			
                char rrs_ip[64 + 4] = {0};			
                if ((p = strstr((char *)value[i].c_str(), "rtsp://")) != NULL) {
                    if (0 == mid_tool_check_RTSPURL(p, rrs_ip)) {
                        sysSettingGetInt("fcc_port", &port, 0);
                        rrsAddr = htonl(inet_addr(rrs_ip));				
                        mid_stream_setRRS(rrsAddr, port);				
                        rrsFlag = true;
                    }					
                }
            }			
        }
        else if(!strncasecmp(key[i].c_str(), "ChannelSDP", 10 ) ) {
            pNode->SetChanSDP(value[i]);
        }
        else if(!strncasecmp(key[i].c_str(), "ChannelLogURL", 13 ) ) {
            pNode->SetLogoURL(value[i]);
        }
        else if(!strncasecmp(key[i].c_str(), "ChannelLogoURL", 14 ) ) {
            pNode->SetLogoURL(value[i]);
        }
        else if(!strncasecmp(key[i].c_str(), "UserChannelID", 13 ) ) {
            pNode->SetUserChanID(atoi(value[i].c_str()));
        }
        else if(!strncasecmp(key[i].c_str(), "TimeShiftLength", 15 ) ) {
            pNode->SetTimeShiftLength(atoi(value[i].c_str()));
        }
        else if(!strncasecmp(key[i].c_str(), "TimeShift", 9 ) ) {
            pNode->SetTimeShift(atoi(value[i].c_str()));
        }
        else if(!strncasecmp(key[i].c_str(), "ChannelType", 11 ) ) {
            if(session().getPlatform() == PLATFORM_ZTE) {
#if 0 //#if defined(SHANGHAI_HD)||defined(SHANGHAI_SD)
                int ct = value[i].toInt32();
                if(ct == 2 || ct == 3)
                    ct = 5 - ct;
                pNode->SetChanType(ct);
#else
            	pNode->SetChanType(1);
#endif
            } else {
            	pNode->SetChanType(atoi(value[i].c_str()));
            }
        }
        else if(!strncasecmp(key[i].c_str(), "IsHDChannel", 11 ) ) {
            pNode->SetIsHD_Chan(atoi(value[i].c_str()));
        }
        else if(!strncasecmp(key[i].c_str(), "PreviewEnable", 13 ) ) {
            pNode->SetPrevEnable(atoi(value[i].c_str()));
        }
        else if(!strncasecmp(key[i].c_str(), "ChannelPurchased", 16 ) ) {
            pNode->SetChanPurchased(atoi(value[i].c_str()));
        }
        else if(!strncasecmp(key[i].c_str(), "ChannelLocked", 13 ) ) {
            pNode->SetChanLocked(atoi(value[i].c_str()));
        }
        else if(!strncasecmp(key[i].c_str(), "PositionX", 9 ) ) {
            pNode->SetLogoXPos(atoi(value[i].c_str()));
        }
        else if(!strncasecmp(key[i].c_str(), "PositionY", 9 ) ) {
            pNode->SetLogoYPos(atoi(value[i].c_str()));
        }
        else if(!strncasecmp(key[i].c_str(), "BeginTime", 9 ) ) { // BeginTime: Logo show begin time second from channel play start.
            pNode->SetBeginTime(atoi(value[i].c_str()));
        }
        else if(!strncasecmp(key[i].c_str(), "Interval", 8 ) ) { // Interval: -1 is show alway, 0 is show one time
            pNode->SetInterval(atoi(value[i].c_str()));
        }
        else if(!strncasecmp(key[i].c_str(), "Lasting", 7 ) ) { // Lasting: Logo show times from begin.
            pNode->SetLasting(atoi(value[i].c_str()));
        }
        else if(!strncasecmp(key[i].c_str(), "ActionType", 10 ) ) {
            pNode->SetActionType(atoi(value[i].c_str()));
        }
        else if(!strncasecmp(key[i].c_str(), "ChannelFECPort", 14 ) ) {
            pNode->SetChannelFECPort(atoi(value[i].c_str()));;
        }
        else if(!strncasecmp(key[i].c_str(), "ActionType", 10 ) ) {
            pNode->SetActionType(atoi(value[i].c_str()));
        }
        else if(!strncasecmp(key[i].c_str(), "FCCEnable", 9 ) ) {
            pNode->SetFCC_Enable(atoi(value[i].c_str()));
        }
        else if(!strncasecmp(key[i].c_str(), "FECEnable", 9 ) ) {
            pNode->SetFEC_Enable(atoi(value[i].c_str()));
        }
        else if(!strncasecmp(key[i].c_str(), "CanUnicast", 10 ) ) {
            pNode->SetCanUnicast(atoi(value[i].c_str()));
        }
    }

    if((pNode->GetCanUnicast() == 1)
         && (!strncmp(pNode->GetChanURL().c_str(), "igmp://", 7)
         && (!strstr(pNode->GetChanURL().c_str(), "rtsp://")
         && (!pNode->GetTimeShiftURL().empty())))) {

        pNode->GetChanURL() += '|';
        pNode->GetChanURL() += pNode->GetTimeShiftURL();
    }

#ifdef INCLUDE_SQA
//FEC+RET
/*  华为文档中没有fec 开启的标志位，默认情况下只要是组播就要开启sqa 模块  */
    int sqa_code = 0;
    int ret_code = 1;
	int FEC_Enable = 0;
    int FCC_Enable = pNode->GetFCC_Enable();
	int FEC_Port = pNode->GetChannelFECPort();
	int fccSwitch = 0;

    appSettingGetInt("fcc_switch", &fccSwitch, 0);

	if(FEC_Port > 0) {
		FEC_Enable = 1;
	}

    if( 1 == fccSwitch ) {
        switch(FCC_Enable){
            case 0: // open fec only
                if(FEC_Enable != 0)
                    sqa_code |= SQA_FEC_CODE;
                else
                    PROGRAM_LOG_WARNING("fec don not have channelfecport in channel url \n");
                PROGRAM_LOG("Set SQA sqa_code(%d), ret_code(%d)\n",sqa_code, ret_code);
                break;

            case 1: // open fcc+ret+fec
                sqa_code |= SQA_FCC_CODE;
                sqa_code |= SQA_RET_CODE;
                if(FEC_Enable != 0)
                    sqa_code |= SQA_FEC_CODE;
                else
                    PROGRAM_LOG_WARNING("fec don not have channelfecport in channel url \n");
                PROGRAM_LOG("Set SQA sqa_code(%d), ret_code(%d)\n",sqa_code, ret_code);
                break;

            case 2:	//open fcc+fec
                sqa_code |= SQA_FCC_CODE;
                if(FEC_Enable != 0)
                    sqa_code |= SQA_FEC_CODE;
                else
                    PROGRAM_LOG_WARNING("fec don not have channelfecport in channel url \n");
                PROGRAM_LOG("Set SQA sqa_code(%d), ret_code(%d)\n",sqa_code, ret_code);
                break;

            case 3:
                sqa_code |= SQA_RET_CODE;
                if(FEC_Enable != 0)
                    sqa_code |= SQA_FEC_CODE;
                else
                    PROGRAM_LOG_WARNING("fec don not have channelfecport in channel url \n");
                PROGRAM_LOG("Set SQA sqa_code(%d), ret_code(%d)\n",sqa_code, ret_code);
				break;
        }
    } else {
		sqa_code = 0;
		ret_code = 0;
	}
    pNode->SetSqaCode(sqa_code);
    pNode->SetRetCode(ret_code);

#endif
    m_parseChannellistOK = 1;

    return pNode;
}

ProgramParser &programParser()
{
    return gProgramParser;
}

} // namespace Hippo
