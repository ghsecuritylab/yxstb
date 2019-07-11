#ifndef __NetworkDiagnose__H_
#define __NetworkDiagnose__H_

#include "Thread.h"

#define ND_STATE_FAIL       0
#define ND_STATE_FINISH     1
#define ND_STATE_RUNNING    2
#define ND_STATE_MANUSTOP   3
#define ND_STATE_AUTOSTOP   4

#define ND_RESULT_FAIL      0
#define ND_RESULT_SUCCESS   1

#define ND_MODE_ICMP        1
#define ND_MODE_SERVICE     2

#ifdef __cplusplus
#include <string>

enum {
    eTypePhysicalNetDiagnose = 0,
    eTypeAddressDiagnose,
    eTypeGatewayDiagnose,
    eTypeCentralAreaDiagnose,
    eTypeNtpDiagnose,
    eTypeMulticastDiagnose,
    eTypeDnsDiagnose,
    eTypePingTestDiagnose,
    eTypeNetworkSpeedDiagnose,
};

enum {
    eTEST_ERR = -1,
    eTEST_OK = 0,
    eTEST_STOP = 1,
    eTEST_TIMEOUT = 2,

    eDNS_ERR = -100,
    eDNS_TIMEOUT,
    eDNS_NOADDR,
    eDNS_STOP,

    ePING_ERR = -200,
    ePING_TIMEOUT,
    ePING_STOP,

    eHTTP_TIMEOUT = -300,
    eHTTP_REJECT,
    eHTTP_RESPONSE,
    eHTTP_UNKNOW,
    eHTTP_STOP,

    eNTP_TIMEOUT = -400,
    eNTP_ERR,
    eNTP_STOP,

    eDNS_OK = 100,
    ePING_OK,
    eHTTP_OK,
    eNTP_OK,
};

typedef int (*ReportEvent_f)(int, int, void*);

class NetworkDiagnose : public Hippo::Thread {
public:
    class DiagnoseProcess {
    public:
        virtual ~DiagnoseProcess(){ }
        virtual int type() = 0;
        virtual int start(NetworkDiagnose* netdiag) = 0;
        virtual int stop() = 0;
        virtual int doResult(int result, int arg1, void* arg2 = 0) { return 0; }
        enum {
            eRun = 0,
            eFinish = 1,
            eStop = 2,
        };
    protected:
        DiagnoseProcess(){ };
        int mDiagState;
    };

    NetworkDiagnose();
    ~NetworkDiagnose(); 

    virtual void run();

    int testStart(DiagnoseProcess* diagnose);
    int testStop();

    void setTestType(const char* type) { mTestType = type; }
    void setTestState(int state) { mTestState = state; }
    void setTestResult(int result) { mTestResult = result; }
    void setErrorCode(int errorcode) { mErrorCode = errorcode; }

    int getTestState() { return mTestState; }
    int getTestResult() { return mTestResult; }
    int getErrorCode() { return mErrorCode; }
    const char* getTestType() { return mTestType.c_str(); }

    DiagnoseProcess* getDiagnose() { return mDiagnose; }

private:
    int mIsRun;
    int mPipeFds[2];

    std::string mTestType;
    std::string mHostIP;

    int mTestState;
    int mTestResult;
    int mErrorCode;

    DiagnoseProcess* mDiagnose;
};

NetworkDiagnose* networkDiagnose();

char* SplitUrl2Host(const char* u, char* s, int len, int* p);

#endif
#endif
