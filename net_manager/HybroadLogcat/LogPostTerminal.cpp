#include "LogPostTerminal.h"

#include <stdio.h>

namespace android {

bool 
LogPostTerminal::pushBlock(uint8_t* blockHead, uint32_t blockLength)
{
    fwrite(blockHead, blockLength, 1, stdout);
    return false;
}

} // namespace android
