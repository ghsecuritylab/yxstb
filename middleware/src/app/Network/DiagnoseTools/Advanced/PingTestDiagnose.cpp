#include "NetworkAssertions.h"
#include "PingTestDiagnose.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <netdb.h>
#include <errno.h>
#include <sys/select.h>

#include "libcares/ares.h"
#include "oping.h"

static void _Callback(void *arg, int status, int timeouts, struct hostent *host);
static void _ShowInfo(PingPacketInfo_s* info);

PingTestDiagnose::PingTestDiagnose(ReportEvent_f fun) 
    : mResolvIP("") , mHostName("")
    , mPacketSize(32), mPacketCount(4)
    , mTimeoutMs(4000), mTTL(255)
    , mPacketSendCount(0), mPacketRecvCount(0)
    , mPacketTimeoutCount(0), mPacketErrorCount(0)
    , mMaxDelay(-0.1), mMinDelay(-0.1), mAverageDelay(0.0)
    , _ReportEvent(fun)
{
}

PingTestDiagnose::~PingTestDiagnose()
{
    while (mDiagState != eFinish)
        usleep(100000);
}

int 
PingTestDiagnose::_DnsResolv(int family, const char* hostname)
{
    NETWORK_LOG_INFO("family:%d, hostname:%s\n", family, hostname);
    const int kTimeout = 5;
    int i = 0, ret = -1, status = -1, nfds = 0;
    fd_set rfds, wfds;
    struct timeval tv, maxtv;
    ares_channel channel;
    struct ares_options options;

    ret = ares_library_init(ARES_LIB_INIT_ALL);
    if (ret != ARES_SUCCESS)
        return eDNS_ERR;

    options.timeout = 500; //ms
    options.tries = 2;
    ret = ares_init_options(&channel, &options, ARES_OPT_TIMEOUTMS | ARES_OPT_TRIES);
    if (ret != ARES_SUCCESS) {
        ares_library_cleanup();
        return eDNS_ERR;
    }

    ares_gethostbyname(channel, hostname, family, _Callback, this);

    status = eDNS_TIMEOUT;
    for (i=0;;) {
        if (mDiagState != eRun)
            break;
        if (i > kTimeout)
            break;
        maxtv.tv_sec = kTimeout;
        maxtv.tv_usec = 0;
        ares_timeout(channel, &maxtv, &tv);
        i += tv.tv_sec;

        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        nfds = ares_fds(channel, &rfds, &wfds);
        if (nfds == 0)
            break;
        select(nfds, &rfds, &wfds, 0, &tv);
        ares_process(channel, &rfds, &wfds);
    }
    ares_library_cleanup();
    ares_destroy(channel);
    if (!mResolvIP.empty())
        status = eDNS_OK;
    return mDiagState == eRun ? status : ePING_STOP;
}

