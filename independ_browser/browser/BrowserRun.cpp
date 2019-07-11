#include "AgentHelper.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <signal.h>
#include <fcntl.h>

extern int gIpcModuleLevel;
int main(int argc, char** argv)
{
#if !defined(BROWSER_ROOT_RUN)
    struct passwd* user = 0; 
    if (user = getpwnam("nobody")) {
        if (!setuid(user->pw_uid))
            printf("setuid %d ok\n", user->pw_uid);  
        if (!setgid(user->pw_gid))
            printf("setgid %d ok\n", user->pw_gid);
    }
#endif
    //Log output
    if (2 == argc) {
        if (0 == atoi(argv[1])) {
            int fd = open("/dev/null", O_WRONLY|O_CREAT);
            dup2(fd, 1);
            dup2(fd, 2);
            close(fd);
        } else {
            char* lstr = getenv("logipc");
            if (lstr) {
                int level = atoi(lstr);
                if (-1 < level && level < 4) {
                    printf("---------------------->Browser IPC debug[[%d]\n", level);
                    gIpcModuleLevel = level;
                }
            }
        }
    }
    while (true) {
        Agent()->wait();
        if (0 != Agent()->serve())
            break;
    }
    printf("==========================BrowserRun Quit======================\n");
    return 0;
}
