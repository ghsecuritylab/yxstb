#include "AgentHelper.h"

#include "config/webpageConfig.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/types.h>

extern int createJVMCallService();
extern "C" int epgBrowserAgentTestRestore(char* url);

static pid_t gChildPid = -1;
static int gPowerOffFlag = 0;

static void _BrowserExit(int signo)
{
    int stat;
    if (SIGCHLD == signo) {
        if (waitpid(gChildPid, &stat, WNOHANG) > 0) {
            fprintf(stderr, "_BrowserExit Child pid[%d] exit [%d]\n", gChildPid, stat);

            if (gPowerOffFlag) {
                gPowerOffFlag = 0;
                return;
            }

            //TODO How restart browser.elf

            Agent()->release();
            Agent()->initialize(1);

            if ( 0 == (gChildPid = fork())) {
#if defined(DEBUG_BUILD)
                if (execlp("browser.elf", "browser.elf", "1", NULL) < 0) //Debug Mode
#else
                if (execlp("browser.elf", "browser.elf", "0", NULL) < 0) //Safe Mode
#endif
                    perror("execl browser.elf");
                kill(0, SIGKILL); //normally , cannot run here!
            }

            if (gChildPid < 0)
                return ;

            char eBase[] = LOCAL_WEBPAGE_PATH_PREFIX"/eBase/customize/C03L/eBase.html";
            epgBrowserAgentTestRestore(eBase);
        }
    }
}

extern "C"
int MidwareAgentRun(void)
{
#if defined(DEBUG_BROWSER)
    return 0;
#endif
    int ret = -1;
    switch ((gChildPid = fork())) {
        case -1:
            perror("fork browser process");
            break;
        case 0:
#if defined(DEBUG_BUILD)
            if (execlp("browser.elf", "browser.elf", "1", NULL) < 0) //Debug Mode
#else
                if (execlp("browser.elf", "browser.elf", "0", NULL) < 0) //Safe Mode
#endif
                    perror("execl browser.elf, check PATH");
            break;
        default:
            signal(SIGCHLD, _BrowserExit);
            ret = 0;
    }
    createJVMCallService();
    return ret;
}

extern "C"
int KillBrowserForPowerOff()
{
    gPowerOffFlag = 1;
    kill(gChildPid, SIGKILL);
    return 0;
}
