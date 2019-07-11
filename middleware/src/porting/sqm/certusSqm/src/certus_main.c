/*******************************************************************************
*	启动和停止赛特斯质量监控系统
*
*	sunqiquan
*	2011.06.18
********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>
#include <unistd.h>

#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "config/pathConfig.h"

#define FULL_NAME_PATH_PIPE DEFAULT_PIPE_DATAPATH"/certus_qos"

enum {
    CERTUS_START = 0,
	CERTUS_STOP,
};

int createCertusQosMsgq(void)
{
    if (access(FULL_NAME_PATH_PIPE, R_OK | W_OK) == -1) {
		printf("access file, file=%s", FULL_NAME_PATH_PIPE);
        if ((mkfifo(FULL_NAME_PATH_PIPE, O_CREAT | O_EXCL) < 0)) {
    		if (errno != EEXIST) {
	    		printf("[%s] create fifo error\n", FULL_NAME_PATH_PIPE);
		    	goto mkfifo_err;
    		}
	    	printf("[%s] fifo file has found\n", FULL_NAME_PATH_PIPE);
    	}
	}

	int rfd = open(FULL_NAME_PATH_PIPE, O_RDWR | O_NONBLOCK, 0);
	if (rfd < 0)
		goto mkfifo_err;

	return rfd;
mkfifo_err:
	printf("create msg queue error: %s ", FULL_NAME_PATH_PIPE);
	perror("");
	return -1;
}

int getCertusQosMsgq(int rfd, int *msg)
{
	if (msg == NULL)
		return -1;

	if (read(rfd, msg, sizeof(int)) != sizeof(int));
    	return -1;

    return 0;
}

//check
static int isCertusQosRun(char *run_name)
{
	struct dirent *dirp;
    DIR *dp;
    int fd;
    char path[32];
    char buff[128];

	if ((dp = opendir("/proc")) == NULL) {
    	printf("read directory failed!\n");
        return;
    }

	while ((dirp = readdir(dp)) != NULL) {
		if (strcmp(dirp->d_name, ".") == 0 ||
		  strcmp(dirp->d_name, "..") == 0 ||
		  dirp->d_type != DT_DIR)
			continue;
		if (atoi(dirp->d_name) != 0) {
			sprintf(path, "/proc/%s/cmdline", dirp->d_name);
			if ((fd = open(path, O_RDONLY)) != -1) {
				memset(buff, 0, 128);
				read(fd, buff, 128);
				close(fd);
				if (strstr(buff, run_name)) {
					closedir(dp);
					return atoi(dirp->d_name);
				}
			}
		}
	}
	closedir(dp);

	return 0;
}

//opt
static void startQosMonLoader()
{
	pid_t pid;

	if ((pid = fork()) < 0)
		printf("fork() error!");
	else if (pid == 0) {
#if defined(brcm7405)
#elif defined(hi3560e)
        if (isCertusQosRun("QosMonLoader") > 0) {
            printf("\n\n\n\nother--------restart------------\n\n\n\n");
            execlp("/home/hybroad/bin/QosMonLoader", "QosMonLoader", "-C", DEFAULT_MODULE_CERTUS_DATAPATH"/certus.cfg", "-S", DEFAULT_MODULE_CERTUS_DATAPATH, "-R", (char *)0);
            printf("errno %d:%s\n", errno, strerror(errno));
        } else {
            printf("\n\n\n\nother--------start------------\n\n\n\n");
            execlp("/home/hybroad/bin/QosMonLoader", "QosMonLoader", "-C", DEFAULT_MODULE_CERTUS_DATAPATH"/certus.cfg", "-S", DEFAULT_MODULE_CERTUS_DATAPATH, (char *)0);
            printf("errno %d:%s\n", errno, strerror(errno));
        }
#else
#endif
	} else {
		int status;
		waitpid(pid, &status, WNOHANG);
	}
}

static void stopQosMonLoader()
{
	pid_t pid;
	if ((pid = isCertusQosRun("QosMonLoader")) > 0)
		kill(pid, SIGTERM);
}

int main(void)
{
	struct timeval tv;
	fd_set rfds;
	int msg_id = -1;

	signal(SIGCHLD,SIG_IGN);

	int certus_fd = createCertusQosMsgq();

	while(1) {
		tv.tv_sec = 60 * 60;
		tv.tv_usec = 0;
		FD_ZERO(&rfds);
		FD_SET(certus_fd, &rfds);

		printf("\n\n\n\nother--------before select------------\n\n\n\n");
    	if ((select(certus_fd + 1, &rfds, NULL, NULL, &tv) > 0) && FD_ISSET(certus_fd, &rfds)) {
	        printf("\n\n\n\nother--------after select------------\n\n\n\n");

    		if (getCertusQosMsgq(certus_fd, &msg_id) <= 0)
	    		continue;

	    	printf("\n\n\n\nother--------recv msg = %d------------\n\n\n\n", msg_id);
		    switch(msg_id) {
			    case CERTUS_START:
				    printf("before start_certus_qos---------\n\n\n");
    				startQosMonLoader();
	    			printf("after start_certus_qos---------\n\n\n");
		    		break;
			    case CERTUS_STOP:
				    stopQosMonLoader();
    				break;
	    		default:
		    		printf("wrong messages id!\n");
    		}
	    }
	}
	return 0;
}


