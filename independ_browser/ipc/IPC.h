#ifndef _IPC_H_
#define _IPC_H_

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <errno.h>
#include <stdint.h> //uint16_t
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <string>

#include "IPCAssertions.h"

#define BROWSER_ROOT_RUN
// #define DEBUG_BROWSER
// #define IPC_DEBUG_MODE


#ifdef _SEM_SEMUN_UNDEFINED
/* Union used for argument for `semctl'.  */
union semun {
    int val;                   /* value  for SETVAL              */
    struct semid_ds *buf;      /* buffer for IPC_STAT & IPC_SET  */
    unsigned short int *array; /* array  for GETALL & SETALL     */
    struct seminfo *__buf;     /* buffer for IPC_INFO for Linux    */
};
#endif

typedef enum _ParamType_ { // min(0) - max(255)
    eTypeUnkown    = 0,
    eTypeChar         ,
    eTypeCharArray    , 
    eTypeInt          , 
    eTypeIntArray     , 
    eTypeLong         ,
    eTypeLongArray    ,
    eTypeDouble       , 
    eTypeDoubleArray  ,
    eTypeFloat        ,
    eTypeStruct       ,
}ParamType_t;

typedef enum _DataTag_ { // min(0) - max(255)
    eTagNoUsed = 0,
    eTagArgName   ,
    eTagArgValue  ,
    eTagFuntion   ,
    eTagObject    ,
}DataTag_t;

enum StackPurpose {
	SP_Call = 10,
	SP_Result,
};

typedef struct _ShmHeader_ {
    uint32_t u32StackSequence; //use as debug
    uint16_t u16StackPurpose;
    uint16_t u16StackItemNums;
    uint16_t u16StackOffset;
    uint16_t u16StackLength;
}ShmHeader_t;

typedef struct _StackItem_ {
    DataTag_t u8Tag:8; //token: param value function obj
    ParamType_t u8Type:8;
    uint16_t u16Size;
}StackItem_t;

#define STRING(str) #str

#define LAYER_SHARE_KEY 0xabcdef99
#define SHM_MODE 0666 
#define SEM_MODE 0666


#ifdef __cplusplus

namespace Hippo {

/*-----------------------------------------------------------------------------
 *    OBJ: Caller --> Sevice
 *                                mShmID[0]
 *   push(c), push(b), push(a) |            |   pop(caller)
 *   push(function)            |------------|   pop(function)
 *   push(caller)              |   caller   |   pop(a), pop(b), pop(c)
 *                             |------------|
 *   caller->function(a,b,c)   |  function  |   sevice->function(a,b,c);
 *                             |------------|
 *  Caller ----------------->  |     a      |  -----------------> Sevice
 *    ^   invoke(mSemID[0])    |     b      |  request(mSemID[0])   |
 *    |                        |     c      |                       |
 *    |                        |------------|                       |
 *    |                        |            |                       |
 *    |                                                             |
 *    |                                                             |
 *    |                        |  mShmID[1] |      RETURN RESULT    |
 *    |                        |------------|                       |
 *    |         pop(ret)       |            |    push(ret)          |
 *    +----------------------  |    ret     |  <--------------------+
 *        request(mSemID[1])   |            |    invoke(mSemID[1])
 *                             |------------|
 *-----------------------------------------------------------------------------*/

class IPCServer;

class IPC {
public:

    typedef enum _StackState {
        eStackStateBuffSmall  = -105,
        eStackStateNotMatch   = -104,
        eStackStateIsFull     = -103,
        eStackStateIsEmpty    = -102,
        eStackStateError      = -101,
        eStackStateSuccess    = 0,
        eStackStateNullValue  = 100,
    }EStackState_t;

    IPC();
    ~IPC();

    int invoke();
    int wait(int block = 1);

    int initialize(int bind);
    int release() {_ReleaseSharedMemoryAndSemphore(0); _ReleaseSharedMemoryAndSemphore(1); return 0;}

    int initStack(int purpose, int sequence = 0);
    int freeStack(); //Debug

private:
    int _CreateSharedMemoryAndSemphore(int id);
    int _InitializeSharedMemoryHeader(int id);
    int _ReleaseSharedMemoryAndSemphore(int id);

    int _PushArg(const char* argName, const void* argValue, int argLength);
    int _PopArg(const char* argName, void* argValue, int argLength);
protected:
    void _ShowStackData(void); //for Debug

    int pushArg(const char* argName, const char* argValue);
    int pushArg(const char* argName, const void* argValue, int argLength);
    int pushArg(const char* argName, int argValue);
    int pushArg(const char* argName, unsigned int argValue);
    int pushArg(const char* argName, long argValue);
    int pushArg(const char* argName, float argValue);
    int pushArg(const char* argName, bool argValue);
    int pushArg(const char* argName, std::string& argValue);

    int popArg(const char* argName, void* argValue, int argLength);
    int popArg(const char* argName, unsigned char* argValue, int argLength);
    int popArg(const char* argName, int& argValue);
    int popArg(const char* argName, unsigned int& argValue);
    int popArg(const char* argName, long& argValue);
    int popArg(const char* argName, float& argValue);
    int popArg(const char* argName, bool& argValue);
    int popArg(const char* argName, std::string& argValue);

    bool isResult();
    bool isCall();
    bool isEmpty();

    void setProgramName(const char* name);
    static char mName[32]; //for debug
    static int mShmID[2];
    static int mSemID[2];
    static int mPushUseID;
    static int mPopUseID;
    static void* mBaseBuffer[2];

    static IPCServer* s_topServer;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _IPC_H_
