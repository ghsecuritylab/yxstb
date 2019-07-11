#include <stdio.h>
#include <stdlib.h>

#include "Assertions.h"
#include "sys_key_deal.h"
#include "sys_msg.h"
#include "app_include.h"
#include "browser_event.h"
#include "app/Assertions.h"


static dual_Appmode_e gAppMode;

extern int NativeHandlerSetState(int state);
extern int NativeHandlerGetState();

void sys_key_mode_set(const char* keymode)
{
	if (!strncmp(keymode, KEYMODE_STR_BOOT, strlen(KEYMODE_STR_BOOT))) {
		NativeHandlerSetState(0);
	} else if (!strncmp(keymode, KEYMODE_STR_CONFIG, strlen(KEYMODE_STR_CONFIG))) {
		NativeHandlerSetState(1);
	} else if (!strncmp(keymode, KEYMODE_STR_EPG, strlen(KEYMODE_STR_EPG))) {
		NativeHandlerSetState(2);
	} else if (!strncmp(keymode, KEYMODE_STR_VOD, strlen(KEYMODE_STR_VOD))) {
		NativeHandlerSetState(2);
	} else if (!strncmp(keymode, KEYMODE_STR_IPTV, strlen(KEYMODE_STR_IPTV))) {
		NativeHandlerSetState(2);
	} else if (!strncmp(keymode, KEYMODE_STR_STANDBY, strlen(KEYMODE_STR_STANDBY))) {
		NativeHandlerSetState(3);
    } else if (!strncmp(keymode, KEYMODE_STR_UPGRADE, strlen(KEYMODE_STR_UPGRADE))) {
		NativeHandlerSetState(4);
    } else if (!strncmp(keymode, KEYMODE_STR_UCONFIG, strlen(KEYMODE_STR_UCONFIG))) {
		NativeHandlerSetState(5);
	} else if (!strncmp(keymode, KEYMODE_STR_LOCAL, strlen(KEYMODE_STR_LOCAL))) {
		NativeHandlerSetState(6);
    } else if (!strncmp(keymode, KEYMODE_STR_ERROR, strlen(KEYMODE_STR_ERROR))) {
             NativeHandlerSetState(10);
	} else {
		LogUserOperDebug("ERROR:invalid key mode [%s]\n", keymode);
		return;
	}
	return;
}

int sys_appmode_get(void)
{
	return gAppMode;
}

/*对于需要区分应用模式处理，可以通过注册不同的函数来处理，这些模式只对键值有效，消息一般不在这里处理*/
void sys_appmode_set(int mode)
{
	LogUserOperDebug("Current AppMode = %d, newAppMode = %d\n", gAppMode, mode);
	gAppMode = mode;
}

int sys_frontPanelKey_hungary(int keyvalue)
{
	int trans_key = 0;
	switch(keyvalue){
		case 0xb100:
			trans_key = EIS_IRKEY_DOWN;
			break;
		case 0xb200:
			trans_key = EIS_IRKEY_SELECT;
			break;
		case 0xb300:
			trans_key = EIS_IRKEY_MENU;
			break;
		case 0xb400:
			trans_key = EIS_IRKEY_RIGHT;
			break;
		case 0xb500:
			trans_key = EIS_IRKEY_UP;
			break;
		case 0xb600:
			trans_key = EIS_IRKEY_LEFT;
			break;
		case 0xb000:
			trans_key = EIS_IRKEY_IME;
			break;
		case 0xaa00:
			trans_key = EIS_IRKEY_POWER;
			break;
		default:
			break;
	}

	return trans_key;
}

int sys_frontPanelKey_common(int keyvalue)
{
	int trans_key = 0;
	switch(keyvalue){
		case 0xb100:
			trans_key = EIS_IRKEY_SELECT;
			break;
		case 0xb200:
			trans_key = EIS_IRKEY_UP;
			break;
		case 0xb300:
			trans_key = EIS_IRKEY_LEFT;
			break;
		case 0xb400:
			trans_key = EIS_IRKEY_DOWN;
			break;
		case 0xb500:
			trans_key = EIS_IRKEY_RIGHT;
			break;
		case 0xb600:
			trans_key = EIS_IRKEY_MENU;
			break;
		case 0xaa00:
			trans_key = EIS_IRKEY_FPANELPOWER;
			break;
		default:
			break;
	}

	return trans_key;
}

int sys_frontPanelKey_shanghai(int keyvalue)
{
	int trans_key = 0;
	switch(keyvalue){
		case 0xb000:
			trans_key = EIS_IRKEY_SELECT;
			break;
		case 0xab00:
			trans_key = EIS_IRKEY_UP;
			break;
		case 0xad00:
			trans_key = EIS_IRKEY_LEFT;
			break;
		case 0xac00:
			trans_key = EIS_IRKEY_DOWN;
			break;
		case 0xae00:
			trans_key = EIS_IRKEY_RIGHT;
			break;
		case 0xaf00:
			trans_key = EIS_IRKEY_MENU;
			break;
		case 0xaa00:
			trans_key = EIS_IRKEY_POWER;
			break;
		default:
			break;
	}

	return trans_key;
}

