#ifndef __PingTestDiagnose__H_
#define __PingTestDiagnose__H_

#include "NetworkDiagnose.h"
#include <netdb.h>

#ifdef __cplusplus

#include <string>
#include <list> 

typedef struct _PingPacketInfo {
	char nHostAddr[NI_MAXHOST]; //PING_INFO_ADDRESS
    unsigned int nPacketSize;   //PING_INFO_DATA
    unsigned int nPacketNumber; //PING_INFO_SEQUENCE
    unsigned int nRecvTTL;      //PING_INFO_RECV_TTL
    double nResponsTime;        //PING_INFO_LATENCY
}PingPacketInfo_s;

#define MAX_SAVE_COUNT 100

class PingTestDiagnose : public NetworkDiagnose::DiagnoseProcess {
public:
    PingTestDiagnose(ReportEvent_f fun = 0);
    ~PingTestDiagnose(); 

    virtual int type() { return eTypePingTestDiagnose; }
    virtual int start(NetworkDiagnose* netdiag);
    virtual int stop();
    virtual int doResult(int result, int arg1, void* arg2);

    void setHostName(const char* name) { mHostName = name; }

    void setPacketSize(int size) { mPacketSize = size; }
    void setPacketCount(int count) { mPacketCount = count; }
    void setTiemoutMs(int ms) { mTimeoutMs = ms; }
    void setTTL(int ttl) { mTTL = ttl; }

    //advanced
    const char* getHostAddr() { return mResolvIP.c_str(); }

    int getPacketSendCount() { return mPacketSendCount; }
    int getPacketRecvCount() { return mPacketRecvCount; }
    int getPacketErrorCount() { return mPacketErrorCount; }
    int getPacketTimeoutCount() { return mPacketTimeoutCount; }

    double getMaxDelay() { return mMaxDelay; }
    double getMinDelay() { return mMinDelay; }
    double getAverageDelay() { return mAverageDelay; }
    double getSuccessRate() { return mSuccessRate; }

    std::list<PingPacketInfo_s*>& getPacketInfos() { return mPacketInfos; }

protected:
    int _DnsResolv(int family, const char* hostname);
    int _PingTest(int family);

private:
    std::string mHostName;
    std::string mResolvIP;
    int mPacketSize;
    int mPacketCount;
    int mTimeoutMs;
    int mTTL;

    int mPacketSendCount;
    int mPacketRecvCount;
    int mPacketErrorCount;
    int mPacketTimeoutCount;

    double mMaxDelay;
    double mMinDelay;
    double mAverageDelay;
    double mSuccessRate;

    std::list<PingPacketInfo_s*> mPacketInfos;

    ReportEvent_f _ReportEvent; //fuck doc.
};
#endif

#endif
