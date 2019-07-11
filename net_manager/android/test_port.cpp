#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <utils/Log.h>
#include "HybroadService.h"

namespace android {


extern HybroadService *pHybroad;

int test_service()
{
	//启动service，等待注册listener
    pHybroad = new HybroadService();
    sp<IServiceManager> sm = defaultServiceManager();
    sm->addService(String16("com.hybroad.stb"), pHybroad);
	sp<ProcessState>proc(ProcessState::self());
    ProcessState::self()->startThreadPool();
    IPCThreadState::self()->joinThreadPool();

	return 0;
}
}

int use_service()
{
    android::test_service();
	return 0;
}
