#include "NetworkAssertions.h"
#include "PhysicalNetDiagnose.h"
#include "NetworkCard.h"
#include "NetworkTypes.h"

PhysicalNetDiagnose::PhysicalNetDiagnose()
{
}

PhysicalNetDiagnose::~PhysicalNetDiagnose()
{
}

int 
PhysicalNetDiagnose::start(NetworkDiagnose* netdiag)
{
    NETWORK_LOG_INFO("diagnose:%s\n", netdiag->getTestType());

    if (mDevice && NL_FLG_RUNNING == mDevice->linkStatus())
        netdiag->setTestResult(ND_RESULT_SUCCESS);
    else
        netdiag->setTestResult(ND_RESULT_FAIL);
    netdiag->setTestState(ND_STATE_FINISH);
    return 0;
}

int 
PhysicalNetDiagnose::stop()
{
    return 0;
}

