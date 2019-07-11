
#include "MainThread.h"

#include "Message.h"
#include "MessageTypes.h"
#include "BrowserAgent.h"

#include "config/webpageConfig.h"


namespace Hippo {

MainThread::MainThread()
	: Thread(pthread_self())
{
}

MainThread::~MainThread()
{
}

} /* namespace Hippo */


Hippo::MainThread *gMainThread = NULL;

extern "C" void
mainThreadInit()
{
    gMainThread = new Hippo::MainThread();
    gMainThread->start();
}

extern "C" void
mainOpenUrl(const char *url)
{
    Hippo::epgBrowserAgent().sendEmptyMessageDelayed(MessageType_Timer, 10);
    if (url)
        Hippo::epgBrowserAgent().openUrl(url);
    else {
        #if defined(C30) 
            Hippo::epgBrowserAgent().openUrl(LOCAL_WEBPAGE_PATH_PREFIX"/boot/boot.html");
        #else
            Hippo::epgBrowserAgent().openUrl(LOCAL_WEBPAGE_PATH_PREFIX"/boot.html");
        #endif
    }
	
}

extern "C" void
mainThreadRun()
{
    gMainThread->run();
}

