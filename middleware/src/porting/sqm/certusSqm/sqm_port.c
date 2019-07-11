/*******************************************************************************
*	启动和停止赛特斯质量监控系统
*
*	sunqiquan
*	2011.06.18
********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>
#include <errno.h>

#include "log_sqm_port.h"
#include "SysSetting.h"
#include "VersionSetting.h"
#include "sqm_port.h"
#include "config/pathConfig.h"

#define FULL_NAME_PATH_PIPE DEFAULT_PIPE_DATAPATH"/certus_qos"

enum {
    CERTUS_START = 0,
	CERTUS_STOP,
};

static void putCertusMsg(int msg);

int startCertusQos(void)
{
    putCertusMsg(CERTUS_START);
}

int stopCertusQos(void)
{
    putCertusMsg(CERTUS_STOP);
}

static void putCertusMsg(int msg)
{
    int fd, ret;

    if (access(FULL_NAME_PATH_PIPE, R_OK | W_OK) == -1) {
		PRINTF("access file, file=%s", FULL_NAME_PATH_PIPE);
        if ((mkfifo(FULL_NAME_PATH_PIPE, O_CREAT | O_EXCL) < 0)) {
    		if (errno != EEXIST) {
	    		ERR_OUT("[%s] create fifo error\n", FULL_NAME_PATH_PIPE);
    		}
	    	PRINTF("[%s] fifo file has found\n", FULL_NAME_PATH_PIPE);
    	}
	}

	if ((fd = open(FULL_NAME_PATH_PIPE, O_RDWR | O_NONBLOCK, 0)) < 0)
		ERR_OUT("[%s] open error", FULL_NAME_PATH_PIPE);


	if ((ret = write(fd, (const void *)msg, sizeof(int))) != sizeof(int)){
		PRINTF("write data to sqm program failed!send data len = %d, total len = %d\n", ret, sizeof(int));
    }

    close(fd);
Err:
    return;
}

int writeCertusConfig()
{
	char *p;
	char pppoeuser[33] = {0};
	char userid[33] = {0};
	char softver[33] = {0};
	char hwver[32] = {0};
	char versionserver[512] = {0};
	char certus_cfg[2048] = {0};
	char stbid[33] = {0};

    //write parameters to /root/certus/certus.cfg
	appSettingGetString("ntvuser", userid, 33, 0);
	sysSettingGetString("netuser", pppoeuser, 32, 0);
	get_upgrade_version(softver);
    mid_sys_serial(stbid);
	sysSettingGetString("upgradeUrl", versionserver, 512, 0);

	snprintf(certus_cfg, 2048,
		"SERVICEUSER=%s\n"\
		"PPPOEACCOUNT=%s\n"\
		"EPGSERVER=%s\n"\
		"OUI=%s\n"\
		"SN=%s\n"\
		"VERSIONSERVER=%s/\n"\
		"HWVERSION=%s\n"\
		"SWVERSION=%s\n",
		userid,
		pppoeuser,
		app_epgUrl_ip_port_get(),
		"00E0FC",
		stbid,
		versionserver,
             Hybroad_getHWtype(),
		softver
	);

	struct stat st;
	if (stat(DEFAULT_MODULE_CERTUS_DATAPATH, &st) != 0)
		mkdir(DEFAULT_MODULE_CERTUS_DATAPATH, 0755);

	int fd = open(DEFAULT_MODULE_CERTUS_DATAPATH"/certus.cfg", O_CREAT | O_WRONLY | O_TRUNC, 0644);
	if (fd == -1)
		ERR_OUT("open certus.cfg failed!");
	write(fd, certus_cfg, strlen(certus_cfg));
	close(fd);

	return 0;
Err:
	return -1;
}

unsigned int sqm_get_listen_port(void)
{
    return 37001;
}

unsigned int sqm_get_server_port(void)
{
    return 37000;
}

void sqm_set_listen_port(unsigned int port)
{
    return;
}

void sqm_set_server_port(unsigned int port)
{
    return;
}

