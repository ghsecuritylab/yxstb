
#include "tr069_api.h"
#include "tr069_debug.h"
#include "tr069_port_alarm.h"

#include <stdio.h>
#include <string.h>


#ifdef Sichuan
#define ALARM_LEVEL_CPU ALARM_LEVEL_MAJOR
#else
#define ALARM_LEVEL_CPU ALARM_LEVEL_WARNING
#endif

typedef struct jiffy_counts_cpu {
	/* Linux 2.4.x has only first four */
	unsigned long long usr, nic, sys, idle;
	unsigned long long iowait, irq, softirq, steal;
	unsigned long long total;
	unsigned long long busy;
} jiffy_counts_cpu;

struct globals {
	jiffy_counts_cpu cur_jif;
	jiffy_counts_cpu prev_jif;
	char line_buf[80];
}G_cpu;

#define prev_jif    (G_cpu.prev_jif)
#define cur_jif     (G_cpu.cur_jif )
#define line_buf    (G_cpu.line_buf)

static int gLimitAlarm = -1;


 static int read_cpu_jiffy(FILE *fp, jiffy_counts_cpu *p_jif)
{
	static const char fmt[] = "cpu %llu %llu %llu %llu %llu %llu %llu %llu";
	int ret;

	if (!fgets(line_buf, 80, fp) || line_buf[0] != 'c' /* not "cpu" */)
		return 0;
	ret = sscanf(line_buf, fmt,
			&p_jif->usr, &p_jif->nic, &p_jif->sys, &p_jif->idle,
			&p_jif->iowait, &p_jif->irq, &p_jif->softirq,
			&p_jif->steal);
	if (ret >= 4) {
		p_jif->total = p_jif->usr + p_jif->nic + p_jif->sys + p_jif->idle
			+ p_jif->iowait + p_jif->irq + p_jif->softirq + p_jif->steal;
		/* procps 2.x does not count iowait as busy time */
		p_jif->busy = p_jif->total - p_jif->idle - p_jif->iowait;
	}

	return ret;
}


static void get_jiffy_counts(void)
{
	FILE *fp = fopen("/proc/stat", "r");
	if (!fp) {
		TR069Error("can't open /proc/stat\n");
		return;
	}

	/* We need to parse cumulative counts even if SMP CPU display is on,
	 * they are used to calculate per process CPU% */
	prev_jif = cur_jif;
	if (read_cpu_jiffy(fp, &cur_jif) < 4)
		TR069Error("can't read /proc/stat");

	fclose(fp);
	return;
}

static int cpuUsedPermille(void)
{
	unsigned int total_diff, used_diff;
	jiffy_counts_cpu *p_jif, *p_prev_jif;

	get_jiffy_counts();

	p_jif = &cur_jif;
	p_prev_jif = &prev_jif;

	total_diff = (unsigned int)(p_jif->total - p_prev_jif->total);
    used_diff = (unsigned int)(total_diff - (p_jif->idle - p_prev_jif->idle));
	if (total_diff == 0)
	    total_diff = 1;

	return alarmPermille((unsigned long long)used_diff, (unsigned long long)total_diff);
}

/* CPU使用百分比超过阈值报警 */
void tr069_port_alarmCPU(int percentLower, int percentUpper, int level)
{
    char buffer[64];
    int limitLower, limitUpper, limitAlarm, permille;

    if (percentLower <= 0) {
        TR069Debug("CPU: reset\n");
        gLimitAlarm = -1;
        return;
    }

    if (percentLower > 100 || percentUpper < percentLower) {
        TR069Debug("percentLower = %d, percentUpper = %d\n", percentLower, percentUpper);
        return;
    }
    limitLower = percentLower * 10;
    limitUpper = percentUpper * 10;

    limitAlarm = gLimitAlarm;
    permille = cpuUsedPermille( );
    TR069Debug("CPU: lowner = %d, upper = %d, limitAlarm = %d, permille = %d\n", limitLower, limitUpper, limitAlarm, permille);

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
        if (ALARM_LEVEL_CPU > level) {
            TR069Debug("CPU: ALARM_LEVEL_CPU = %d / %d\n", ALARM_LEVEL_CPU, level);
            return;
        }
        /* MEM's used value more than setted limit value  in 3 serial cycle ,post alarm to tr069*/
        sprintf(buffer, "AlarmPost.%d.%d.%d", ALARM_TYPE_CPU_Used, ALARM_CPU_CODE, ALARM_LEVEL_CPU);
        tr069_api_setValue(buffer, "Used CPU exceeds the limit.", 0);
	} else if(limitAlarm < 0 && gLimitAlarm >= SERIAL_SAMPLING_CYC) {
        sprintf(buffer, "AlarmClear.%d", ALARM_TYPE_CPU_Used);
        tr069_api_setValue(buffer, "", 0);
    }
    gLimitAlarm = limitAlarm;
}


static int processStatusGetValue(char *name, char *str, unsigned int size)
{
    if (strncmp("Device.DeviceInfo.ProcessStatus.", name, 32))
        return -1;
    name += 32;

    if (!strcmp(name, "CPUUsage")) {
       unsigned int Permille = cpuUsedPermille( );
        snprintf(str, size, "%d", (Permille + 9) / 10);
    } else if (!strcmp(name, "ProcessNumberOfEntries")) {
        snprintf(str, size, "%d", 1);
    } else if (!strncmp(name, "ErrorCode.", 10)) {
        return -1;
    }

    return 0;
}

static int processStatusSetValue(char *name, char *str, unsigned int x)
{
    return -1;
}

void tr069_port_processStatusInit(void)
{
    TR069Debug("init\n");
    tr069_api_registFunction("Device.DeviceInfo.ProcessStatus.CPUUsage", processStatusGetValue, processStatusSetValue);
    tr069_api_registFunction("Device.DeviceInfo.ProcessStatus.ProcessNumberOfEntries",  processStatusGetValue, processStatusSetValue);
}
