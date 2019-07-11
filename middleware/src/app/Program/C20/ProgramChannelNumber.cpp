
#include "ProgramChannelNumber.h"


namespace Hippo {

ProgramChannelNumber::ProgramChannelNumber()
{
}

ProgramChannelNumber::~ProgramChannelNumber()
{
}

Program::ProgramType 
ProgramChannelNumber::getType()
{
    return PT_CHANNEL_NUMBER;
}

} // namespace Hippo
