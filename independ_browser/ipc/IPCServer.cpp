/**
 * @file IPCServer.cpp
 * @brief
 * @author
 * @version 1.0
 * @date 2013-03-05
 */

#include "IPCServer.h"
#include "AgentShell.h"

extern const unsigned int kStackItemHeaderSize;

/*
 * ////////////////////////////////////////////////////////////////////////////////////
 *                    Shared Memory                      _____________________
 *                   |             |<---shmat()                      ^
 *        POP        |             |                                 |
 *                   |             |                                 |
 *                   |             |______________________           |
 *  StackOffset +--->|-------------|\                     +          |
 *   (old pos)  |    |tag^type^size| kStackItemHeaderSize |          |
 *              |    |-------------|/                     |     StackLength
 *              |    |             |\                   item         |
 *              |    |payload data | u16Size(objectName)  |          |
 *   (new pos)  |    |             |/_____________________+          |
 *  StackOffset-+--->|-------------|                                 |
 *              |    |      .      |                                 |
 *              |    |      .      |                                 |
 *              |    |      .      |_________________________________v________
 *              |    |-------------|<---mBaseBuffer (StackOffset from mBaseBuffer)
 *              +--->| StackOffset |
 *                   | StackLength |(ShmHeader_t)
 *  high address_____|-------------|
 * ////////////////////////////////////////////////////////////////////////////////////
 */

