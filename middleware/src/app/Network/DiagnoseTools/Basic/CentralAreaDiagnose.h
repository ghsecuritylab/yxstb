#ifndef __CentralAreaDiagnose__H_
#define __CentralAreaDiagnose__H_

#include "NetworkDiagnose.h"
#include "libcares/ares.h"

#ifdef __cplusplus
#include <string>
#include <vector>

#define RS_DNS_ERR -1
#define RS_DNS_NON 0
#define RS_DNS_OK  1

const unsigned int kHostMaxCount = 5;
const unsigned int kHostUrlLength = 256;

const unsigned int kNtpTimeout = 6;
const unsigned int kDnsTimeout = 10;
const unsigned int kPingTimeout = 5;
const unsigned int kHttpTimeout = 6;

class CentralAreaDiagnose : public NetworkDiagnose::DiagnoseProcess {
public:
    CentralAreaDiagnose(int mode = ND_MODE_SERVICE);
    ~CentralAreaDiagnose(); 

    virtual int type() { return eTypeCentralAreaDiagnose; }
    virtual int start(NetworkDiagnose* netdiag);
    virtual int stop();
    virtual int doResult(int result, int arg1, void* arg2);

    void addDns(const char* dns);
    void setUrl(const char* url);

    bool isResolvOK(int n) { return RS_DNS_OK == mResolvST[n]; }

    double getMaxDelay() { return mMaxDelay; }
    double getSuccessRate() { return mSuccessRate; }

protected:
    int _DnsResolv(int family, char** hosts, int count);
    int _AdoptPing(int family);
    int _AdoptHttp(int* httpcode);

protected:
    int mDiagMode;

    struct ares_addr_node* mServers;
    std::vector<int> mResolvST;
    std::vector<std::string> mResolvIP;
    std::vector<std::string> mTestUrl;

    double mMaxDelay;
    double mSuccessRate;
};

#endif

#endif
