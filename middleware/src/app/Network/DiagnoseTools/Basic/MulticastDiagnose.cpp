#include "NetworkAssertions.h"
#include "MulticastDiagnose.h"

#include <arpa/inet.h>
#include <sys/select.h>
#include <string.h>
#include <errno.h>

MulticastDiagnose::MulticastDiagnose()
    : mIGMPVersion(eIGMP_NON), mTimeout(5)
{
}

MulticastDiagnose::~MulticastDiagnose()
{
    while (mDiagState != eFinish)
        usleep(100000);
}

int 
MulticastDiagnose::_MulticastV4()
{
    NETWORK_LOG_INFO("V4!\n");

    int sockfd = -1, i;
    int ret = -1, status = eMulti_ADDRERR;
    struct sockaddr_in sin;

    fd_set rfds;
    struct timeval tv;
    char recv[1024] = { 0 };

    NETWORK_LOG_INFO("MultiAddr:%s, MultiPort:%d LocalAddr:%s\n", getMultiAddr(), getMultiPort(), getLocalAddr());

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        status = eMulti_SOCKERR;
        NETWORK_LOG_ERR("socket:%s\n", strerror(errno));
        goto END;
    }

    bzero(&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = inet_addr(getMultiAddr());;
    sin.sin_port = htons(getMultiPort());

    i = 1;
    ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i));
    if (ret < 0) {
        status = eMulti_SOCKERR;
        NETWORK_LOG_ERR("setsockopt:%s\n", strerror(errno));
        goto END;
    }

    bind(sockfd, (struct sockaddr*)&sin, sizeof(sin));

    //ADD
    if (eIGMP_V2 == mIGMPVersion) {
        struct ip_mreq ipmreq;
        ipmreq.imr_multiaddr.s_addr = inet_addr(getMultiAddr());
        ipmreq.imr_interface.s_addr = inet_addr(getLocalAddr());
        ret = setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &ipmreq, sizeof(ipmreq));
        if (ret < 0) {
            status = eMulti_SOCKERR;
            NETWORK_LOG_ERR("setsockopt:%s\n", strerror(errno));
            goto END;
        }
    } else {
        struct ip_mreq_source imr;
        ret = inet_pton(AF_INET, getSourceAddr(), &sin);
        if (ret <= 0)
            goto END;
        imr.imr_multiaddr.s_addr  = inet_addr(getMultiAddr());
        imr.imr_interface.s_addr  = inet_addr(getLocalAddr());
        imr.imr_sourceaddr.s_addr = inet_addr(getSourceAddr());
        ret = setsockopt(sockfd, IPPROTO_IP, IP_ADD_SOURCE_MEMBERSHIP, &imr, sizeof(imr));
        if (ret < 0) {
            status = eMulti_SOCKERR;
            NETWORK_LOG_ERR("setsockopt:%s\n", strerror(errno));
            goto END;
        }
    }

    //RECV
    status = eMulti_TIMEOUT;
    for(i=0;;) {
        if (mDiagState != eRun)
            break;
        if (i > mTimeout)
            break;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        i += tv.tv_sec;

        FD_ZERO(&rfds);
        FD_SET(sockfd, &rfds);
        ret = select(sockfd + 1, &rfds, 0, 0, &tv);
        if (-1 == ret) {
            if (EINTR == errno)
                continue;
            break;
        }
        if (!ret)
            continue;

        if (FD_ISSET(sockfd, &rfds)) {
            if (read(sockfd, recv, sizeof(recv)) > 0) {
                status = eMulti_OK;
                break;
            }
        }
    }

    //DROP
    if (eIGMP_V2 == mIGMPVersion) {
        struct ip_mreq ipmreq;
        ipmreq.imr_multiaddr.s_addr = inet_addr(getMultiAddr());
        ipmreq.imr_interface.s_addr = inet_addr(getLocalAddr());
        setsockopt(sockfd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &ipmreq, sizeof(ipmreq));
    } else {
        struct ip_mreq_source imr;
        imr.imr_multiaddr.s_addr  = inet_addr(getMultiAddr());
        imr.imr_interface.s_addr  = inet_addr(getLocalAddr());
        imr.imr_sourceaddr.s_addr = inet_addr(getSourceAddr());
        setsockopt(sockfd, IPPROTO_IP, IP_DROP_SOURCE_MEMBERSHIP, &imr, sizeof(imr));
    }

END:
    if (sockfd > 0)
        close(sockfd);
    return mDiagState == eRun ? status : eMulti_STOP;
}

int 
MulticastDiagnose::start(NetworkDiagnose* netdiag)
{
    NETWORK_LOG_INFO("diagnose:%s\n", netdiag->getTestType());

    mDiagState = eRun;

    if (eIGMP_NON == mIGMPVersion) {
        if (mSourceAddr.empty())
            mIGMPVersion = eIGMP_V2;
        else 
            mIGMPVersion = eIGMP_V3;
    }
    int status = _MulticastV4();
    NETWORK_LOG_INFO("status: %d\n", status);
    switch (status) {
    case eMulti_OK:
        netdiag->setTestState(ND_STATE_FINISH);
        netdiag->setTestResult(ND_RESULT_SUCCESS);
        break;
    case eMulti_ADDRERR:
    case eMulti_SOCKERR:
    case eMulti_TIMEOUT:
    case eMulti_STOP:
        netdiag->setTestState(ND_STATE_FINISH);
        netdiag->setErrorCode(102041);
        netdiag->setTestResult(ND_RESULT_FAIL);
    default:
        ;
    }

    mDiagState = eFinish;
    return 0;
}

int 
MulticastDiagnose::stop()
{
    if (mDiagState == eRun)
        mDiagState = eStop;
    return 0;
}