int 
PingTestDiagnose::_PingTest(int family)
{
    NETWORK_LOG_INFO("PingTest!\n");

    int status = 0, i = 0;
    size_t bufflen = 0;
    double latency = 0.0, total = 0.0;
    double timeout = mTimeoutMs / 1000.0; //timeout per packet.
    char* packet = 0;
    PingPacketInfo_s* info = 0, *front = 0;
    pingobj_t *ping = 0;
    pingobj_iter_t *iter = 0;

    packet = (char*)calloc(1, mPacketSize + 1);
    if (!packet)
        goto END;
    memset(packet, 'x', mPacketSize); //ignore the content of packet

    status = ePING_ERR;
    if ((ping = ping_construct()) == 0)
        goto END;
    if (ping_setopt(ping, PING_OPT_DATA, packet) < 0)
        goto END;
    if (ping_setopt(ping, PING_OPT_TIMEOUT, &timeout) < 0)
        goto END;
    if (ping_setopt(ping, PING_OPT_TTL, &mTTL) < 0)
        goto END;
    if (ping_host_add(ping, mResolvIP.c_str()) < 0)
        goto END;

    status = ePING_OK;
    do {
        if (mDiagState != eRun)
            goto END;
        if (ping_send(ping) < 0)
            goto END;

        if (!(iter = ping_iterator_get(ping)))
            goto END;

        latency = -0.1;
        info = (PingPacketInfo_s*)calloc(1, sizeof(PingPacketInfo_s));
        if (!info) 
            goto END;

        bufflen = sizeof(latency);
        ping_iterator_get_info(iter, PING_INFO_LATENCY, &latency, &bufflen);
        bufflen = sizeof(info->nPacketNumber);
        ping_iterator_get_info(iter, PING_INFO_SEQUENCE, &info->nPacketNumber, &bufflen);
        bufflen = sizeof(info->nHostAddr);
        ping_iterator_get_info(iter, PING_INFO_ADDRESS, info->nHostAddr, &bufflen);
        bufflen = sizeof(info->nRecvTTL);
        ping_iterator_get_info(iter, PING_INFO_RECV_TTL, &info->nRecvTTL, &bufflen);

        mPacketSendCount++;
        if (latency > 0.0) {
            mPacketRecvCount++;
            ping_iterator_get_info(iter, PING_INFO_DATA, 0, &info->nPacketSize);
            total += latency;
            if (mMaxDelay < 0.0 || mMaxDelay < latency)
                mMaxDelay = latency;
            if (mMinDelay < 0.0 || mMinDelay > latency)
                mMinDelay = latency;
            info->nResponsTime = latency;
        } else {
            //TODO need check icmp_type(not ECHOREPLY), but the relevant function is not supplied yet.
            info->nRecvTTL > 0 ? mPacketTimeoutCount++ : mPacketErrorCount;
        }
        if (MAX_SAVE_COUNT == (mPacketInfos.size() + 1)) {
            front = mPacketInfos.front();
            if (front)
                free(front);
            front = 0;
            mPacketInfos.pop_front();
        }
        mPacketInfos.push_back(info);
        sleep(1);
        if (_ReportEvent) //For old epg, send 0x300.
            _ReportEvent(type(), (100*mPacketSendCount)/mPacketCount, info);
        //_ShowInfo(info);
    } while (mPacketSendCount < mPacketCount);

    if (mPacketRecvCount > 0.0) {
        mAverageDelay = total / mPacketRecvCount;
        mSuccessRate = 100 * mPacketRecvCount / mPacketSendCount;
    }

END:
    if (packet)
        free(packet);
    ping_destroy (ping);
    return mDiagState == eRun ? status : ePING_STOP;
}

int 
PingTestDiagnose::start(NetworkDiagnose* netdiag)
{
    NETWORK_LOG_INFO("Start:%s\n", netdiag->getTestType());

    mDiagState = eRun;

    int status = 0;
    status = _DnsResolv(AF_INET, mHostName.c_str());
    
    if (status == eDNS_OK) {
        switch (_PingTest(AF_UNSPEC)) {
        case ePING_STOP:
        case ePING_OK:
            netdiag->setTestState(ND_STATE_FINISH);
            netdiag->setTestResult(ND_RESULT_SUCCESS);
            break;
        case ePING_ERR: 
        default:
            netdiag->setErrorCode(102063);
            netdiag->setTestState(ND_STATE_FAIL);
            netdiag->setTestResult(ND_RESULT_FAIL);
        }
    } else {
        netdiag->setTestState(ND_STATE_FAIL);
        netdiag->setTestResult(ND_RESULT_FAIL);
        netdiag->setErrorCode(102061);
    }

    mDiagState = eFinish;

    NETWORK_LOG_INFO("End:%s\n", netdiag->getTestType());
    return 0;
}

int 
PingTestDiagnose::stop()
{
    NETWORK_LOG_INFO("diagnose:%d\n", type());
    if (mDiagState == eRun)
        mDiagState = eStop;
    return 0;
}

int
PingTestDiagnose::doResult(int result, int arg1, void* arg2)
{
    NETWORK_LOG_INFO("Resolve[%d]: %s\n", arg1, (char*)arg2);
    if (result == ARES_SUCCESS)
        mResolvIP = (char*)arg2;
    return 0;
}

static void _Callback(void *arg, int status, int timeouts, struct hostent *host)
{
    if (!arg)
        return;

    char **list;
    char addr[46] = { 0 };
    PingTestDiagnose* diagnose = (PingTestDiagnose*)arg;

    if (status == ARES_SUCCESS) {
        for (list = host->h_addr_list; *list; list++) {
            if (ares_inet_ntop(host->h_addrtype, *list, addr, sizeof(addr)))
                break;
        }
    }
    diagnose->doResult(status, 0, addr); 
}

static void _ShowInfo(PingPacketInfo_s* info)
{
    printf("\n");
    printf("%d bytes from %s: seq=%d ttl=%d time=%.3f ms", info->nPacketSize, info->nHostAddr, info->nPacketNumber, info->nRecvTTL, info->nResponsTime);
    printf("\n");
}
