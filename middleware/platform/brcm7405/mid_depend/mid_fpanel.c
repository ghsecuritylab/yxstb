#include "mid_fpanel.h"
#include "mid/mid_time.h"

#include "libzebra.h"

#include "SysSetting.h"
#include "sys_basic_macro.h"
#include "../app/Assertions.h"
#include "../app/Setting/Transform/SettingTransform.h"

#include "Tr069.h"
#ifdef TVMS_OPEN
#include "tvms_setting.h"
#endif


static int g_standby = 0;


int mid_fpanel_powerled(int pColor)
{
    return yhw_fp_setLEDStatus(YX_LED_POWER, pColor); // 0 is red, 1 is green, 2 is close
}

int mid_fpanel_netled(int pOnOff)
{
   return yhw_fp_setLEDStatus(YX_LED_NETWORK, pOnOff);
}

void mid_fpanel_poweroffdeep( )
{
	sfpPowerOff(); // front panel power off
}

void mid_fpanel_standby_set(int flag)
{
	LogRunOperDebug("g_standby(%d), flag(%d)\n", g_standby, flag);

	if(g_standby == flag)
		return;

	if(flag == 1) {
		g_standby = 1;
#ifndef VIETTEL_HD
        mid_fpanel_netled(0);
#endif
	    yhw_board_enterStandby();

#ifdef TVMS_OPEN
	    tvms_config_save();
#endif
        settingManagerSave( );

#ifdef INCLUDE_TR069
        extern void app_statistic_store(void);
        app_statistic_store();
#endif
	} else {
		g_standby = 0;
#ifndef VIETTEL_HD
	    mid_fpanel_netled(1);
#endif
	    yhw_board_exitStandby();
	}
	return;
}

int mid_fpanel_standby_get(void)
{
	return g_standby;
}

void mid_fpanel_showtext(char *string, int millisecond)
{
	char disp[8];
	int colon_flag = 0;

	if(NULL == string) {
		LogRunOperError("null pointer!\n");
		return;
	}
	if(strlen(string)>4){
		if(':' == string[2]){
			strncpy(disp, string, 2);
			strncpy(disp+2, string+3, 2);
			colon_flag = 1;
		}else{
			strncpy(disp, string, 4);
		}
        disp[4] = '\0';
	}else
		strcpy(disp, string);
	yhw_fp_showText(disp, millisecond);
	if(colon_flag)
		ys_front_colon(1,1);
	return;
}

void mid_fpanel_hide(void)
{
	yhw_fp_hideText( );
	return;
}

void mid_fpanel_reboot(void)
{
	LogRunOperDebug("Begin to reboot !\n");

	int i;
    char ifname[URL_LEN] = { 0 };
    network_disconnect(network_default_ifname(ifname, URL_LEN));

	yhw_vout_setMute(1); // yhw_board_enterStandby();
    mid_fpanel_powerled(0);
    mid_fpanel_netled(0);

#ifdef TVMS_OPEN
	tvms_config_save();
#endif
	settingManagerSave();
#ifdef INCLUDE_TR069
    extern void app_statistic_store(void);
    app_statistic_store();
#endif

	for(i = 0; i < 10; i ++) {
		yhw_board_rebootWithTime(2);
		mid_task_delay(10);
	}
	return;
}

