
#include "ProgramList.h"
#include "ProgramAssertions.h"
#include "Program.h"


namespace Hippo {

ProgramList::ProgramList()
{
}

ProgramList::~ProgramList()
{
}

int 
ProgramList::addProgram(Program *program)
{
    if (program == 0)
        return -1;

    mProgramArray.push_back(program);

    mStringIDMap.insert(std::make_pair(program->getStringID(), program));
    if (program->getNumberID() != -1)
        mNumberIDMap.insert(std::make_pair(program->getNumberID(), program));
    if (program->GetChanKey() != -1)
        mNumberKeyMap.insert(std::make_pair(program->GetChanKey(), program));
    
    return 0;
}

int 
ProgramList::addProgramToPosition(Program *program, int position)
{
    if (program == 0 || position < 0)
        return -1;

    if (position > mProgramArray.size())
        position = mProgramArray.size();

    std::vector<Program*>::iterator it = mProgramArray.begin();
    it += position;
    mProgramArray.insert(it, 1, program);

    mStringIDMap.insert(std::make_pair(program->getStringID(), program));
    if (program->getNumberID() != -1)
        mNumberIDMap.insert(std::make_pair(program->getNumberID(), program));
    if (program->GetChanKey() != -1)
        mNumberKeyMap.insert(std::make_pair(program->GetChanKey(), program));

    return 0;
}

int 
ProgramList::getProgramIndex(Program *program)
{
    int i;

    for (i = 0; i < mProgramArray.size(); i++) {
        if (program == mProgramArray[i])
            return i;
    }

    return -1;
}

int 
ProgramList::moveToFirst(Program *program)
{
    if (program == 0)
        return -1;

    std::vector<Program*>::iterator it;

    for (it = mProgramArray.begin(); it != mProgramArray.end(); ++it) {
        if (program == *it) {
            mProgramArray.erase(it);
            mProgramArray.insert(mProgramArray.begin(), 1, program);
            return 0;
        }
    }

    return -1;
}

int 
ProgramList::moveToLast(Program *program)
{
    if (program == 0)
        return -1;

    std::vector<Program*>::iterator it;

    for (it = mProgramArray.begin(); it != mProgramArray.end(); ++it) {
        if (program == *it) {
            mProgramArray.erase(it);
            mProgramArray.insert(mProgramArray.end(), 1, program);
            return 0;
        }
    }

    return -1;
}

int 
ProgramList::moveToPrevious(Program *program)
{
    if (program == 0)
        return -1;

    if (program == mProgramArray[0])
        return 0;

    int i;
    for (i = 0; i < mProgramArray.size(); i++) {
        if (program == mProgramArray[i]) {
            mProgramArray[i] = mProgramArray[i - 1];
            mProgramArray[i - 1] = program;
            return 0;
        }
    }

    return -1;
}

int 
ProgramList::moveToNext(Program *program)
{
    if (program == 0)
        return -1;

    int i = mProgramArray.size() - 1;
    if (program == mProgramArray[i])
        return 0;

    for (i = 0; i < mProgramArray.size(); i++) {
        if (program == mProgramArray[i]) {
            mProgramArray[i] = mProgramArray[i + 1];
            mProgramArray[i + 1] = program;
            return 0;
        }
    }

    return -1;
}

int 
ProgramList::moveToPosition(Program *program, int position)
{
    if (program == 0 || position < 0)
        return -1;

    if (position >= mProgramArray.size())
        position = mProgramArray.size() - 1;

    std::vector<Program*>::iterator it;
    int i = 0;

    for (it = mProgramArray.begin(); it != mProgramArray.end(); ++it, ++i) {
        if (program == *it) {
            if (position == i)
                return 0;
            mProgramArray.erase(it);
            break;
        }
    }

    it = mProgramArray.begin() + position;
    mProgramArray.insert(it, 1, program);

    return 0;
}

int 
ProgramList::removeProgram(Program *program)
{
    if (program == 0)
        return -1;

    std::vector<Program*>::iterator it;

    for (it = mProgramArray.begin(); it != mProgramArray.end(); ++it) {
        PROGRAM_LOG("find id(%d) delete id(%d)\n", ((Program *)*it)->getNumberID(), program->getNumberID());
        if (program == *it) {
            PROGRAM_LOG("delete program address(%p)\n", program);
            mNumberIDMap.erase(program->getNumberID());
            mStringIDMap.erase(program->getStringID());
            mNumberKeyMap.erase(program->GetChanKey());
            mProgramArray.erase(it);
            break;
        }
    }
    PROGRAM_LOG("Current channel count(%d)\n", mProgramArray.size());
    return 0;
}

int 
ProgramList::removeProgramByIndex(int index)
{
    Program *program = getProgramByIndex(index);
    return removeProgram(program);
}

int 
ProgramList::removeProgramByNumberID(int id)
{
    Program *program = getProgramByNumberID(id);

    if(program)
        PROGRAM_LOG("Remove program address(%p) UserChanID(%d)\n", program, program->getNumberID());
    return removeProgram(program);
}

int 
ProgramList::removeProgramByStringID(const char *stringID)
{
    Program *program = getProgramByStringID(stringID);
    return removeProgram(program);
}

Program *
ProgramList::getProgramByIndex(int index)
{
    if (index < 0 || index >= mProgramArray.size())
        return 0;
    return mProgramArray[index];
}

Program *
ProgramList::getProgramByNumberID(int id)
{
    std::map<int, Program*>::iterator it = mNumberIDMap.find(id);

    if (it != mNumberIDMap.end())
        return it->second;
    else
        return 0;
}

Program *
ProgramList::getProgramByStringID(const char *stringID)
{
    if (stringID == 0)
        return 0;

    std::map<std::string, Program*>::iterator it = mStringIDMap.find(stringID);

    if (it != mStringIDMap.end())
        return it->second;
    else
        return 0;
}

Program *
ProgramList::getProgramByNumberKey(int key)
{
    std::map<int, Program*>::iterator it = mNumberKeyMap.find(key);

    if (it != mNumberKeyMap.end())
        return it->second;
    else
        return 0;
}


int 
ProgramList::getProgramCount()
{
    return mProgramArray.size();
}

int 
ProgramList::clearAndRelease()
{
    mNumberIDMap.clear();
    mStringIDMap.clear();
    mNumberKeyMap.clear();
    std::vector<Program*>::iterator it;
    for (it = mProgramArray.begin(); it != mProgramArray.end(); ++it) {
        (*it)->unref();
    }

    mProgramArray.clear();
    return 0;
}

} // namespace Hippo
