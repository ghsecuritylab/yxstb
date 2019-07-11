
#include "TerminalControl.h"
#include "UserInformation.h"
#include "NativeHandler.h"
#include "MessageValueDebug.h"
#include "MessageTypes.h"

#include "Assertions.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

extern "C" int yos_systemcall_runSystemCMD(char *buf, int *ret);

static unsigned int SSHState = 0;
static unsigned int serialPortState = 1;//0 is close state, 1 is open state

#ifdef __cplusplus

extern "C" {

// Terminal: SerialProt
int getSerialPortPrintState()
{
    return serialPortState;
}

// open/close stdout,stderr;0:colse,1:open.
int setSerialPortPrintState(int flag)
{
    if (flag != 0 && flag != 1) {
        LogSafeOperError("Input parameter error flag=[%d]\n", flag);
        return -1;
    }

    static int saveStdoutFD = 0;
    static int saveStderrFD = 0;
    if (!saveStdoutFD && !saveStderrFD) {
        LogSafeOperDebug("Backup zhe stdout and stderr\n");
        saveStdoutFD = dup(1);
        saveStderrFD = dup(2);
    }

    if (serialPortState != flag) {
        serialPortState = flag;
        if (serialPortState) {
            LogSafeOperDebug("Open SerialPort\n");
            dup2(saveStdoutFD, 1);
            dup2(saveStderrFD, 2);
            close(saveStdoutFD);
            close(saveStderrFD);
            saveStdoutFD = 0;
            saveStderrFD = 0;
        } else {
            LogSafeOperDebug("Close SerialPort\n");
            int fd = open("/dev/null", O_WRONLY | O_CREAT);
            if (!fd) {
                LogSafeOperWarn("Open /dev/null error\n");
                return -1;
            }
            fflush(stdout);
            fflush(stderr);
            dup2(fd, 1);
            dup2(fd, 2);
            close(fd);
        }
    }
    return 0;
}

// Terminal: SSH
int getSSHState()
{
    return SSHState;
}

int setSSHState(int flag)
{
    printf("setSSHState is %d\n", flag);
    if(flag != 0 && flag != 1) {
        LogSafeOperError("Input parameter error\n");
        return -1;
    }

    if(SSHState != flag) {
        SSHState = flag;
        if(SSHState) {
            yos_systemcall_runSystemCMD("/usr/sbin/sshd", NULL);
        } else {
            yos_systemcall_runSystemCMD("killall -9 sshd", NULL);
            yos_systemcall_runSystemCMD("rm /var/run/sshd_locked_log", NULL);
        }
    }
    return 0;
}

void controlSSHDebug(void)
{
    sendMessageToNativeHandler(MessageType_Debug, MV_Debug_WatchLoggedUsers, 1, 1000);
    return;
}

} // extern "C"
#endif // __cplusplus

