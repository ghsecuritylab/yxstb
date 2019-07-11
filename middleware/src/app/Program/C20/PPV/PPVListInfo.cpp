
#include "PPVListInfo.h"
#include "PPVProgram.h"
#include "HttpDataSource.h"
#include "RingBuffer.h"
#include "BrowserEventQueue.h"
#include "SystemManager.h"
#include "UltraPlayer.h"
#include "UltraPlayerClient.h"
#include "ProgramParser.h"
#include "ProgramParserC20.h"

#include "AppSetting.h"

#include "mid/mid_tools.h"
#include "mid/mid_time.h"

#include "json/json.h"
#include "json/json_object.h"
#include "json/json_public.h"

#include <vector>

namespace Hippo {

PPVListInfo::PPVListInfo()
    : m_isParsing(false)
{

}

PPVListInfo::~PPVListInfo()
{

}


bool
PPVListInfo::onDataArrive()
{
    return sendEmptyMessage(PPV_DataArrive);
}

bool
PPVListInfo::onError()
{
    return sendEmptyMessage(PPV_Error);
}

bool
PPVListInfo::onEnd()
{
    return sendEmptyMessage(PPV_End);
}

void
PPVListInfo::receiveData()
{
    char* buf = NULL;
    while (1) {
        uint8_t* bufPointer;
        uint32_t bufLength;

        m_ringBuffer->getReadHead(&bufPointer, &bufLength);
        if (bufLength == 0)
           break;
        buf = (char*)malloc(bufLength + 4);
        memset(buf, 0, bufLength + 4);
        memcpy(buf, bufPointer, bufLength);
        m_PPVProgram += buf;
        free(buf);
        m_ringBuffer->submitRead(bufPointer, bufLength);
    }
}

void
PPVListInfo::receiveError()
{
    free(m_memory);
    delete m_PPVSource;
    delete m_ringBuffer;
    m_isParsing = false;

}

void
PPVListInfo::receiveEnd()
{
    parsePPVProgram();

    free(m_memory);
    delete m_PPVSource;
    delete m_ringBuffer;
    m_isParsing = false;

}

bool
PPVListInfo::addPPVProgram(PPVProgram* program)
{
    if (!program)
        return false;

    program->m_upgradeFlag = true;
    m_ppvList.insert(std::make_pair(program->chanID(), program));

    return true;
}

void
PPVListInfo::parseAuthorizedProgram(PPVProgram* ppvProgram, std::string authorized)
{
    std::string residualInfo = authorized;
    std::string programInfo;
    std::size_t position = 0;
    std::size_t s1,s2,s3,s4;
    PPVAuthorizedProgram* program = 0;
    int authorizedNum = 0;

    while(1) {
        if (position == authorized.length())
            break;
        position = residualInfo.find_first_of(';');
        if (position == string::npos)
            break;
        programInfo = residualInfo.substr(0, position);
        if (programInfo.empty())
            break;
        s1 = programInfo.find(":");
        std::string programid = programInfo.substr(0, s1);
        s2 = programInfo.find("-");
        s3 = programInfo.find_first_of(',');
        std::string stopTime = programInfo.substr(s2 + 1, s3 - 1);
        int stopTimeNum = mid_tool_string2time((char*)(stopTime.c_str()));
        s4 = programInfo.find_last_of(',');
        residualInfo = residualInfo.substr(position + 1, authorized.length());
        if (stopTimeNum <= mid_time())
            continue;

        program = ppvProgram->findPPVAuthorizedProgram(programid);
        if (!program) {
            program = new PPVAuthorizedProgram();
            program->m_programID = programid;
            program->m_upgradeFlag = false;
        }
        program->m_startTime = programInfo.substr(s1 + 1, s2 - 1);
        program->m_startTimeNum = mid_tool_string2time((char*)(program->m_startTime.c_str()));
        program->m_stopTime = stopTime;
        program->m_stopTimeNum = stopTimeNum;
        program->m_status = atoi(programInfo.substr(s3 + 1, s4 - 1).c_str());
        program->m_programCode = programInfo.substr(s4 + 1, position - 1);
        if (!program->m_upgradeFlag)
            ppvProgram->addAuthorizedProgram(program);
    }
}

/*************************************************
Description:解析ppv频道列表
eg.
ChannelVer=20100724185402
ChannelNum=1
ChanID=105
authorizedProgram=9576:20100714091000-20100714091900,1,1111;9577:20100714092000-20100714092900,1,1112;9578:20100714093000-20100714093900,1,1113;9579:20100714094000-20100714094900,1,1114;12562:20100720115000-20100720115900,1,1115;
[END]
 *************************************************/
void
PPVListInfo::parsePPVProgram()
{
    vector<std::string> vLine;
    vector<std::string>::iterator iteratorIt, iteratorKey, iteratorValue;
    PPVProgram* ppvProgramNode = 0;

    if (m_PPVProgram.empty())
        return;

    //ProgramParser parser = programParser();
    //((ProgramParserC20)programParser()).ChannelListTokenize(m_PPVProgram, vLine);
    iteratorIt = vLine.begin();
    while(iteratorIt < vLine.end()){
        iteratorKey = iteratorIt;
        if (iteratorIt + 1 < vLine.end()) {
            if(!iteratorKey->compare("END")) {
                if (ppvProgramNode && !ppvProgramNode->m_upgradeFlag)
                    addPPVProgram(ppvProgramNode);
                break;
            } else if (!iteratorKey->compare("ChannelVer")) {
                m_PPVVersion = *iteratorValue;
            } else if (!iteratorKey->compare("ChannelNum")) {
                ;
            } else if (!iteratorKey->compare("ChanID")) {
                if (ppvProgramNode && !ppvProgramNode->m_upgradeFlag)
                    addPPVProgram(ppvProgramNode);
                iteratorValue = iteratorIt + 1;
                std::map<int, PPVProgram*>::iterator it = m_ppvList.find(atoi(iteratorValue->c_str()));
                if (it != m_ppvList.end())
                    ppvProgramNode = it->second;
                else {
                    ppvProgramNode = new PPVProgram();
                    ppvProgramNode->setChanID(atoi(iteratorValue->c_str()));
                }
            } else if (!iteratorKey->compare("authorizedProgram")) {
                parseAuthorizedProgram(ppvProgramNode, *iteratorValue);
            }
        }
    }
    if (!m_ppvList.empty())
        sendEmptyMessage(PPV_Reminder);

}

void
PPVListInfo::refreshPPVList()
{
    char requestURL[1024 + 4] = {0};

    if (m_isParsing) {
        sendEmptyMessageDelayed(PPV_Delay, 1000);
        return;
    }
    app_construct_url(requestURL, "EPG/jsp/getchannelppvlist.jsp");

    m_memory = (char*)malloc(1024);
    m_ringBuffer = new RingBuffer((uint8_t*)m_memory, 1024);

    m_PPVSource = new HttpDataSource();
    m_PPVSource->SetRequestUrl(requestURL);
    m_PPVSource->setBuffer(m_ringBuffer);
    m_PPVSource->attachSink(this);
    m_PPVSource->start();
    m_isParsing = true;
}

void
PPVListInfo::handleMessage(Message* msg)
{
    switch (msg->what) {
    case PPV_Delay:
        refreshPPVList();
        break;
    case PPV_Reminder:
        PPVReminder();
        break;
    case PPV_DataArrive:
        receiveData();
        break;
    case PPV_End:
        receiveEnd();
        break;
    case PPV_Error:
        receiveError();
        break;
    default:
        break;
    }

}

static void buildPPVEvent(const char* eventType, int instanceID, const char* message)
{
    json_object* jsonInfo = NULL;
    jsonInfo = json_object_create_object();
    if (!jsonInfo)
        return;

    json_object_object_add(jsonInfo, "type", json_object_new_string(eventType));
    json_object_object_add(jsonInfo, "instance_id", json_object_new_int(instanceID));
    json_object_object_add(jsonInfo, "message", json_object_new_string(message));
    browserEventSend(json_object_to_json_string(jsonInfo), NULL);
    json_object_delete(jsonInfo);
}

static ProgramChannelC20* getChannelProgramByChannelID(int channelId)
{
	Hippo::ProgramChannelC20 *program = NULL;
	Hippo::SystemManager &sysManager = Hippo::systemManager();

	program = (Hippo::ProgramChannelC20 *)(sysManager.channelList().getProgramByNumberID(channelId));

	return program;
}

void
PPVListInfo::PPVReminder()
{
    if (m_ppvList.empty())
        return;

    char ntvUser[32] = {0};
    char message[32 * 4] = {0};
    int instanceID = -1;

    SystemManager &sysManager = systemManager();
    UltraPlayer *player = sysManager.obtainMainPlayer();
    if (player)
        instanceID =  player->GetPlayerClient()->instanceId();
    sysManager.releaseMainPlayer(player);

    sendEmptyMessageDelayed(PPV_Reminder, 60 * 1000);
    appSettingGetString("ntvuser", ntvUser, 32, 0);
    std::map<int, PPVProgram*>::iterator it;
    for (it = m_ppvList.begin(); it != m_ppvList.end(); it++) {
        PPVProgram *program = it->second;
        PPVAuthorizedProgram* authorized = 0;
        authorized = program->getPPVProgram(mid_time(), PPVProgram::PPV_Start);
        if (authorized) {
            ProgramChannelC20* channelProgram = getChannelProgramByChannelID(program->m_chanID);
            sprintf(message, "%s,%s,%s,%s,%d,%s,%s",
                ntvUser, authorized->m_programID.c_str(), authorized->m_startTime.c_str(), program->m_chanID,
                channelProgram->GetChanKey(), channelProgram->GetChanName().c_str(), "");
            buildPPVEvent("EVENT_PPV_START", instanceID, message);
        }
        authorized = program->getPPVProgram(mid_time(), PPVProgram::PPV_End);
        if (authorized) {
            ProgramChannelC20* channelProgram = getChannelProgramByChannelID(program->m_chanID);
            sprintf(message, "%s,%s,%s,%s,%d,%s,%s",
                ntvUser, authorized->m_programID.c_str(), authorized->m_startTime.c_str(), program->m_chanID,
                channelProgram->GetChanKey(), channelProgram->GetChanName().c_str(), "");
            buildPPVEvent("EVENT_PPV_REMINDER_END", instanceID, message);
            program->removeAuthorizedProgram(authorized->m_programID);
            if (!program->hasAuthorizedProgram()) {
                delete program;
                m_ppvList.erase(it);
            }
        }
    }
}

PPVAuthorizedProgram*
PPVListInfo::checkPPVChannel(int channelID)
{
    if (m_ppvList.empty())
        return 0;

    PPVProgram* program = 0;
    std::map<int, PPVProgram*>::iterator it = m_ppvList.find(channelID);
    if (it == m_ppvList.end())
        return 0;
    program = it->second;

    return program->getPPVProgram(mid_time(), PPVProgram::PPV_Range);
}

static PPVListInfo* g_PPVListInfo = 0;

PPVListInfo* ppvListInfo()
{
    return g_PPVListInfo;
}

}

extern "C" void PPVListInfoCreate()
{
    Hippo::g_PPVListInfo = new Hippo::PPVListInfo();
}
