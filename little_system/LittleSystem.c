#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#include "config.h"
#include "io_xkey.h"
#include "mid_middle.h"
#include "mid_sys.h"

#include "sys_msg.h"
#include "app_include.h"
#include "libzebra.h"

#include "MainThread.h"
#include "KeyDispatcher.h"
#include "NativeHandler.h"
#include "BrowserAgent.h"

static void *threadTakin(void *p)
{
    mid_sem_t psem = (mid_sem_t)p;
    signal(SIGPIPE, SIG_IGN);
    yhw_IRinputsystem_init();
    NativeHandlerSetState(9);
    io_xkey_reg((void*)sys_msg_port_irkey);
    mid_sem_give(psem);
    io_xkey_loop();
    while(1) {
        PRINTF("main loop\n");
        usleep(0x7fffffff);
    }
}

int main(int argc, char *argv[])
{
    mainThreadInit();
    logInit();
    defNativeHandlerCreate();
    keyDispatcherCreate();
    epgBrowserAgentCreate();
    settingManagerInit();
    settingManagerLoad(0);
    mid_sem_t sem = mid_sem_create();
    pthread_t pHandle;
    pthread_create(&pHandle, NULL, threadTakin, sem);
    mid_sem_take(sem, 0x0fffffff, 1);
    sendMessageToNativeHandler(0x400, LITTLE_SYSTEM_RUN, 0, 0);
    mainThreadRun();

    return 0;
}
