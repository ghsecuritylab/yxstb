
#include "tr069_api.h"
#include "tr069_debug.h"
#include "tr069_port_alarm.h"

#include <stdio.h>
#include <string.h>


#ifdef Sichuan
#define ALARM_LEVEL_Memory ALARM_LEVEL_MAJOR
#else
#define ALARM_LEVEL_Memory ALARM_LEVEL_WARNING
#endif

static int gLimitAlarm = 0;

static int memoryStatus(unsigned int *pTotal, unsigned int *pUsed)
{
	FILE *fp;
	char buf[80];
	unsigned long total,used,mfree, shared, buffers, cached;

    *pTotal = 0;
    *pUsed = 0;

	/* read memory info */
	fp = fopen("/proc/meminfo", "r");
	if (!fp) {
	    TR069Error("fopen\n");
	    return -1;
	}

	/*
	 * Old kernels (such as 2.4.x) had a nice summary of memory info that
	 * we could parse, however this is gone entirely in 2.6. Try parsing
	 * the old way first, and if that fails, parse each field manually.
	 *
	 * First, we read in the first line. Old kernels will have bogus
	 * strings we don't care about, whereas new kernels will start right
	 * out with MemTotal:
	 *                              -- PFM.
	 */
	if (fscanf(fp, "MemTotal: %lu %s\n", &total, buf) != 2) {
		fgets(buf, sizeof(buf), fp);    /* skip first line */

		if (fscanf(fp, "Mem: %lu %lu %lu %lu %lu %lu", &total, &used, &mfree, &shared, &buffers, &cached) != 6)
			TR069Debug("fscanf err\n");
		/* convert to kilobytes */

		used /= 1024;
		mfree /= 1024;
		shared /= 1024;
		buffers /= 1024;
		cached /= 1024;
		total /= 1024;
	} else {
		/*
		 * Revert to manual parsing, which incidentally already has the
		 * sizes in kilobytes. This should be safe for both 2.4 and
		 * 2.6.
		 */
		if (fscanf(fp, "MemFree: %lu %s\n", &mfree, buf) != 2)
                    TR069Debug("fscanf err\n");

		/*
		 * MemShared: is no longer present in 2.6. Report this as 0,
		 * to maintain consistent behavior with normal procps.
		 */
		if (fscanf(fp, "MemShared: %lu %s\n", &shared, buf) != 2)
			shared = 0;

		if (fscanf(fp, "Buffers: %lu %s\n", &buffers, buf) != 2)
                TR069Debug("fscanf err\n");

		if (fscanf(fp, "Cached: %lu %s\n", &cached, buf) != 2)
                TR069Debug("fscanf err\n");

		used = total - mfree;
	}
	fclose(fp);

    *pTotal = (unsigned int)total;
    *pUsed = (unsigned int)used;

    return 0;
}

static int memUsedPermille(void)
{
    unsigned int total, used;

    memoryStatus(&total, &used);

	return alarmPermille((unsigned long long)used, (unsigned long long)total);
}

/* 内存使用百分比超过阈值报警 */
void tr069_port_alarmMemory(int percentLower, int percentUpper, int level)
{
    char buffer[64];
    int limitLower, limitUpper, limitAlarm, permille;

    if (percentLower <= 0) {
        TR069Debug("Memory: reset\n");
        gLimitAlarm = 0;
        return;
    }
    if (percentLower > 100 || percentUpper < percentLower) {
        TR069Error("percentLower = %d, percentUpper = %d\n", percentLower, percentUpper);
        return;
    }
    limitLower = percentLower * 10;
    limitUpper = percentUpper * 10;

    limitAlarm = gLimitAlarm;
    permille = memUsedPermille( );
    TR069Debug("Memory: lowner = %d, upper = %d, limitAlarm = %d, permille = %d\n", limitLower, limitUpper, limitAlarm, permille);

    if (permille < limitUpper) {
        if(permille < limitLower)
            limitAlarm = -1;
        else if (limitAlarm < SERIAL_SAMPLING_CYC)
            limitAlarm = 0;
    } else {
        if (limitAlarm < 0)
            limitAlarm = 1;
        else if(limitAlarm < SERIAL_SAMPLING_CYC)
            limitAlarm++;
    }

    if(limitAlarm >= SERIAL_SAMPLING_CYC && gLimitAlarm < SERIAL_SAMPLING_CYC) {
        if (ALARM_LEVEL_Memory > level) {
            TR069Debug("Memory: ALARM_LEVEL_Memory = %d / %d\n", ALARM_LEVEL_Memory, level);
            return;
        }
       /* MEM's used value more than setted limit value  in 3 serial cycle ,post alarm to tr069*/
        sprintf(buffer, "AlarmPost.%d.%d.%d", ALARM_TYPE_Memory_Used, ALARM_MEM_CODE, ALARM_LEVEL_Memory);
        tr069_api_setValue(buffer, "Used memory exceeds the limit.", 0);
    } else if(!limitAlarm && gLimitAlarm >= SERIAL_SAMPLING_CYC) {
        sprintf(buffer, "AlarmClear.%d", ALARM_TYPE_Memory_Used);
        tr069_api_setValue(buffer, "", 0);
	}
    gLimitAlarm = limitAlarm;
}

static int memoryStatusGetValue(char *name, char *str, unsigned int size)
{
    unsigned int total, used;
    
    if (strncmp("Device.DeviceInfo.MemoryStatus.", name, 31))
        return -1;
    name += 31;

    if (!strcmp(name, "Total")) {
        memoryStatus(&total, &used);
        snprintf(str, size, "%d", total);
    } else if (!strcmp(name, "Free")) {
        memoryStatus(&total, &used);
        snprintf(str, size, "%d", total - used);
    } else if (!strncmp(name, "ErrorCode.", 10)) {
        return -1;
    }

    return 0;
}

static int memoryStatusSetValue(char *name, char *str, unsigned int x)
{
    return -1;
}

void tr069_port_memoryStatusInit(void)
{
    TR069Debug("init\n");
    tr069_api_registFunction("Device.DeviceInfo.MemoryStatus.Total", memoryStatusGetValue, memoryStatusSetValue);
    tr069_api_registFunction("Device.DeviceInfo.MemoryStatus.Free",  memoryStatusGetValue, memoryStatusSetValue);
}

