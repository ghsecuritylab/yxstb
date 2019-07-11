#ifndef __APP_SYS_H__
#define __APP_SYS_H__

#ifdef __cplusplus
extern "C" {
#endif

void mid_sys_boot_set(int flag);
int  mid_sys_boot_get(void);

typedef enum
{
	PLATFORM_ZTE,
	PLATFORM_HW,
	PLATFORM_NONE
}YX_PLATFORM;

#if (defined(SQM_VERSION_C26) ||defined(SQM_VERSION_C28) || defined(SQM_VERSION_ANDROID))

int sqm_file_check(void);

#endif

const char *sys_get_net_interface( void );
void ssh_task_create(int flag);


#ifdef __cplusplus
}
#endif


#endif

