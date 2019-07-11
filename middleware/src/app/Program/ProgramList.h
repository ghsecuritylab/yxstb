#ifndef _ProgramList_H_
#define _ProgramList_H_

#include <map>
#include <vector>
#include <string>

#include "RefCnt.h"

#ifdef __cplusplus

namespace Hippo {

class Program;

class ProgramList : public RefCnt{
public:
    ProgramList();
    ~ProgramList();

    virtual int addProgram(Program *program);
    virtual int addProgramToPosition(Program *program, int position);

    virtual int getProgramIndex(Program *program);

    virtual int moveToFirst(Program *program);
    virtual int moveToLast(Program *program);
    virtual int moveToPrevious(Program *program);
    virtual int moveToNext(Program *program);
    virtual int moveToPosition(Program *program, int position);

    virtual int removeProgram(Program *program);
    virtual int removeProgramByIndex(int index);
    virtual int removeProgramByNumberID(int id);
    virtual int removeProgramByStringID(const char *stringID);

    virtual Program *getProgramByIndex(int index);
    virtual Program *getProgramByNumberID(int id);
    virtual Program *getProgramByStringID(const char *stringID);
    virtual Program *getProgramByNumberKey(int key);

    int getProgramCount();
    int clearAndRelease();

private:
	std::vector<Program*> mProgramArray;
    std::map<int, Program*> mNumberIDMap;
    std::map<int, Program*> mNumberKeyMap;
    std::map<std::string, Program*> mStringIDMap;
};

ProgramList *programList();

} // namespace Hippo

#endif // __cplusplus

#endif // _ProgramList_H_
