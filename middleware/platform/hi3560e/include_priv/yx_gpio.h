#ifndef __YX_GPIO_H__
#define __YX_GPIO_H__

#include "yx_type.h"

//=====================================这个需要根据硬件电路配置========================

enum gpio_pin{
	MUTE_PIN = 0,		//GPIO0_0 B(双向)
	VER_PIN1,		//GPIO0_1 B
	VER_PIN2,		//GPIO0_2 B
	VER_PIN3 = 3,		//GPIO0_3 B
	VER_PIN4,		//GPIO0_4 B
	VER_PIN5,		//GPIO0_5 B
	VER_PIN6 = 6,		//GPIO0_6 BD(双向,输入下拉)
	POWER_LAMP_PIN = 32,	//GPIO4_0 BSU(双向,斯密特触发,输入上拉)
	NET_LAMP_PIN0,		//GPIO4_1 BS(双向,斯密特触发)
	NET_LAMP_PIN1 = 35,	//GPIO4_3 BSD(双向,斯密特触发,输入下拉)
	GPIO_RESET_PIN = 37,	//GPIO4_5 BSU
	IR_LAMP_PIN = 39,	//GPIO4_7 BSD
	WIFI_LAMP_PIN0 = 54,	//GPIO6_6 BSOD
	WIFI_LAMP_PIN1 = 55,	//GPIO6_7 BSOD
};

enum lamp_color{
	PILOT_LAMP_RED = 0,
	PILOT_LAMP_GREEN,
	PILOT_LAMP_OFF,
};

enum gpio_name{
	GPIO_FOR_MUTE = 0,
	GPIO_FOR_NET_LAMP,
	GPIO_FOR_POWER_LAMP,
	GPIO_FOR_IR_LAMP,
	GPIO_FOR_VER,
	GPIO_FOR_RESET,
	GPIO_FOR_WIFI_LAMP,
};
//===================================================================================

int hisi_gpioInit_yx( void );

int hisi_gpioQuit_yx( void );

//'1'表示高电平 '0'表示低电平
static int hisi_gpioWrite_yx( int whichGPIO, int value );

//'1'使能静音,‘0’是关闭静音
int hisi_audioMute_yx( int onoff );

int hisi_netLamp_yx( int color );

int hisi_powerLamp_yx( int color );

int hisi_remoteLamp_yx( int onoff );

int hisi_getBOMVer_yx( void );

int hisi_getPCBVer_yx( void );


YX_S32  yx_middle_drv_gpio_open();
YX_VOID yx_middle_drv_gpio_close();
YX_S32  yx_middle_drv_gpio_config(YX_U32 gpio_num,YX_U32 gpio_function,YX_U32 gpio_dir);
YX_S32  yx_middle_drv_gpio_read(YX_U32 gpio_num,YX_U32 *gpio_data);
YX_S32  yx_middle_drv_gpio_write(YX_U32 gpio_num,YX_U32 *gpio_data);
YX_S32  yx_middle_drv_gpio_audio_mute(int mute);
YX_S32 yx_middle_drv_gpio_net_led(int onoff);
YX_S32 yx_middle_drv_gpio_power_led(int onoff);
YX_S32 yx_middle_drv_gpio_power_key(int onoff);

#endif /*__YX_GPIO_H__*/


