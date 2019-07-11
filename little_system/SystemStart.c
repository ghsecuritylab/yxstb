#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>

int main(int argc, char *argv[])
{
    pid_t childpid;
    pid_t childSqmpid;	
    int status;
    
    while (1) {
        childpid = fork();
        if (childpid == -1) {
            break;
        } else if (childpid == 0) {
            char *argv1[] ={"littleSystem.elf" , 0};
            execvp("littleSystem.elf", argv1);
        } else {
            waitpid(childpid, &status, 0);
            printf("child progress exit\n");
            if (WIFEXITED(status)) {
                childpid = fork();
                if (childpid == -1) {
                    break;
                } else if (childpid == 0) {
                    childSqmpid = fork();
                    if (childSqmpid == -1) {
                        break;
                    } else if (childSqmpid == 0) {
                        char *argv2[] ={"sqm.elf" , 0};
                        execvp("sqm.elf", argv2);
                        exit(0);
                    } else {
                        sleep(5);//zm add , sqm.elf must run befor iptv_B200.efl
                        char *argv3[] ={"iptv_B200.elf" , 0};
                        execvp("iptv_B200.elf", argv3);
                        exit(0);
                    }
                } else {
                    waitpid(childpid, &status, 0);
                    if (WIFEXITED(status)) {
                        system("killall -9 browser.elf");
                        system("killall -9 iptv_B200.elf");
                        system("killall -9 sqm.elf");
                        system("killall -9 sqmpro")	;
#ifdef INCLUDE_DLNA
                        system("killall -9 FASTDMRAPP");
#endif			
                        sleep(2);
                        continue;
                    } else {
                        break;
                    }
                }
            } else {
                break;
            }  
        }
    }
    return 0;
}