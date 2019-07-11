#ifndef _AgentShell_H_
#define _AgentShell_H_  

#include "IPCServer.h"
#include "IPCCaller.h"

namespace Hippo {

class AgentShell : public IPCServer, public IPCCaller {
public:
    AgentShell();
    ~AgentShell();

private:
    AgentShell(const AgentShell&);
    AgentShell& operator=(const AgentShell&);
};
} //End namespace Hippo

#endif //End _AgentShell_H_