namespace Hippo {
/**
 * @brief IPCServer constructor
 */
IPCServer::IPCServer()
{
    LogIpcDebug("constructor\n");
}

/**
 * @brief ~IPCServer destructor
 */
IPCServer::~IPCServer()
{
    LogIpcDebug("destructor\n");
}

/**
 * @brief popObjectName pop object name from the shared memory stack specified by mPopUseID
 *
 * @param objectName object name
 *
 * @return nonnegative number on success (>=0)
 */
int
IPCServer::popObjectName(std::string& objectName)
{
    ShmHeader_t* const pShmHeader = (ShmHeader_t*)mBaseBuffer[mPopUseID];
    StackItem_t* const pStkHeader = (StackItem_t*)((char*)pShmHeader - pShmHeader->u16StackOffset);
    LogIpcStack("%s:seq[%d] shm[%d] offset[%u] size[%u] Tag[%d]\n", mName, pShmHeader->u32StackSequence, mPopUseID, pShmHeader->u16StackOffset, pShmHeader->u16StackLength, pStkHeader->u8Tag);
    if (eTagObject != pStkHeader->u8Tag)
        return eStackStateNotMatch;
    pShmHeader->u16StackOffset -= (kStackItemHeaderSize + pStkHeader->u16Size);
    pShmHeader->u16StackItemNums -= 1;
    if (pShmHeader->u16StackOffset <= 0 ) {
        pShmHeader->u16StackOffset += (kStackItemHeaderSize + pStkHeader->u16Size); //restore the original value
        pShmHeader->u16StackItemNums += 1;
        return eStackStateIsEmpty;
    }
    //Pop data
    if (pStkHeader->u8Type == eTypeCharArray)
        objectName.assign((char*)pStkHeader + kStackItemHeaderSize, pStkHeader->u16Size);
    LogIpcStack("%s\n", objectName.c_str());
    return eStackStateSuccess;
}

/**
 * @brief popFunctionName pop function from the shared memory stack specified by mPopUseID
 *
 * @param functionName function name
 *
 * @return nonnegative number on success (>=0)
 */
int
IPCServer::popFunctionName(std::string& functionName)
{
    ShmHeader_t* const pShmHeader = (ShmHeader_t*)mBaseBuffer[mPopUseID];
    StackItem_t* const pStkHeader = (StackItem_t*)((char*)pShmHeader - pShmHeader->u16StackOffset);
    LogIpcStack("%s:seq[%d] shm[%d] offset[%u] size[%u] Tag[%d]\n", mName, pShmHeader->u32StackSequence, mPopUseID, pShmHeader->u16StackOffset, pShmHeader->u16StackLength, pStkHeader->u8Tag);
    if (eTagFuntion != pStkHeader->u8Tag)
        return eStackStateNotMatch;
    pShmHeader->u16StackOffset -= (kStackItemHeaderSize + pStkHeader->u16Size);
    pShmHeader->u16StackItemNums -= 1;
    if (pShmHeader->u16StackOffset < 0) {
        pShmHeader->u16StackOffset += (kStackItemHeaderSize + pStkHeader->u16Size); //restore the original value
        pShmHeader->u16StackItemNums += 1;
        return eStackStateIsEmpty;
    }
    //Pop data
    if (pStkHeader->u8Type == eTypeCharArray)
        functionName.assign((char*)pStkHeader + kStackItemHeaderSize, pStkHeader->u16Size);
    LogIpcStack("%s \n", functionName.c_str());
    return eStackStateSuccess;
}

/**
 * @brief _serve
 *          # 1. recursive _serve for pop object name
 *          # 2. pop function name
 *          # 3. execute function caller
 *          # 4. invoke the other side(process) for get result of function call.
 * @return -1 on error or eStackStateError
 */
int
IPCServer::_serve()
{
    //Pop Object
    static std::string objectName; //Using 'static' for high efficiency, reduce the call of constructor and destructor of std:string
    if (eStackStateSuccess == popObjectName(objectName)) {
        IPCServer* obj = mObjects[objectName.c_str()];
        if (!obj) {
            LogIpcError("object[%s] not found.\n", objectName.c_str());
            _ShowObjectsList();
            return eStackStateError;
        }
        return obj->_serve();
    }
    //Pop Function
    static std::string functionName; //Using 'static' for high efficiency, reduce the call of constructor and destructor of std:string
    if (eStackStateSuccess == popFunctionName(functionName)) {
        IPCFunction functionCall = mFunctions[functionName.c_str()];
        if (!functionCall) {
            LogIpcError("function[%s] not found.\n", functionName.c_str());
            _ShowFunctionsList();
            return eStackStateError;
        }
        functionCall(this);
    } else {
        LogIpcError("<%s> (%s) seriously:not have function tag.\n", mName, objectName.c_str());
        _ShowStackData();
        return eStackStateError;
    }
    return invoke(); //function return value invoke;
}

/**
 * @brief serve the entrance of server
 *
 * @return 0 is ok,  '< 0' on error
 */
int
IPCServer::serve(int block)
{
    int ret = _serve();

    if (-1 == ret && EINTR == errno)
        return 0;
    return ret;
}

void
IPCServer::_ShowObjectsList(void)
{
    IterMapObjects_t it = mObjects.begin();
    int iCnt = 0;
    while (it != mObjects.end()) {
        LogIpcTest("<%s>### %d. name[%s] pointer[%p]\n", mName, ++iCnt, it->first.c_str(), it->second);
        it++;
    }
}

void
IPCServer::_ShowFunctionsList(void)
{
    IterMapFunctions_t it = mFunctions.begin();
    int iCnt = 0;
    while (it != mFunctions.end()) {
        LogIpcTest("<%s>### %d. name[%s] pointer[%p]\n", mName, ++iCnt, it->first.c_str(), it->second);
        it++;
    }
}

/**
 * @brief addObject add object that is the aggregation relationship with this OBJ
 *
 * @param objectName object name
 * @param object object instance pointer
 *
 * @return 0
 */
int
IPCServer::addObject(const char* objectName, IPCServer* object)
{
    LogIpcDebug("%s\n", objectName);
    if (mObjects.find(objectName) == mObjects.end())
        mObjects[objectName] = object;
    else
        LogIpcDebug("object[%s] already add.\n", objectName);
    return 0;
}

/**
 * @brief addFunction register member method
 *
 * @param functionName function name
 * @param function function pointer
 *
 * @return 0
 */
int
IPCServer::addFunction(const char* functionName, IPCFunction function)
{
    LogIpcDebug("%s\n", functionName);
    if (mFunctions.find(functionName) == mFunctions.end())
        mFunctions[functionName] = function;
    else
        LogIpcDebug("function[%s] already add.\n", functionName);
    return 0;
}

/**
 * @brief removeObject remove server object from map table and release object resource
 *
 * @param objectName name
 *
 * @return always ok
 */
int 
IPCServer::removeObject(const char* objectName)
{
    LogIpcTest("%s\n", objectName);
    IPCServer* pObj = NULL;
    if (mObjects.find(objectName) != mObjects.end()) {
        pObj = mObjects[objectName];
        mObjects.erase(objectName);
        delete pObj;
    }
    return 0;
}

/**
 * @brief removeServerObject 
 *
 * @param obj object pointer
 *
 * @return always ok
 */
int 
IPCServer::removeServerObject(void* obj)
{
    IPCServer* self = (IPCServer*)obj;

    std::string objectName;
    self->popArg("objectName", objectName);
    self->freeStack();
    self->removeObject(objectName.c_str());

    self->initStack(SP_Result);
    return 0;
}

} // namespace Hippo

