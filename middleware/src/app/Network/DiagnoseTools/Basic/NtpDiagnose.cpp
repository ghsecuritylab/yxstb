#include "NetworkAssertions.h"
#include "NtpDiagnose.h"

#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/select.h>

NtpDiagnose::NtpDiagnose(int mode) 
    : CentralAreaDiagnose(mode) 
{
}

NtpDiagnose::~NtpDiagnose()
{
    while (mDiagState != eFinish) 
        usleep(100000);
}

int 
NtpDiagnose::_AdoptSntp(int family)
{
    const int kTimeout = kNtpTimeout;
    const int kSendTimes = 3;

    int tCount = mTestUrl.size();
    unsigned char data[48] = { 0 }; // sntp head size, but only use the first byte.
    struct sockaddr_in srvr[kHostMaxCount];

    int ret = 0, i = 0, j = 0;
    fd_set rfds;     

    int port[kHostMaxCount] = {0};
    char hosts[kHostMaxCount][kHostUrlLength] = {{0}};
    for (i = 0; i < tCount; ++i)
        SplitUrl2Host(mTestUrl[i].c_str(), hosts[i], kHostUrlLength-1, &port[i]);

    ret = _DnsResolv(family, (char**)hosts, tCount);
    if (ret != eDNS_OK)
        return ret;

    int sockfd = socket(family, SOCK_DGRAM, 0);
    if (sockfd < 0)
        return eNTP_ERR;     

    data[0] = 0x0B; /* standard SNTP client request */
    for (j = 0; j < kSendTimes; ++j) {
        for (i = 0; i < tCount; i++) {
            if (!isResolvOK(i))
                continue;
            bzero(&srvr[i], sizeof(srvr[i]));
            srvr[i].sin_family = family;
            srvr[i].sin_port = htons(port[i]);
            inet_pton(family, hosts[i], &srvr[i].sin_addr);
            sendto(sockfd, data, sizeof(data), 0, (struct sockaddr*)&srvr[i], sizeof(srvr[i]));
        }
        sleep(1);
    }

    long total = 0;
    do {
        if (mDiagState != eRun) {
            ret = eHTTP_STOP;
            break;
        }
        if (total >= kTimeout) {
            ret = eNTP_TIMEOUT;
            break;
        }

        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        total += tv.tv_sec;

        FD_ZERO(&rfds);
        FD_SET(sockfd, &rfds);
        ret = select(sockfd + 1, &rfds, 0, 0, &tv);   
        if (-1 == ret) { //system call interrupt
            if (EINTR == errno)
                continue;
            break;
        } else if (!ret)
            continue;

        if (FD_ISSET(sockfd, &rfds)) {
            ret = recvfrom(sockfd, data, sizeof(data), 0, 0, 0);
            if (ret > 0) {
                ret = eNTP_OK;
                break;
            }
        }    	
    } while(1);

    close(sockfd);
    return ret;
}

int 
NtpDiagnose::start(NetworkDiagnose* netdiag)
{
    NETWORK_LOG_INFO("diagnose:%s\n", netdiag->getTestType());

    mDiagState = eRun;

    if (mTestUrl.size() <= 0) {
        netdiag->setTestState(ND_STATE_FAIL);
        netdiag->setTestResult(ND_RESULT_FAIL);
        mDiagState = eFinish;
        return 0;
    }

    int status, httpcode = 0;

    if (ND_MODE_ICMP == mDiagMode)
        status = _AdoptPing(AF_INET);
    else
        status = _AdoptSntp(AF_INET);

    NETWORK_LOG_INFO("ret = %d\n", status);
    switch (status) {
    case eDNS_ERR:
    case eDNS_NOADDR:
        netdiag->setTestState(ND_STATE_FAIL);
        netdiag->setTestResult(ND_RESULT_FAIL);
        netdiag->setErrorCode(102014);
        break;
    case eDNS_TIMEOUT:
        netdiag->setTestState(ND_STATE_FINISH);
        netdiag->setTestResult(ND_RESULT_FAIL);
        netdiag->setErrorCode(102013);
        break;
    case ePING_ERR:
        netdiag->setTestState(ND_STATE_FAIL);
        netdiag->setTestResult(ND_RESULT_FAIL);
        netdiag->setErrorCode(102040);
        break;
    case ePING_OK:
        netdiag->setTestState(ND_STATE_FINISH);
        if (100.0 == mSuccessRate) {
            if (mMaxDelay < 800.0)
                netdiag->setTestResult(ND_RESULT_SUCCESS);
            else {
                netdiag->setTestResult(ND_RESULT_FAIL);
                netdiag->setErrorCode(102038);
            }
        } else {
            netdiag->setTestResult(ND_RESULT_FAIL);
            if (0.0 == mSuccessRate)
                netdiag->setErrorCode(102040);
            else
                netdiag->setErrorCode(102039);
        }
        break;
    case eNTP_TIMEOUT:
    case eNTP_ERR:
        netdiag->setTestState(ND_STATE_FINISH);
        netdiag->setTestResult(ND_RESULT_FAIL);
        netdiag->setErrorCode(102011);
        break;
    case eNTP_OK:
        netdiag->setTestState(ND_STATE_FINISH);
        netdiag->setTestResult(ND_RESULT_SUCCESS);
        break;
    case eDNS_STOP:
    case ePING_STOP:
    case eNTP_STOP:
    default:
        netdiag->setTestState(ND_STATE_FAIL);
        netdiag->setTestResult(ND_RESULT_FAIL);
        break;
    }

    mDiagState = eFinish;
    return 0;
}

int 
NtpDiagnose::stop()
{
    if (mDiagState == eRun)
        mDiagState = eStop;
    return 0;
}
