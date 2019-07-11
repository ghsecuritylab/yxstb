/*
    filename : sqm_main.c create date: 2010.12.7
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wait.h>
#include <errno.h>
#include <stdarg.h>
#include <dirent.h>

#include "sqm_port.h"
#include "sqm_types.h"
#include "probe_external_api_C28.h"


#define SQM_PRINT(x...) \
	do{ \
        char w_buf[256] = { 0 }; \
		sprintf ( w_buf, "[SQM]:[%s]:[%4d]", __FILE__, __LINE__ ); \
        write( fd_info_w, w_buf, 256 ); \
        bzero( w_buf, 256 ); \
        sqm_sent_log( w_buf, x ); \
        write( fd_info_w, w_buf, 256 ); \
	}while(0)


static int fd_info_w;

/*
 * ===  FUNCTION  ===============================================================
 *         Name:  sqm_sent_log
 *  Description:  å°†ç‹¬ç«‹æ¨¡å—sqm.elfæ—¥å¿—(å«æœ‰å˜å‚)æ ¼å¼åŒ–ä¸ºå­—ç¬¦ä¸².
 *        Input:  str : æŒ‡å‘æ ¼å¼åŒ–åçš„å­—ç¬¦ä¸²; fmt & ... : å˜å‚ & å˜é‡
 *       Output:  æ ¼å¼åŒ–åçš„å­—ç¬¦ä¸²çš„é•¿åº¦.
 *        Other:  åœ¨SQM_PRINTå®å‡½æ•°ä¸­è¢«è°ƒç”¨.
 *       Author:  Michael           03/09/2011 04:47:36 PM
 * ==============================================================================
 */
int sqm_sent_log(char *str, char *fmt, ... )
{
	int count;
	int size = 256;

	va_list ap;
	va_start(ap, fmt);
	count = vsnprintf(str, size, fmt, ap);
	va_end(ap);
	return count;
}

/*
//ÏÖÔÚÒªÇóÍ³Ò»Â·¾¶£¬ËùÒÔ±àÒë²»¹ıµÄÈ¥ĞŞ¸Ärootfs
¸ñÊ½: sqmloaderÂ·¾¶  -c  sqm.iniÂ·¾¶ -s ±£´æsqmproÂ·¾¶
Àı×Ó:sqmloader -c  /var/sqm.ini  -s  /root
*/
static void startSqmpro(void)
{
    SQM_PRINT("Child process PID(%d)\n", getpid());
    system("sqmloader -c  /var/sqm.ini  -s  /root");
    SQM_PRINT("sqmloader -c  /var/sqm.ini  -s  /root\n");
    return;
}

int main(int argc, char *argv[])
{
	int ret;
	int fd_mid_to_sqm;
	int size;
	int paralen;
	int leftlen;
	int flag_sqm_start = 0; // to indicate whether sqm start interface is called
	unsigned char readbuf[MAX_MSG_LEN];
	fd_set fdSets;
	static pid_t pid_sqm;

	/* Eagle. add 2011å¹´01æœˆ19æ—¥ */
	unsigned int sqm_listen_port = 0;
	unsigned int sqm_server_port = 0;

	umask(0);

	/* Eagle. add for using iptv.elf's telnet debug  at 02/18/2011 11:33:18 AM */
	if (-1 == access(TELNET_FIFO, F_OK)) {
		mkdir(DEFAULT_PIPE_DATAPATH,0755);
		if (mkfifo(TELNET_FIFO, FIFO_MODE) < 0) {
			perror("mkfifo error!");
		}
	}
	fd_info_w = open(TELNET_FIFO, O_RDWR | O_NONBLOCK);
	if (-1 == fd_info_w) {
		perror("open fifo error !");
		exit(-1);
	}

	SQM_PRINT("SQM is running, build time is %s %s\n", __TIME__, __DATE__);
	ret = mknod(MID_TO_SQM, S_IFIFO | 0666, 0);
	if (ret == -1) {
		SQM_PRINT("error to create fifo!\n");
		//exit(-1);
	}

	fd_mid_to_sqm = open(MID_TO_SQM, 0666);
	if (fd_mid_to_sqm == -1) {
		SQM_PRINT("error to open fifo: %s\n", MID_TO_SQM);
		exit(-1);
	}

	while(1) {
		FD_ZERO(&fdSets);
		FD_SET(fd_mid_to_sqm, &fdSets);

		ret = select(fd_mid_to_sqm + 1, &fdSets, NULL, NULL, NULL);
		if (ret > 0) {
			if (FD_ISSET(fd_mid_to_sqm, &fdSets)) {
				memset(readbuf, 0, MAX_STR_LEN);
				size = read(fd_mid_to_sqm, readbuf, 3);
				if (size != 3) {
					SQM_PRINT("Can NOT read enough data!\n");
					continue;
				}
				paralen = (readbuf[1]<<8) + readbuf[2];
				size = read(fd_mid_to_sqm, &readbuf[3], paralen);
				leftlen = paralen;
				while(leftlen > size) {
					leftlen -= size;
					SQM_PRINT("read again, left %d bytes\n", leftlen);
					sleep(1);
					size = read(fd_mid_to_sqm, &readbuf[paralen - leftlen + 3], leftlen);
				}
				switch(readbuf[0]) {
				case SQM_CMD_START_MONITOR:
                    SQM_PRINT("receive start monitor command!\n");
                    SQM_PRINT("sqm_main::Main PID = %d\n", getpid());
                    pid_sqm = vfork();
                    if(pid_sqm == -1) {
						SQM_PRINT("sqm_main:: create sub process fault.\n");
					}else if(pid_sqm == 0){  // child
                        startSqmpro();
                        exit(0);
					} else { // fatrher
						wait(NULL);
						SQM_PRINT("sqm_main::parent process create sub process successfuly.\n");
				       	SQM_PRINT("sqm_main:: Parent process PID = %d,pid_sqm=%d \n", getpid(),pid_sqm);
					}
					break;
				case SQM_CMD_STOP_MONITOR:
					SQM_PRINT("receive stop monitor command!\n");
					system("killall -9 sqmloader");
					system("killall -9 sqmpro");
					break;
				default:
					SQM_PRINT("Unknown sqm command: %#x!\n", readbuf[0]);
				}
			}
		}else{
			SQM_PRINT("time out!\n");
		}
		SQM_PRINT("sqm program running!\n");
		sleep(1);
	}
	return 0;
}


