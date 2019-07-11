/**
 * @file IPCCaller.cpp
 * @brief 
 * @author 
 * @version 1.0
 * @date 2013-03-05
 */

#include "IPCCaller.h"
#include "AgentShell.h"

extern const unsigned int kStackItemHeaderSize;

/*
 * ////////////////////////////////////////////////////////////////////////////////////
 *                       Shared Memory                      _____________________
 *                      |             |<---shmat()                      +
 *          PUSH        |             |                                 |
 *                      |             |                                 |
 *                      |             |______________________           | 
 *     StackOffset +--->|-------------|\                     +          |
 *      (new pos)  |    |tag^type^size| kStackItemHeaderSize |          |
 *                 |    |-------------|/                     |     StackLength
 *                 |    |             |\                   item         |
 *                 |    |payload data | u16Size              |          |
 *      (old pos)  |    |             |/_____________________+          |
 *     StackOffset-+--->|-------------|                                 |
 *                 |    |      .      |                                 |
 *                 |    |      .      |                                 |
 *                 |    |      .      |_________________________________+________
 *                 |    |-------------|<---mBaseBuffer[mPushUseID]        
 *                 +--->| StackOffset |                                          
 *                      | StackLength |(ShmHeader_t)
 *     high address_____|-------------|
 * ////////////////////////////////////////////////////////////////////////////////////
 */

namespace Hippo {

/**
 * @brief IPCCaller constructor
 *
 * @param objectName object name
 * @param parentNodeCaller it exists the aggregation relationship with this object
 */
IPCCaller::IPCCaller(const char* objectName, IPCCaller* parentNodeCaller)
    :mObjectName(objectName), mParentNodeCaller(parentNodeCaller) 
{
    LogIpcDebug("constructor:[%s]\n", mObjectName.c_str());
}

/**
 * @brief ~IPCCaller destructor
 */
IPCCaller::~IPCCaller()
{
    LogIpcDebug("destructor:[%s]\n", mObjectName.c_str());
    if (mParentNodeCaller)
        mParentNodeCaller->removeServerObject(mObjectName);
}

/**
 * @brief pushFunction push the funtion name into the shared memory stack specified by mPushUseID
 *
 * @param functionName function name
 *
 * @return nonnegative number on success (>=0)
 */
int 
IPCCaller::pushFunction(const char* functionName)
{
    ShmHeader_t* const pShmHeader = (ShmHeader_t*)mBaseBuffer[mPushUseID];
    StackItem_t item = { eTagFuntion, eTypeCharArray, strlen(functionName) + 1 };
    LogIpcStack("%s:seq[%d] shm[%d] offset[%u] size[%u] Tag[%d]\n", mName, pShmHeader->u32StackSequence, mPushUseID, pShmHeader->u16StackOffset, pShmHeader->u16StackLength, item.u8Tag);
    pShmHeader->u16StackOffset += (kStackItemHeaderSize + item.u16Size); //update the new pos
    pShmHeader->u16StackItemNums += 1;
    if (pShmHeader->u16StackOffset > pShmHeader->u16StackLength) {
        pShmHeader->u16StackOffset -= (kStackItemHeaderSize + item.u16Size); //return the old pos 
        pShmHeader->u16StackItemNums -= 1;
        return eStackStateIsFull;
    }
    //push data into stack
    memcpy(mempcpy((char*)pShmHeader - pShmHeader->u16StackOffset, &item, kStackItemHeaderSize), functionName, item.u16Size);
    LogIpcStack("%s\n", functionName);
    return eStackStateSuccess;
}

/**
 * @brief pushObject push the object name into the shared memory stack specified by mPushUseID
 *
 * @return nonnegative number on success (>=0)
 */
int 
IPCCaller::pushObject()
{
    ShmHeader_t* const pShmHeader = (ShmHeader_t*)mBaseBuffer[mPushUseID];
    StackItem_t item = { eTagObject, eTypeCharArray, mObjectName.length()};
    LogIpcStack("%s:seq[%d] shm[%d] offset[%u] size[%u] Tag[%d]\n", mName, pShmHeader->u32StackSequence, mPushUseID, pShmHeader->u16StackOffset, pShmHeader->u16StackLength, item.u8Tag);
    pShmHeader->u16StackOffset += (kStackItemHeaderSize + item.u16Size); //update the new pos
    pShmHeader->u16StackItemNums += 1;
    if (pShmHeader->u16StackOffset > pShmHeader->u16StackLength) {
        //LogIpcDebug("stack is full\n");
        pShmHeader->u16StackOffset -= (kStackItemHeaderSize + item.u16Size); // return the old pos
        pShmHeader->u16StackItemNums -= 1;
        return eStackStateIsFull;
    }
    //push data into stack
    memcpy(mempcpy((char*)pShmHeader - pShmHeader->u16StackOffset, &item, kStackItemHeaderSize), mObjectName.c_str(), item.u16Size);
    LogIpcStack("%s\n", mObjectName.c_str());
    return eStackStateSuccess;
}

/**
 * @brief removeServerObject 
 *
 * @param objectName name
 *
 * @return nonnegative number on sucess (>=0)
 */
int 
IPCCaller::removeServerObject(std::string& objectName)
{
    initStack(SP_Call);
    pushArg("objectName", objectName);
    call("removeServerObject");
    freeStack();
    return 0;
}

/**
 * @brief _call recursive _call for push object name into stack and invoke the other process
 *
 * @return nonnegative number on success (>=0)
 */
int 
IPCCaller::_call()
{
    pushObject();
    if (mParentNodeCaller)
        return mParentNodeCaller->_call();
    return invoke();//notify the other process side which can read shared memory 
}

/**
 * @brief call need execute 'call' after 'pushed' one function's all paramters into the stack
 *
 * @param functionName function name
 *
 * @return nonnegative number on success (>=0)
 */
int 
IPCCaller::call(const char* functionName)
{
    pushFunction(functionName);
    if (_call() < 0) //recursive push object name
        return -1;
    while (true) {
        if (wait() < 0)
            return -1;
        if (isResult())
            return 0;
        if (s_topServer->serve() < 0)
            return -1;
    }
    return -1;
}

} // namespace Hippo
