
#include "LogPrinter.h"

#if defined(ANDROID)
#include "cutils/log.h"
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "IPTV :IPTVMiddleware"
#endif

#include <stdio.h>

namespace Hippo {
bool
LogPrinter::pushBlock(uint8_t* blockHead, uint32_t blockLength)
{
#if defined(ANDROID)
    android_writeLog(ANDROID_LOG_INFO, LOG_TAG, (const char*)blockHead);
#else
    fwrite(blockHead, blockLength, 1, stdout);
#endif

    return false;
}

} // namespace Hippo
