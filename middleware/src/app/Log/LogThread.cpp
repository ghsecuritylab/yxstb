
#include "LogThread.h"

#include <unistd.h>


extern "C" void logInit();

namespace Hippo {

LogThread::LogThread()
	: Thread()
{
}

LogThread::~LogThread()
{
}

void 
LogThread::run() 
{
    logInit();

    Thread::run();
}

} /* namespace Hippo */

Hippo::LogThread* gLogThread = 0;

extern "C" void 
logThreadInit()
{
    gLogThread = new Hippo::LogThread();
    gLogThread->start();

    usleep(20 * 1000);
}
