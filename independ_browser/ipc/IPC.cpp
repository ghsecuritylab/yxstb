#include "IPC.h"

#define SHM_METUX 

#define SHM_SIZE 10240  //size of shared memory

const int kShmKey[] = { 0xabcdef01, 0xabcdef03 }; // fix:abcdef[0-9][0-9]
const int kSemKey[] = { 0xabcdef02, 0xabcdef04 }; // fix:abcdef[0-9][0-9]

unsigned int kSharedMemHeaderSize = sizeof(ShmHeader_t);
unsigned int kStackItemHeaderSize = sizeof(StackItem_t);

namespace Hippo {

void* IPC::mBaseBuffer[] = {NULL, NULL};
int IPC::mShmID[] = {-1, -1};
int IPC::mSemID[] = {-1, -1};
int IPC::mPushUseID = -1;
int IPC::mPopUseID = -1;

char IPC::mName[32] = { 0 };

IPCServer* IPC::s_topServer = 0;

IPC::IPC()
{
}

IPC::~IPC()
{
}

/**
 * @brief initialize
 *
 * @param bind set the process push operation using shared memory id (0,1)
 *
 * @return 0
 */
int
IPC::initialize(int bind)
{
    if (-1 == mShmID[0] || -1 == mSemID[0])
        _CreateSharedMemoryAndSemphore(0);
    if (-1 == mShmID[1] || -1 == mSemID[1])
        _CreateSharedMemoryAndSemphore(1);
    mPushUseID = bind;
    mPopUseID = !bind;

    return 0;
}

/**
 * @brief _CreateSharedMemoryAndSemphore
 *      //////////////////////////////////////////////////////////////////////////////
 *        ________Shared Memory             ______________
 *           ^    |           |<---shmat();        ^
 *           |    |           |                    |
 *           |    |           |                    |
 *           |    |           |               StackLength
 *           |    |           |                    |
 *       SHM_SIZE |           |                    |
 *           |    |           |____________________v________
 *           |    |-----------|<---mBaseBuffer (StackOffset from mBaseBuffer)
 *           |    |StackOffset|\
 *           |    |StackLength| -> kSharedMemHeaderSize
 *        ___v____|-----------|/
 *
 *      /////////////////////////////////////////////////////////////////////////////
 *
 * @param id
 *
 * @return 0
 */
int
IPC::_CreateSharedMemoryAndSemphore(int id)
{
    //create shared memory
    mShmID[id] = shmget(kShmKey[id], SHM_SIZE, IPC_CREAT | IPC_EXCL | SHM_MODE);
    if (-1 == mShmID[id]) {
        if (errno == EEXIST) {
            mShmID[id] = shmget(kShmKey[id], 0, 0);
            mBaseBuffer[id] = (char*)shmat(mShmID[id], NULL, SHM_RND) + SHM_SIZE - kSharedMemHeaderSize;
        }
    } else {
        mBaseBuffer[id] = (char*)shmat(mShmID[id], NULL, SHM_RND) + SHM_SIZE - kSharedMemHeaderSize;
        _InitializeSharedMemoryHeader(id);
    }
    //create semphore
    mSemID[id] = semget(kSemKey[id], 2, IPC_CREAT | IPC_EXCL | SEM_MODE);
    if (-1 == mSemID[id]) {
        if (errno == EEXIST)
            mSemID[id] = semget(kSemKey[id], 0, 0);
    } else {
        semctl(mSemID[id], 0, SETVAL, 0); //init 0 resource for invoke
        semctl(mSemID[id], 1, SETVAL, 1); //init 1 resource for asynchronous
    }
    LogIpcDebug("ID[%d] shmKey[%02x] shmID[%d] semKey[%02x] semID[%d] create!\n", id, kShmKey[id], mShmID[id], kSemKey[id], mSemID[id]);
    return 0;
}

/**
 * @brief _InitializeSharedMemoryHeader
 *
 * @param id (0 or 1)
 *
 * @return 0
 */
int
IPC::_InitializeSharedMemoryHeader(int id)
{
    ShmHeader_t* pShmHeader = (ShmHeader_t*)mBaseBuffer[id];
    pShmHeader->u32StackSequence = 0;
    pShmHeader->u16StackItemNums = 0;
    pShmHeader->u16StackOffset   = 0;
    pShmHeader->u16StackLength   = SHM_SIZE - kSharedMemHeaderSize;
    return 0;
}

/**
 * @brief _ReleaseSharedMemoryAndSemphore
 *
 * @param  id (0 or 1)
 *
 * @return 0
 */
int
IPC::_ReleaseSharedMemoryAndSemphore(int id)
{
    if (-1 != mShmID[id]) {
        void* addr = shmat(mShmID[id], NULL, SHM_RND);
        if (addr)
            shmdt(addr);
        shmctl(mShmID[id], IPC_RMID, 0);
    }
    if (-1 != mSemID[id]) {
        semctl(mSemID[id], 0, IPC_RMID);
        semctl(mSemID[id], 1, IPC_RMID);
    }
    LogIpcDebug("ID[%d] shmID[%d] semID[%d] release!\n", id, mShmID[id], mSemID[id]);
    //TODO
    mShmID[id] = -1;
    mSemID[id] = -1;
    return 0;
}

void
IPC::_ShowStackData(void)
{
    ShmHeader_t* pShmHeader = (ShmHeader_t*)mBaseBuffer[mPopUseID];
    StackItem_t* pStkHeader = (StackItem_t*)((char*)pShmHeader - pShmHeader->u16StackOffset);

    int offset = pShmHeader->u16StackOffset;
    LogIpcTest("%s:ShmHeader<%u %u %u %u %u>\n", mName, pShmHeader->u32StackSequence, pShmHeader->u16StackPurpose, pShmHeader->u16StackItemNums, pShmHeader->u16StackOffset, pShmHeader->u16StackLength);
    for (int i = 1; i <= pShmHeader->u16StackItemNums; ++i) {
        pStkHeader = (StackItem_t*)((char*)pShmHeader - offset);
        LogIpcTest("###Item%d <%u %u %u>\n", i, pStkHeader->u8Tag, pStkHeader->u8Type, pStkHeader->u16Size);
        offset += kStackItemHeaderSize + pStkHeader->u16Size;
    }
    //sleep(100);
}

void
IPC::setProgramName(const char* name)
{
    strncpy(mName, name, 31);
}

int
IPC::initStack(int purpose, int sequence)
{
#if defined(SHM_METUX)
    static struct sembuf sBuf;
    sBuf.sem_num = 1;
    sBuf.sem_op = -1;
    sBuf.sem_flg = 0;
    semop(mSemID[mPushUseID], &sBuf, 1);  //will wait
#endif

    ShmHeader_t* pShmHeader = (ShmHeader_t*)mBaseBuffer[mPushUseID];
    pShmHeader->u32StackSequence = sequence;
    pShmHeader->u16StackPurpose  = purpose;
    pShmHeader->u16StackItemNums = 0;
    pShmHeader->u16StackOffset   = 0;
    pShmHeader->u16StackLength   = SHM_SIZE - kSharedMemHeaderSize;
    return 0;
}

int 
IPC::freeStack()
{
#if defined(SHM_METUX)
    static struct sembuf sBuf;
    sBuf.sem_num = 1;
    sBuf.sem_op = +1;
    sBuf.sem_flg = 0;
    semop(mSemID[mPushUseID], &sBuf, 1);
#endif
    return 0;
}

/**
 * @brief pushArg push function's parameter (value type is char*)
 *
 * @param argName  char*
 * @param argValue char*
 * @param argLength int
 *
 * @return nonnegative if ok (>=0)
 */
int
IPC::_PushArg(const char* argName, const void* argValue, int argLength)
{
    ShmHeader_t* const pShmHeader = (ShmHeader_t*)mBaseBuffer[mPushUseID];

    //Push-ArgValue
    StackItem_t item = { eTagArgValue, eTypeCharArray, argLength };
    pShmHeader->u16StackOffset += (kStackItemHeaderSize + item.u16Size);
    pShmHeader->u16StackItemNums += 1;
    if (pShmHeader->u16StackOffset > pShmHeader->u16StackLength) {
        pShmHeader->u16StackOffset -= (kStackItemHeaderSize + item.u16Size); //restore the orignal offset
        pShmHeader->u16StackItemNums -= 1;
        return eStackStateIsFull;
    }
    void* pEndItemHeader = mempcpy((char*)pShmHeader - pShmHeader->u16StackOffset, &item, kStackItemHeaderSize);
    if (item.u16Size > 0)
        memcpy(pEndItemHeader, argValue, item.u16Size);

#if defined(IPC_DEBUG_MODE)
    //Push-ArgName
    StackItem_t itemName = { eTagArgName, eTypeCharArray, strlen(argName) + 1 };
    pShmHeader->u16StackOffset += (kStackItemHeaderSize + itemName.u16Size);
    pShmHeader->u16StackItemNums += 1;
    if (pShmHeader->u16StackOffset > pShmHeader->u16StackLength) {
        pShmHeader->u16StackOffset -= (kStackItemHeaderSize + itemName.u16Size); //restore the orignal offset
        pShmHeader->u16StackItemNums -= 1;
        return eStackStateIsFull;
    }
    //push data into stack
    memcpy(mempcpy((char*)pShmHeader - pShmHeader->u16StackOffset, &itemName, kStackItemHeaderSize), argName, itemName.u16Size);
#endif
    return eStackStateSuccess;
}

int
IPC::pushArg(const char* argName, const void* argValue, int argLength)
{
    int ret = _PushArg(argName, argValue, argLength);
    if (ret == eStackStateSuccess)
        LogIpcStack("name[%s] argLength[%d]\n", argName, argLength);
    else
        LogIpcWarn("program[%s] name[%s] Code [%d]\n", mName, argName, ret);
    return ret;
}

int
IPC::pushArg(const char* argName, const char* argValue)
{
    int ret = _PushArg(argName, argValue, strlen(argValue) + 1);
    if (ret == eStackStateSuccess)
        LogIpcStack("name[%s] value[%s]\n", argName, argValue);
    else
        LogIpcWarn("program[%s] name[%s] Code [%d]\n", mName, argName, ret);
    return ret;
}

int
IPC::pushArg(const char* argName, int argValue)
{
    int ret = _PushArg(argName, (const void*)&argValue, sizeof(argValue));
    if (ret == eStackStateSuccess)
        LogIpcStack("name[%s] value[%d] Hex[0x%02x]\n", argName, argValue, argValue);
    else
        LogIpcWarn("program[%s] name[%s] Code [%d]\n", mName, argName, ret);
    return ret;
}

int
IPC::pushArg(const char* argName, unsigned int argValue)
{
    int ret = _PushArg(argName, (const void*)&argValue, sizeof(argValue));
    if (ret == eStackStateSuccess)
        LogIpcStack("name[%s] value[0x%02x]\n", argName, argValue);
    else
        LogIpcWarn("program[%s] name[%s] Code [%d]\n", mName, argName, ret);
    return ret;
}

int
IPC::pushArg(const char* argName, long argValue)
{
    int ret = _PushArg(argName, (const void*)&argValue, sizeof(long));
    if (ret == eStackStateSuccess)
        LogIpcStack("name[%s] value[%ld] Hex[0x%02x]\n", argName, argValue, argValue);
    else
        LogIpcWarn("program[%s] name[%s] Code [%d]\n", mName, argName, ret);
    return ret;
}

int
IPC::pushArg(const char* argName, float argValue)
{
    int ret = _PushArg(argName, (const void*)&argValue, sizeof(float));
    if (ret == eStackStateSuccess)
        LogIpcStack("name[%s] value[%f]\n", argName, argValue);
    else
        LogIpcWarn("program[%s] name[%s] Code [%d]\n", mName, argName, ret);
    return ret;
}

int
IPC::pushArg(const char* argName, bool argValue)
{
    int ret = _PushArg(argName, (const void*)&argValue, sizeof(argValue));
    if (ret == eStackStateSuccess)
        LogIpcStack("name[%s] value[%d]\n", argName, argValue, argValue);
    else
        LogIpcWarn("program[%s] name[%s] Code [%d]\n", mName, argName, ret);
    return ret;
}

int
IPC::pushArg(const char* argName, std::string& argValue)
{
    int ret = _PushArg(argName, argValue.c_str(), argValue.length());
    if (ret == eStackStateSuccess)
        LogIpcStack("name[%s] value[%s]\n", argName, argValue.c_str());
    else
        LogIpcWarn("program[%s] name[%s] Code [%d]\n", mName, argName, ret);
    return ret;
}

/**
 * @brief popArg  pop function's parameter (value type is char*)
 *
 * @param argName the function's paramter name
 * @param argValue the function's paramter value
 * @param argLength the length of value
 *
 * @return  nonnegative if ok (>=0)
 */
int IPC:: _PopArg(const char* argName, void* argValue, int argLength)
{
    ShmHeader_t* const pShmHeader = (ShmHeader_t*)mBaseBuffer[mPopUseID];
    StackItem_t* pStkHeader = (StackItem_t*)((char*)pShmHeader - pShmHeader->u16StackOffset);

#if defined(IPC_DEBUG_MODE)
    //Pop-ArgValue
    if (pShmHeader->u16StackOffset < kStackItemHeaderSize)
        return eStackStateIsEmpty;
    if (eTagArgName == pStkHeader->u8Tag) {
        pShmHeader->u16StackOffset -= (kStackItemHeaderSize + pStkHeader->u16Size); //pop stack
        pShmHeader->u16StackItemNums -= 1;
        if (pShmHeader->u16StackOffset <= 0) {
            pShmHeader->u16StackOffset += (kStackItemHeaderSize + pStkHeader->u16Size); //restore the original value
            pShmHeader->u16StackItemNums += 1;
            return eStackStateIsEmpty;
        }
        if (eTypeCharArray == pStkHeader->u8Type) {
            std::string name((char*)pStkHeader + kStackItemHeaderSize, pStkHeader->u16Size);
            if (strncmp(argName, name.c_str(), pStkHeader->u16Size))
                LogIpcStack("argName[%s] vs [%s]\n", argName, name.c_str());
        }
        pStkHeader = (StackItem_t*)((char*)pShmHeader - pShmHeader->u16StackOffset); //next stack header
    }
#endif

    //argValue
    pShmHeader->u16StackOffset -= (kStackItemHeaderSize + pStkHeader->u16Size); //pop offset
    pShmHeader->u16StackItemNums -= 1;
    if (eTagArgValue != pStkHeader->u8Tag)
        return eStackStateNotMatch;
    if (0 == pStkHeader->u16Size)
        return eStackStateNullValue;
    if (eTypeCharArray == pStkHeader->u8Type) {
        if (pStkHeader->u16Size > argLength) {
            LogIpcWarn("< %d %d >\n",  pStkHeader->u16Size , argLength);
            return eStackStateBuffSmall;
        }
        memcpy(argValue, (char*)pStkHeader + kStackItemHeaderSize, pStkHeader->u16Size);
    }
    return eStackStateSuccess;
}

int IPC::popArg(const char* argName, void* argValue, int argLength)
{
    int ret = _PopArg(argName, argValue, argLength);
    if (ret == eStackStateSuccess)
        LogIpcStack("name[%s] argLength[%d]\n", argName, argLength);
    else if (ret < 0) {
        LogIpcWarn("program[%s] name[%s] Code [%d]\n", mName, argName, ret);
        _ShowStackData();
    }
    return ret;
}

int
IPC:: popArg(const char* argName, unsigned char* argValue, int argLength)
{
    int ret = _PopArg(argName, argValue, argLength);
    if (ret == eStackStateSuccess)
        LogIpcStack("name[%s] value[%s]\n", argName, argValue);
    else if (ret < 0) {
        LogIpcWarn("program[%s] name[%s] Code [%d]\n", mName, argName, ret);
        _ShowStackData();
    }
    return ret;
}

int
IPC::popArg(const char* argName, int& argValue)
{
    int ret = _PopArg(argName, (void*)&argValue, sizeof(int));
    if (ret == eStackStateSuccess)
        LogIpcStack("name[%s] value[%d]\n", argName, argValue);
    else if (ret < 0)
        LogIpcWarn("program[%s] name[%s] Code [%d]\n", mName, argName, ret);
    return ret;
}

int
IPC::popArg(const char* argName, unsigned int& argValue)
{
    int ret = _PopArg(argName, (void*)&argValue, sizeof(unsigned int));
    if (ret == eStackStateSuccess)
        LogIpcStack("name[%s] value[0x%02x]\n", argName, argValue);
    else if (ret < 0)
        LogIpcWarn("program[%s] name[%s] Code [%d]\n", mName, argName, ret);
    return ret;
}

int
IPC::popArg(const char* argName, long& argValue)
{
    int ret = _PopArg(argName, (void*)&argValue, sizeof(long));
    if (ret == eStackStateSuccess)
        LogIpcStack("name[%s] value[%ld]\n", argName, argValue);
    else if (ret < 0)
        LogIpcWarn("program[%s] name[%s] Code [%d]\n", mName, argName, ret);
    return ret;
}

int
IPC::popArg(const char* argName, float& argValue)
{
    int ret = _PopArg(argName, (void*)&argValue, sizeof(float));
    if (ret == eStackStateSuccess)
        LogIpcStack("name[%s] value[%f]\n", argName, argValue);
    else if (ret < 0)
        LogIpcWarn("program[%s] name[%s] Code [%d]\n", mName, argName, ret);
    return ret;
}

int
IPC::popArg(const char* argName, bool& argValue)
{
    int ret = _PopArg(argName, (void*)&argValue, sizeof(bool));
    if (ret == eStackStateSuccess)
        LogIpcStack("name[%s] value[%d]\n", argName, argValue);
    else if (ret < 0)
        LogIpcWarn("program[%s] name[%s] Code [%d]\n", mName, argName, ret);
    return ret;
}

int
IPC::popArg(const char* argName, std::string& argValue)
{
    ShmHeader_t* const pShmHeader = (ShmHeader_t*)mBaseBuffer[mPopUseID];
    StackItem_t* pStkHeader = (StackItem_t*)((char*)pShmHeader - pShmHeader->u16StackOffset);

#if defined(IPC_DEBUG_MODE)
    //Pop-ArgName
    if (pShmHeader->u16StackOffset < kStackItemHeaderSize)
        return eStackStateIsEmpty;
    if (eTagArgName == pStkHeader->u8Tag) {
        pShmHeader->u16StackOffset -= (kStackItemHeaderSize + pStkHeader->u16Size); //pop stack
        pShmHeader->u16StackItemNums -= 1;
        if (pShmHeader->u16StackOffset <= 0) {
            pShmHeader->u16StackOffset += (kStackItemHeaderSize + pStkHeader->u16Size); //restore the original value
            pShmHeader->u16StackItemNums += 1;
            return eStackStateIsEmpty;
        }
        if (eTypeCharArray == pStkHeader->u8Type) {
            std::string name((char*)pStkHeader + kStackItemHeaderSize, pStkHeader->u16Size);
            if (strncmp(argName, name.c_str(), pStkHeader->u16Size))
                LogIpcDebug("argName[%s] vs [%s]\n", argName, name.c_str());
        }
        pStkHeader = (StackItem_t*)((char*)pShmHeader - pShmHeader->u16StackOffset); //next stack header
    }
#endif

    //Pop-ArgValue
    pShmHeader->u16StackOffset -= (kStackItemHeaderSize + pStkHeader->u16Size); //pop offset
    pShmHeader->u16StackItemNums -= 1;
    if (eTagArgValue != pStkHeader->u8Tag)
        return eStackStateNotMatch;
    if (0 == pStkHeader->u16Size)
        return eStackStateNullValue;
    if (eTypeCharArray == pStkHeader->u8Type)
        argValue.assign((char*)pStkHeader + kStackItemHeaderSize, pStkHeader->u16Size);
    LogIpcDebug("name[%s] value[%s]\n", argName, argValue.c_str());
    return eStackStateSuccess;
}

/**
 * @brief invoke notify the other side(process), just like P(0)
 *
 * @param semNum the number of the semphore (default: 0)
 *
 * @return 0 if ok
 */
int
IPC::invoke()
{
    static struct sembuf sBuf;
	sBuf.sem_num = 0;
	sBuf.sem_op = +1;
	sBuf.sem_flg = 0; //SEM_UNDO;
    //errno: EIDRM(43:mID already removed) EFBIG(27:semnum>=mNums) EAGAIN(when flag==IPC_NOWAIT)
	return semop(mSemID[mPushUseID], &sBuf, 1);
}

/**
 * @brief request wait or check whether the stack is not empty, just like V(0)
 *
 * @param semNum the number of the semphore (default: 0)
 * @param block if block == 1, then request will block until the stack is not empty(P(0)) (default: 1)
 *          SEM_UNDO: P,V operation must exist in the same program, otherwise cause ERANGE error.
 *
 * @return 0 if ok
 */
int
IPC::wait(int block)
{
    //If semval is greater than or equal to the absolute value of sem_op, the operation can proceed immediately
    static struct sembuf sBuf;
    sBuf.sem_num = 0;
    sBuf.sem_op = -1;
    sBuf.sem_flg = block ? 0/*SEM_UNDO*/ : IPC_NOWAIT;
    //errno: EIDRM(43:mID already removed) EFBIG(27:semnum>=mNums) EAGAIN(when flag==IPC_NOWAIT) ERANGE(34:SEM_UNDO cause)
    return semop(mSemID[mPopUseID], &sBuf, 1);
}

bool
IPC::isResult()
{
    ShmHeader_t* const pShmHeader = (ShmHeader_t*)mBaseBuffer[mPopUseID];

    if (pShmHeader->u16StackPurpose == SP_Result)
        return true;
    else
        return false;
}

bool
IPC::isCall()
{
    ShmHeader_t* const pShmHeader = (ShmHeader_t*)mBaseBuffer[mPopUseID];

    if (pShmHeader->u16StackPurpose == SP_Call)
        return true;
    else
        return false;
}

bool
IPC::isEmpty()
{
    ShmHeader_t* const pShmHeader = (ShmHeader_t*)mBaseBuffer[mPopUseID];
    return (pShmHeader->u16StackItemNums == 0);
}

/*                                                  semop errno reference
 *  E2BIG   (7:Argument list too long )
 *          The argument nsops is greater than SEMOPM, the maximum number of operations allowed per system call.
 *
 *  EACCES  (13:Permission denied)
 *          The calling process does not have the permissions required to perform the specified semaphore operations, and does not
 *          have the CAP_IPC_OWNER capability.
 *
 *  EAGAIN  (11:Try again)
 *          An operation could not proceed immediately and either IPC_NOWAIT was specified in sem_flg or the time limit  specified
 *          in timeout expired.
 *
 *  EFAULT  (14:Bad address)
 *          An address specified in either the sops or the timeout argument isn't accessible.
 *
 *  EFBIG   (27:File too large)
 *          For  some  operation  the  value of sem_num is less than 0 or greater than or equal to the number of semaphores in the
 *          set.
 *
 *  EIDRM   (43:ID was removed)
 *          The semaphore set was removed.
 *
 *  EINTR   (4: Interrupted system call)
 *          While blocked in this system call, the process caught a signal; see signal(7).
 *
 *  EINVAL  (22:Invalid argument)
 *          The semaphore set doesn't exist, or semid is less than zero, or nsops has a nonpositive value.
 *
 *  ENOMEM  (12:Out of memor)
 *          The sem_flg of some operation specified SEM_UNDO and the system does not have  enough  memory  to  allocate  the  undo
 *          structure.
 *
 *  ERANGE  (34:Numerical result out of range - think of SEM_UNDO)
 *          For some operation sem_op+semval is greater than SEMVMX, the implementation dependent maximum value for semval. */

} // namespace Hippo
