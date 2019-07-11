#include "NetworkAssertions.h"
#include "GatewayDiagnose.h"
#include "NetworkInterface.h"
#include "NetworkTypes.h"

#include <sys/select.h>
#include <errno.h>
#include <fcntl.h> 
#include <string.h>

GatewayDiagnose::GatewayDiagnose() : mIface(0), mErrorCode(0)
{
}

GatewayDiagnose::~GatewayDiagnose()
{
    while (mDiagState != eFinish)
        usleep(100000);
}

int GatewayDiagnose::_SendARP()
{
    NETWORK_LOG_INFO("//TODO Run here\n");
    const int kTimeout = 10;
    int sfd = -1;
    int ret = -1, i = 0;
    char address[32] = { 0 };
    char gateway[32] = { 0 };
    char mac[32] = { 0 };
    struct timeval tv;
    fd_set rset;

    sfd = InitArpingSocket(mIface->ifname());
    if (sfd < 0)
        return -1;

    mIface->getAddress(address, 31);
    mIface->getGateway(gateway, 31);
    NETWORK_LOG_INFO("address:%s, gateway:%s\n", address, gateway);

    for (i=0;;) {
        if (mDiagState != eRun)
            break;
        if (i > kTimeout)
            break;
        FD_ZERO(&rset);
        FD_SET(sfd, &rset);
        tv.tv_sec = 3;
        tv.tv_usec = 0;
        i += tv.tv_sec;
        if (SendArpingPacket(sfd, address, gateway) < 0)
            break;
        ret = select(sfd + 1, &rset, 0, 0, &tv);
        if (-1 == ret) { //system call interrupt
            if (EINTR == errno)
                continue;
            return -1;
        }
        if (0 == ret) //timeout
            continue;
        if (FD_ISSET(sfd, &rset)) {
            if (1 == RecvArpingPacket(sfd, gateway, mac, 31)) {
                NETWORK_LOG_INFO("gateway[%s]:[%s]\n", gateway, mac);
                close (sfd);
                return 1;
            }
        }
    }
    close(sfd);
    return 0;
}

int GatewayDiagnose::_SendLCP()
{
    //TODO 
    int fd = -1, i = 10;
    char pipename[64] = {0};
    sprintf(pipename, "%s/check-%s.pipe", PPPOE_ROOT_DIR, mIface->ifname());
    if ((fd = open(pipename, O_RDWR | O_NONBLOCK, 0)) == -1) {
        if (errno == ENXIO)
            NETWORK_LOG_ERR("open fifo:%s\n", strerror(errno));
    }

    write(fd, &i, sizeof(int));
    sleep(2);
    write(fd, &i, sizeof(int));

    while (i--) {
        if (mDiagState != eRun)
            break;
        sleep(1);
    }
    close (fd);
    return mErrorCode ? -2 : 1;
}

int
GatewayDiagnose::start(NetworkDiagnose* netdiag)
{
    NETWORK_LOG_INFO("diagnose:%s\n", netdiag->getTestType());

    mDiagState = eRun;
    if (!mIface) {
        netdiag->setTestResult(ND_RESULT_FAIL);
        netdiag->setTestState(ND_STATE_FAIL);
        mDiagState = eFinish; 
        return -1;
    }

    int ret = -1;
    if (NetworkInterface::PT_PPPOE == mIface->getProtocolType())
        ret = _SendLCP(); //will block
    else
        ret = _SendARP(); //will block

    switch (ret) {
        case -2:
            netdiag->setErrorCode(102006);
            netdiag->setTestResult(ND_RESULT_FAIL);
            break;
        case -1:
            netdiag->setErrorCode(102031);
            netdiag->setTestResult(ND_RESULT_FAIL);
            break;
        case 0:
            netdiag->setErrorCode(102034);
            netdiag->setTestResult(ND_RESULT_FAIL);
            break;
        case 1:
            netdiag->setTestResult(ND_RESULT_SUCCESS);
            break;
        default:
            ;
    }
    netdiag->setTestState(ND_STATE_FINISH);

    mDiagState = eFinish; //end!
    return 0;
}

int
GatewayDiagnose::stop()
{
    if (mDiagState == ND_STATE_RUNNING)
        mDiagState = ND_STATE_FAIL;
    return 0;
}

int 
GatewayDiagnose::doResult(int result, int arg1, void* arg2)
{
    //TODO transform hyerr and hwerr to correct errorcode.
    if (result < 0)
        mErrorCode = arg1;
    mDiagState = eFinish;
    return 0;
}
