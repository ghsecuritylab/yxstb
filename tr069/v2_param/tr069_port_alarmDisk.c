
#include "tr069_api.h"
#include "tr069_debug.h"
#include "tr069_port_alarm.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <sys/vfs.h>
#include <dirent.h>

#ifdef Sichuan
#define ALARM_LEVEL_Disk ALARM_LEVEL_MAJOR
#else
#define ALARM_LEVEL_Disk ALARM_LEVEL_WARNING
#endif

static int gLimitAlarm = 0;

/*
struct statfs {
	long    f_type;     // �ļ�ϵͳ����
	long    f_bsize;    // �����Ż��Ĵ�����С
	long    f_blocks;   // �ļ�ϵͳ���ݿ�����
	long    f_bfree;    // ���ÿ���
	long    f_bavail;   // �ǳ����û��ɻ�ȡ�Ŀ���
	long    f_files;    // �ļ��������
	long    f_ffree;    // �����ļ������
	fsid_t  f_fsid;     // �ļ�ϵͳ��ʶ
	long    f_namelen;  // �ļ�������󳤶�
};
*/

#ifdef ANDROID
#define MOUNTDIR_PREFIX     "/mnt/sd"
#else
#define MOUNTDIR_PREFIX     "/mnt/usb"
#endif

static int diskGetMountDir(char *mountDir)
{
    FILE *fp;
    char *p0, *p1, buf[256];

    mountDir[0] = '\0';
    fp = fopen("/proc/mounts", "r");
    if (!fp) {
        TR069Error("fopen %s %s\n", mountDir, strerror(errno));
        return -1;
    }

    while (!feof(fp)) {
        buf[0] = '\0';
        if (!fgets(buf, 255, fp))
            break;

        if ('/' != buf[0])
            continue;

        buf[255] = 0;
        p0 = strchr(buf, ' ');
        if (!p0)
            continue;

        p0++;
        if (strncmp(MOUNTDIR_PREFIX, p0, strlen(MOUNTDIR_PREFIX)))
            continue;

        p1 = strchr(p0, ' ');
        if (!p1)
            continue;
        *p1 = 0;
        strcpy(mountDir, p0);
        break;
    }
    fclose(fp);

    if ('\0' == mountDir[0])
        return -1;

    return 0;
}

static int diskUsedPermille(void)
{
    char mountDir[24];
    struct statfs fs;

    if (diskGetMountDir(mountDir)) {
        TR069Error("disk not found!\n");
        return -1;
    }
    if (statfs(mountDir, &fs)) {
        TR069Error("statfs %s %s\n", mountDir, strerror(errno));
        return -1;
    }

    return alarmPermille((unsigned long long)(fs.f_blocks - fs.f_bfree), (unsigned long long)(fs.f_blocks - fs.f_bfree + fs.f_bavail));
}

/* ����ʹ�ðٷֱȳ�����ֵ���� */
void tr069_port_alarmDisk(int percentLower, int x, int level)
{
    char buffer[64];
    int limitLower, permille;

    if (percentLower <= 0) {
        gLimitAlarm = 0;
        return;
    }
    limitLower = percentLower * 10;

    permille = diskUsedPermille( );
    if (permille < 0) {
        TR069Error("permille = %d\n", permille);
        return;
    }
    TR069Debug("Disk: limitLower = %d, limitAlarm = %d, permille = %d\n", limitLower, gLimitAlarm, permille);

    if (permille > limitLower && !gLimitAlarm) {
        if (ALARM_LEVEL_Disk > level) {
            TR069Debug("Memory: ALARM_LEVEL_Disk = %d / %d\n", ALARM_LEVEL_Disk, level);
            return;
        }
        sprintf(buffer, "AlarmPost.%d.%d.%d", ALARM_TYPE_Disk_Used, ALARM_DISK_FULL_CODE, ALARM_LEVEL_Disk);
        tr069_api_setValue(buffer, "UNeed Clean-up Recorded files.", 0);
        gLimitAlarm = 1;
    } else if (permille <= limitLower && gLimitAlarm) {
        sprintf(buffer, "AlarmClear.%d", ALARM_TYPE_Disk_Used);
        tr069_api_setValue(buffer, "", 0);
        gLimitAlarm = 0;
    }
}

#if 0
#ifndef Sichuan
/*  ���̶�дʧ�ܱ���	*/
static int app_report_disk_rw_alarm(void)
{
	unsigned long long totalspace = 0;
	unsigned long long freespace = 0;
	struct statfs fs;

	if (statfs(DEFAULT_EXTERNAL_DATAPATH, &fs) == -1)
	{
		LogTr069Debug("statfs errno = %s\n", strerror(errno));
		return -1;
	}

	yhw_dm_getDiskTotalSpace(&totalspace);

	LogTr069Debug("totalspace = %lld,freespace = %lld\n",totalspace,freespace);
	if(totalspace == 0)
		return -1;

	if(fopen(DEFAULT_EXTERNAL_DATAPATH"/tms_diskRW.ini","w+") == NULL)
		tr069_port_alarm_post(ALARM_TYPE_Disk_Write,ALARM_DISK_WR_CODE,ALARM_LEVEL_MAJOR,"Hard Disk Error, Can't Access");

	return 0;
}
/*���̵��߸澯*/
int app_report_disk_offline_alarm(void)
{
	unsigned long long totalspace = 0;
	unsigned long long freespace = 0;

	LogTr069Debug("now,i will begin to alarm the usb drive offline\n");

	yhw_dm_getDiskTotalSpace(&totalspace);
	LogTr069Debug("totalspace = %lld,freespace = %lld\n",totalspace,freespace);
	if(totalspace == 0 /* || pvr_get_usb_dev() == 0*/)
	{
		LogTr069Debug("now,i   alarm the usb drive offline end\n");
		tr069_port_alarm_post(ALARM_TYPE_Disk_Write,ALARM_DISK_WR_CODE,ALARM_LEVEL_MAJOR,"Hard Disk Drive Offline");
	}
	else
	{
	LogTr069Debug("now,i not alarm this usb drive offline event\n");
	}
	return 0;
}
#endif//Sichuan

    static int gDiskCheck += 1;
    /*  ������3600����һ�δ����Ƿ��д����⵽ʧ���ϱ��������澯   */
    if(gDiskCheck == 60) {
        #if defined(brcm7405)
        app_report_disk_rw_alarm();
        #endif
        gDiskCheck = 0;
    }
#endif

