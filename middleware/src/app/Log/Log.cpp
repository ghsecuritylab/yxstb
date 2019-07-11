
#include "Log.h"

#include "DataSink.h"
#include "RingBuffer.h"
#include "SysTime.h"
#include "LogModuleHuawei.h"

#include "customer.h"

#include <string.h>

#include "NetworkFunctions.h"
#include "stbinfo/stbinfo.h"

namespace Hippo {

#define MAX_BLOCK_SIZE	256


static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
static char g_logFixHead[128 + 1]; // STBType + HuaweiVersion | STBMac

static const char* textLevel[] = {"Assert : ", "Error! : ", "Warning: ", "Normal : ", "Verbose: "};
static const char* monthStr[] = {"Undefined", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

Log::Log()
    : mExtStyle(1)
{
}

Log::~Log()
{
}

void Log::log(int level, const char* fmt, va_list args)
{
    uint8_t* bufPointer;
    uint32_t bufLength;

    pthread_mutex_lock(&g_mutex);

    m_ringBuffer->getWriteHead(&bufPointer, &bufLength);
    if (bufLength == 0) {
        pthread_mutex_unlock(&g_mutex);
        return;
    }

    int dataSize = 8, blockSize = 0, writeLen = 0;

    writeLen = snprintf((char*)bufPointer + dataSize, bufLength - dataSize, "%s", textLevel[level]);
    if (writeLen < 0) /* Error. */
        goto Exit;
    dataSize += writeLen;

    writeLen = vsnprintf((char*)bufPointer + dataSize, bufLength - dataSize, fmt, args);
    if (writeLen < 0) /* Error. */
        goto Exit;
    if (writeLen >= ((int)bufLength - dataSize))
        dataSize = bufLength;
    else
        dataSize += writeLen;

    //date size.
    *((int *)(bufPointer + 4)) = dataSize - 8;
    //block size.
    blockSize = (dataSize + 8 + 3) & 0xfffffffc;
    if ((bufLength - blockSize) > MAX_BLOCK_SIZE) {
        *((int *)(bufPointer + 0)) = blockSize;
        m_ringBuffer->submitWrite(bufPointer, blockSize);
    }
    else {
        *((int *)(bufPointer + 0)) = bufLength;
        m_ringBuffer->submitWrite(bufPointer, bufLength);
    }

    if (m_dataSink)
        m_dataSink->onDataArrive();
Exit:
    pthread_mutex_unlock(&g_mutex);
}

void Log::logVerbose(const char* file, int line, const char* function, LogType pType, int level, const char* fmt, va_list args)
{
    uint8_t* bufPointer;
    uint32_t bufLength;

    /** Note: must execute before pthread_mutex_lock, otherwise will deathlock */
    if (!g_logFixHead[0]) {
        static int sGetHeadStatus = 0;
        if (1 == sGetHeadStatus) {
            printf("Sorry, ignore %s:%s:%d\n", file, function, line);
            return ;
        }
        sGetHeadStatus = 1;
        char tag[32] = { 0 };
        char mac[64] = { 0 };
        // Deadlock: nested pthread_mutex_lock(app_sys_settings::g_mutex).
        snprintf(tag, sizeof(tag), "%s", StbInfo::STB::Model());   /** Deadlock: nested execute LogModule possibly */
        network_tokenmac_get(mac, 64, ':');
        snprintf(g_logFixHead, 128, "%s %s%s", mac, tag, StbInfo::STB::Version::HWVersion());
        sGetHeadStatus = 2;
    }
    pthread_mutex_lock(&g_mutex);

    m_ringBuffer->getWriteHead(&bufPointer, &bufLength);  /** getWriteHead cannot use LogModule */
    if (bufLength == 0) {
        pthread_mutex_unlock(&g_mutex);
        return;
    }

    int dataSize = 8, blockSize, writeLen;

    writeLen = logPrefix((char*)bufPointer + dataSize, bufLength - dataSize, file, line, function, pType, level);
    if ((writeLen < 0) || (writeLen >= ((int)bufLength - dataSize))) /* Error or log too long. */
        goto Exit;
    dataSize += writeLen;

    writeLen = vsnprintf((char*)bufPointer + dataSize, bufLength - dataSize, fmt, args);
    if (writeLen < 0) /* Error. */
        goto Exit;
    if (writeLen >= ((int)bufLength - dataSize))
        dataSize = bufLength;
    else
        dataSize += writeLen;

    //date size.
    *((int *)(bufPointer + 4)) = dataSize - 8;
    //block size.
    blockSize = (dataSize + 8 + 3) & 0xfffffffc;
    if ((bufLength - blockSize) > MAX_BLOCK_SIZE) {
        *((int *)(bufPointer + 0)) = blockSize;
        m_ringBuffer->submitWrite(bufPointer, blockSize); /** submitWrite cannot use LogModule */

    }
    else {
        *((int *)(bufPointer + 0)) = bufLength;
        m_ringBuffer->submitWrite(bufPointer, bufLength);
    }
    if (m_dataSink)
        m_dataSink->onDataArrive();
Exit:
    pthread_mutex_unlock(&g_mutex);
}

int Log::logPrefix(char* buffer, int length, const char* file, int line, const char* function, LogType pType, int level)
{
    if (!buffer || !file)
        return -1;
    static SysTime::DateTime sDTime;
    SysTime::GetDateTime(&sDTime);

    const char* pFile = strrchr(file, '/');
    if (pFile)
        pFile = pFile + 1;
    else
        pFile = file;

    // <134>Jun 10 17:00:20 00:11:09:EF:B5:AF EC1308V100R001C02B021 xxx.c:1538 code:11 202.173.12.88 connect  rtsp://202.173.4.88/mov/test.ts timeout
    if (mExtStyle) {
        int tHuaweiLogLevel = 0;
        int tHuaweiLogType = 0;

        if(level == LOG_LEVEL_ERROR) {
            tHuaweiLogLevel = HLL_ERROR;
        } else if(level == LOG_LEVEL_WARNING) {
            tHuaweiLogLevel = HLL_INFORMATION;
        } else {
            tHuaweiLogLevel = HLL_DEBUG;
        }
        switch(pType) {
            case HLG_OPERATION:
                tHuaweiLogType = 16;
                break;
        	case HLG_RUNNING:
        	    tHuaweiLogType = 17;
                break;
        	case HLG_SECURITY:
        	    tHuaweiLogType = 19;
                break;
        	case HLG_USER:
        	    tHuaweiLogType = 20;
                break;
        	case HLG_ALL:
        	default:
        	    tHuaweiLogType = 1;
                break;
        }
//        return snprintf(buffer, length, "%04d-%02d-%02d %02d:%02d:%02d.%03d | %s | %d:%d | %s:%d |code:11| %s",
//            sDTime.mYear, sDTime.mMonth, sDTime.mDay, sDTime.mHour, sDTime.mMinute, sDTime.mSecond, sTimeSpec.tv_nsec / 1000000,
//            g_logFixHead, tHuaweiLogType, tHuaweiLogLevel, pFile, line, textLevel[level]);
        return snprintf(buffer, length, "<%d>%s %02d %02d:%02d:%02d %s %s:%d code:11 %s",
            tHuaweiLogType * 8 + tHuaweiLogLevel, monthStr[sDTime.mMonth], sDTime.mDay,
            sDTime.mHour, sDTime.mMinute, sDTime.mSecond, g_logFixHead, pFile, line, textLevel[level]);
    } else {
        static struct timespec sTimeSpec;
        clock_gettime(CLOCK_REALTIME, &sTimeSpec);
        return snprintf(buffer, length, "%02d:%02d:%02d.%03d|%s:%d|%s|%s",
            sDTime.mHour, sDTime.mMinute, sDTime.mSecond, sTimeSpec.tv_nsec / 1000000,  pFile, line, function, textLevel[level]);
    }
}

} // namespace Hippo
