#ifndef _IPCSERVER_H_
#define _IPCSERVER_H_

#ifdef __cplusplus

#include "IPC.h"
#include <map>

namespace Hippo {

typedef int (* IPCFunction)(void*);

class IPCServer : public virtual IPC {
public:
    IPCServer();
    virtual ~IPCServer();

    virtual int _serve();
    int serve(int block = 1);

    int popObjectName(std::string& functionName);
    int popFunctionName(std::string& objectName);

    int addObject(const char* objectName, IPCServer* object);
    int removeObject(const char* objectName);

protected:
    //self-member funtion
    int addFunction(const char* functionName, IPCFunction function);
    int removeFunction(const char* functionName);
    static int removeServerObject(void*);

    typedef std::map<std::string, IPCServer*>::const_iterator IterMapObjects_t;
    typedef std::map<std::string, IPCFunction>::const_iterator IterMapFunctions_t;
    void _ShowObjectsList(void);//for Debug
    void _ShowFunctionsList(void);//for Debug
private:
    std::map<std::string, IPCServer*> mObjects;
    std::map<std::string, IPCFunction> mFunctions;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _IPCSERVER_H_
