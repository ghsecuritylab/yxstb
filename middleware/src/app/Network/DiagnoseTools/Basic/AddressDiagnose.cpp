#include "NetworkAssertions.h"
#include "AddressDiagnose.h"
#include "NetworkInterface.h"

AddressDiagnose::AddressDiagnose() : mIface(0), mErrorCode(0)
{
}

AddressDiagnose::~AddressDiagnose()
{
    while (mDiagState != eFinish) 
        usleep(100000);
}

void 
AddressDiagnose::_InitSet()
{
    //address conflict configuration
    IPConflictSetting ipcset;
    ipcset.setReplyTime(3);
    ipcset.setConflictTime(1);
    ipcset.setUnconflictTime(3);
    mIface->setIPConflictSetting(ipcset);

    //DHCP configuration
    if (NetworkInterface::PT_DHCP == mIface->getProtocolType()) {
        DHCPSetting dhcpset = mDHCPConfBak;
        dhcpset.setRetryTimes(1);
        dhcpset.setReTransSeq("2,4,4"); //timeout: 10s = 2 + 4 + 4
        mIface->setDHCPSetting(dhcpset);
    } 

    //PPPoE configuration
    if (NetworkInterface::PT_PPPOE == mIface->getProtocolType()) {
        //TODO
    } 
}

void 
AddressDiagnose::_Restore()
{
    mIface->setIPConflictSetting(mConflictConfBak);
    switch (mIface->getProtocolType()) { //restore original mIface settings
    case NetworkInterface::PT_DHCP:
        mIface->setDHCPSetting(mDHCPConfBak);
        mIface->startCheckIP();
        break;
    case NetworkInterface::PT_PPPOE:
        mIface->setPPPSetting(mPPPConfBak);
        break;
    default:
        ;
    }
}

void 
AddressDiagnose::_Backup()
{
    mConflictConfBak = mIface->getIPConflictSetting();
    switch (mIface->getProtocolType()) { //bakup original mIface settings
    case NetworkInterface::PT_DHCP:
        mDHCPConfBak = mIface->getDHCPSetting();
        mIface->stopCheckIP();
        break;
    case NetworkInterface::PT_PPPOE:
        mPPPConfBak = mIface->getPPPSetting();
        break;
    default:
        ;
    }
}

int
AddressDiagnose::start(NetworkDiagnose* netdiag)
{
    NETWORK_LOG_INFO("diagnose:%s\n", netdiag->getTestType());

    mDiagState = eRun;

    if (!mIface) {
        netdiag->setTestResult(ND_RESULT_FAIL);
        netdiag->setTestState(ND_STATE_FAIL);
        mDiagState = eFinish;
        return -1;
    }

    if (mIface->isActive()) {
        if (NetworkInterface::PT_PPPOE == mIface->getProtocolType()) {
            netdiag->setTestState(ND_STATE_FINISH);
            netdiag->setTestResult(ND_RESULT_SUCCESS);
            mDiagState = eFinish;
            return 0;
        }
    }

    _Backup();

    _InitSet();

    mIface->connect(1);

    for (;;) { //wait for result.
        if (mDiagState != eRun)
            break;
        usleep(200000);
    }

    _Restore();

    netdiag->setTestState(ND_STATE_FINISH);
    netdiag->setErrorCode(mErrorCode);
    if (!mErrorCode)
        netdiag->setTestResult(ND_RESULT_SUCCESS);
    else
        netdiag->setTestResult(ND_RESULT_FAIL);

    mDiagState = eFinish; //end!
    return 0;
}

int
AddressDiagnose::stop()
{
    if (mDiagState == eRun)
        mDiagState = eStop;
    return 0;
}

int
AddressDiagnose::doResult(int result, int arg1, void* arg2)
{
    //TODO transform hyerr and hwerr to correct errorcode.
    if (result < 0) {
        switch (arg1) {
        default:
            mErrorCode = arg1;
        }
    }
    mDiagState = eFinish;
    return 0;
}
