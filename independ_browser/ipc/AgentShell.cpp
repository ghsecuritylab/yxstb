#include "AgentShell.h"
#include <signal.h>

namespace Hippo {

AgentShell::AgentShell()
{
    s_topServer = this;

    addObject("agent", this);
}

AgentShell::~AgentShell()
{
}

} //End namespace Hippo
