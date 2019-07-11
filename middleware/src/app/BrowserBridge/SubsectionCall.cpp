
#include "SubsectionCall.h"


namespace Hippo {

SubsectionCall::SubsectionCall()
{
}

SubsectionCall::~SubsectionCall()
{
}

const char *
SubsectionCall::subsection()
{
    return "root";
}

int 
SubsectionCall::registerNextSubsection(SubsectionCall *sub)
{
    return 0;
}

int 
SubsectionCall::unregisterNextSubsection(SubsectionCall *sub)
{
    return 0;
}

int 
SubsectionCall::collisionReport(const char *subsections)
{
    return 0;
}

int 
SubsectionCall::call(const char *subsections)
{
    return 0;
}

} // namespace Hippo
