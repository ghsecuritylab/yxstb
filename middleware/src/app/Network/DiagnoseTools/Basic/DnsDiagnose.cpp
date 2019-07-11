#include "NetworkAssertions.h"
#include "DnsDiagnose.h"

#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/select.h>

DnsDiagnose::DnsDiagnose() 
    : CentralAreaDiagnose() 
{
}

DnsDiagnose::~DnsDiagnose()
{
    while (mDiagState != eFinish) 
        usleep(100000);
}

int 
DnsDiagnose::start(NetworkDiagnose* netdiag)
{
    NETWORK_LOG_INFO("diagnose:%s\n", netdiag->getTestType());

    mDiagState = eRun;

    if (mTestUrl.size() <= 0) {
        netdiag->setTestState(ND_STATE_FAIL);
        netdiag->setTestResult(ND_RESULT_FAIL);
        mDiagState = eFinish;
        return 0;
    }

    int tCount = mTestUrl.size();
    int status = 0, i = 0;
    char *hosts[kHostMaxCount] = {0};

    for (i = 0; i < tCount; ++i) {
        hosts[i] = (char*)calloc(1, kHostUrlLength);
        if (hosts[i])
            SplitUrl2Host(mTestUrl[i].c_str(), hosts[i], kHostUrlLength-1, 0);
    }
    status = _DnsResolv(AF_INET, hosts, tCount);
    for (i = 0; i < tCount; ++i) {
        if (hosts[i])
            free(hosts[i]);
    }
    switch (status) {
    case eDNS_OK:
        netdiag->setTestState(ND_STATE_FINISH);
        netdiag->setTestResult(ND_RESULT_SUCCESS);
        break;
    case eDNS_ERR:
        netdiag->setTestState(ND_STATE_FAIL);
        netdiag->setErrorCode(102044);
        break;
    case eDNS_NOADDR:
        netdiag->setTestState(ND_STATE_FAIL);
        netdiag->setErrorCode(102042);
        break;
    case eDNS_TIMEOUT:
        netdiag->setTestState(ND_STATE_FINISH);
        netdiag->setTestResult(ND_RESULT_FAIL);
        netdiag->setErrorCode(102043);
        break;
    case eDNS_STOP:
    default:
        netdiag->setTestState(ND_STATE_FAIL);
        break;
    }

    mDiagState = eFinish;
    return 0;
}


int 
DnsDiagnose::stop()
{
    if (mDiagState == eRun)
        mDiagState = eStop;
    return 0;
}
