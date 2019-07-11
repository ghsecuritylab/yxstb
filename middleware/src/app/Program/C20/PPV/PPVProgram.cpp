#include "PPVProgram.h"

#include "mid/mid_tools.h"

#include <string.h>

namespace Hippo {

PPVProgram::PPVProgram()
    : m_chanID(-1)
    , m_upgradeFlag(false)
{

}

PPVProgram::~PPVProgram()
{

}

PPVAuthorizedProgram*
PPVProgram::findPPVAuthorizedProgram(std::string programID)
{
    std::map<std::string, PPVAuthorizedProgram*>::iterator it = m_Authorized.find(programID);
    if (it == m_Authorized.end())
        return 0;

    return it->second;

}

bool
PPVProgram::addAuthorizedProgram(PPVAuthorizedProgram* program)
{
    if (!program)
        return false;

    program->m_upgradeFlag = true;
    m_Authorized.insert(std::make_pair(program->m_programID, program));

    return true;
}

bool
PPVProgram::removeAuthorizedProgram(std::string programID)
{
    if (m_Authorized.empty())
        return false;

    std::map<std::string, PPVAuthorizedProgram*>::iterator it = m_Authorized.find(programID);

    if (it == m_Authorized.end())
        return false;

    PPVAuthorizedProgram *program = 0;
    program = it->second;
    delete program;
    m_Authorized.erase(it);

    return true;
}

PPVAuthorizedProgram*
PPVProgram::getPPVProgram(int time, int compareFlag)
{
    if (m_Authorized.empty())
        return 0;

    char strCurrentTime[16] = {0};
    mid_tool_time2string(time, strCurrentTime, 0);
    std::map<std::string, PPVAuthorizedProgram*>::iterator it;
    for(it = m_Authorized.begin(); it != m_Authorized.end(); ++it) {
        PPVAuthorizedProgram* program = 0;
        program = it->second;
        if (compareFlag == PPV_Range) {
            if ((time >= program->m_startTimeNum - 60)
                && time <= program->m_stopTimeNum)
                return program;
        } else if (compareFlag == PPV_Start) {
            if (!strncasecmp(strCurrentTime, program->m_startTime.c_str(), 12))
                return program;
        } else {
            if (!strncasecmp(strCurrentTime, program->m_stopTime.c_str(), 12))
                return program;
        }
    }

    return 0;
}

}