#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <signal.h>
#include "jvm_porting.h"
#include <errno.h>

#include "Assertions.h"

static pid_t g_jvm_pid = -1;

static int jvm_boot_pid = -1;
static int jvm_boot_pid_opp = -1;
static int g_ctrl_fd[2];

void Jvm_Main_Close(void)
{
    int nouse = -1;
    write(g_ctrl_fd[1], &nouse, sizeof(nouse));
#if defined(BROWSER_INDEPENDENCE) || defined(ANDROID)
    JVMDestroyLayer(0);
#endif
	return;
}

void Jvm_Main_Running(void)
{
    PRINTF("Main PID = %d\n", getpid());
    system("rm -r /var/jvm*");
    if (-1 == pipe(g_ctrl_fd))
        return;
    pid_t pid = fork();
    if(pid == -1) {
        PRINTF("create sub process fault.\n");
    }
    else if(pid) {
        PRINTF("Parent process PID = %d\n", getpid());
    }
    else { //子进程
        PRINTF("Child process PID = %d\n", getpid());
        umask(0);
        if(mknod(JVM_BOOT_INFO, S_IFIFO | 0666, 0) != 0) {
            PRINTF("mknod err\n");
            exit(-1);
        }
        if(mknod(JVM_BOOT_INFO_OPP, S_IFIFO | 0666, 0) != 0) {
            PRINTF("mknod err\n");
            exit(-1);
        }
        jvm_boot_pid = open(JVM_BOOT_INFO, 0666);
        jvm_boot_pid_opp = open(JVM_BOOT_INFO_OPP, 0666);
        PRINTF("jvm_boot_pid = %d,\n", jvm_boot_pid);
        if(jvm_boot_pid == -1) {
            perror("jvm boot pipe: create named fifo fault:");
            exit(-1);
        }
        fd_set fdSets;
        char buf[MAX_JVM_STRING_COUNT][MAX_JVM_STRING_LEN];
        int ret, size;
        while(1) {
            FD_ZERO(&fdSets);
            FD_SET(g_ctrl_fd[0], &fdSets);
            FD_SET(jvm_boot_pid, &fdSets);
            int maxfd = g_ctrl_fd[0] > jvm_boot_pid ? g_ctrl_fd[0] : jvm_boot_pid;
            ret = select(maxfd + 1, &fdSets, NULL, NULL, NULL);
            if(ret > 0) {
                if(FD_ISSET(g_ctrl_fd[0], &fdSets)) {
                    int cmd = -1;
                    read(g_ctrl_fd[0], &cmd, sizeof(cmd));
                    if (g_jvm_pid > 0) {
                        kill(g_jvm_pid, SIGKILL);
                        waitpid(g_jvm_pid);
                    }
                    g_jvm_pid = -1;
                    exit(0);
                }  else if(FD_ISSET(jvm_boot_pid, &fdSets)) {
                    size = read(jvm_boot_pid, (*buf), MAX_JVM_STRING_LEN * MAX_JVM_STRING_COUNT);
                    if(size != MAX_JVM_STRING_LEN * MAX_JVM_STRING_COUNT) {
                        usleep(200000);
                        continue;
                    }
                    int i = 0;
                    for(; i < MAX_JVM_STRING_COUNT; i++) {
                        PRINTF("%s\n", buf[i]);
                    }
                    pid_t jvm_pid = vfork();
                    if(jvm_pid == -1) {
                        PRINTF("create sub process fault.\n");
                    } else if(jvm_pid) {
                        g_jvm_pid = jvm_pid;
                        PRINTF("parent process create sub process successfuly.\n");
                    } else { //子进程
                        int child_pid = getpid();
                        if(jvm_boot_pid_opp > 0)
                            write(jvm_boot_pid_opp, &child_pid, 4);
                        execl(buf[0],
                              buf[1],
                              buf[2],
                              buf[3],
                              buf[4],
                              buf[5],
                              buf[6],
                              buf[7],
                              buf[8],
                              buf[9],
                              buf[10],
                              buf[11],
                              buf[12],
                              buf[13],
                              buf[14],
                              buf[15],
                              buf[16],
                              buf[17],
                              NULL
                             );
                    }
                }
            } else {
                PRINTF("timeout\n");
                // time out
            }
            PRINTF("sub process running\n");
            sleep(1);
        }
    }
    return;
}


