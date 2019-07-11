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

#include "NetworkFunctions.h"

static int g_standby = 0;
static unsigned int RealstandbyFlag = 0;

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
    if (RealstandbyFlag) {
        return;
    }

    char devname[USER_LEN] = { 0 };
    network_default_devname(devname, USER_LEN);
    network_device_link_down(devname);
    RealstandbyFlag = 1;
    yhw_board_uninit();

    if (0 == yhw_board_realStandby()){
        //exit(0);
        system("killall -9 sqm.elf");
#if defined(INCLUDE_DLNA)
        system("killall -9 FASTDMRAPP");
#endif
        system("killall -9 sshd");
        system("killall -9 iptv_B200.elf");
    }
}

unsigned int mid_real_standbyFlag_get(void)
{
    return RealstandbyFlag;
}

void mid_fpanel_standby_set(int flag)
{
    if(g_standby == flag){
		return;
	}

	if(flag == 1) { // power off
		g_standby = 1;
#if defined(Huawei_v5)
		int mode = 0;

		yhw_vout_getHdcpMode(&mode);
		if (0 != mode)
			yhw_vout_setHdcpMode(0);
#endif
        yhw_fp_setLEDStatus(YX_LED_POWER, LAMP_RED);
		yhw_fp_setLEDStatus(YX_LED_NETWORK, LAMP_OFF);

#ifdef TVMS_OPEN
		tvms_config_save();
#endif
		settingManagerSave( );
#ifdef INCLUDE_TR069
		extern void tr069StatisticStore(void);
		tr069StatisticStore();
#endif

#if defined(Cameroon_v5)
		yhw_vout_setMute(1);
#else
		yhw_board_enterFakeStandby();
#endif
	} else {
		g_standby = 0;
#if defined(Cameroon_v5)
		yhw_vout_setMute(0);
#else
        yhw_board_exitFakeStandby();
#endif
		yhw_fp_setLEDStatus(YX_LED_POWER, LAMP_GREEN);
		yhw_fp_setLEDStatus(YX_LED_NETWORK, LAMP_GREEN);
	}
	return ;
}

int mid_fpanel_standby_get(void)
{
	return g_standby;
}

void mid_fpanel_showtext(char *string, int millisecond)
{
	char disp[8] = {0};

	if(NULL == string){
		LogRunOperError("null pointer!\n");
		return;
	}
	if(strlen(string) > 4) {
		if(':' == string[2]){
			strncpy(disp, string, 2);
			strncpy(disp+2, string+3, 2);
			disp[4] = '\0';
		} else {
			strncpy(disp, string, 4);
			disp[4] = '\0';
		}

	} else {
		strcpy(disp, string);
	}
	yhw_fp_showText(disp, millisecond);
	return;
}

void mid_fpanel_hide(void)
{
	yhw_fp_hideText( );
	return ;
}

void mid_fpanel_reboot(void)
{
	LogRunOperDebug("Begin to reboot !\n");

	int i = 0;

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
    extern void tr069StatisticStore(void);
    tr069StatisticStore();
#endif

	for(i = 0; i < 10; i ++) {
		yhw_board_rebootWithTime(2);
		mid_task_delay(10);
	}
	return ;
}

