#ifndef _IPCCaller_H_
#define _IPCCaller_H_

#include "IPC.h"

#ifdef __cplusplus

namespace Hippo {

class IPCCaller : public virtual IPC {
public:
    IPCCaller(const char* objectName = "agent", IPCCaller* parentNodeCaller = NULL);
    ~IPCCaller();

    virtual int _call();
    int call(const char* functionName);

protected:
    virtual int removeServerObject(std::string& objectName);

    int pushFunction(const char* functionName);
    int pushObject();

    std::string mObjectName;
private:
    IPCCaller* mParentNodeCaller;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _IPCCaller_H_
