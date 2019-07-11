#include <sys/mman.h>
#include <fcntl.h>

#include "mid_fpanel.h"
#include "mid/mid_time.h"

#include "libzebra.h"

#include "hi_type.h"
#include "hi_unf_ecs.h"

#include "SysSetting.h"
#include "sys_basic_macro.h"
#include "../app/Assertions.h"

#include "NetworkFunctions.h"

#include "Tr069.h"

#ifdef TVMS_OPEN
#include "tvms_setting.h"
#endif


static int g_standby = 0;
static unsigned int RealstandbyFlag = 0;


static HI_VOID SetExternDevPower(HI_BOOL poweron);


int mid_fpanel_powerled(int pColor)
{
    return yx_hi_set_power_led(pColor); // 0 is red, 1 is green, 2 is close
}

int mid_fpanel_netled(int pOnOff)
{
    return yx_hi_set_net_led(pOnOff);
}

unsigned int mid_real_standbyFlag_get(void)
{
    return RealstandbyFlag;
}

void mid_fpanel_poweroffdeep()
{
    if (RealstandbyFlag) {
        return;
    }

    char devname[USER_LEN] = { 0 };
    network_default_devname(devname, USER_LEN);
    network_device_link_down(devname);
    RealstandbyFlag = 1;
    yhw_board_uninit();
    SetExternDevPower(HI_FALSE);

    if (0 == yhw_board_realStandby()) {
        SetExternDevPower(HI_TRUE);
        system("killall -9 sqm.elf");
#if defined(INCLUDE_DLNA)
        system("killall -9 FASTDMRAPP");
#endif
        system("killall -9 sshd");
        system("killall -9 iptv_B200.elf");
    }
    return;
}

void mid_fpanel_standby_set(int flag)
{
    if(g_standby == flag)
        return;

    if(flag == 1) {
        g_standby = 1;
        yx_drv_display_set(0);
        yx_drv_videoout_set(1, 0);
        mid_fpanel_netled(0);
        yx_hi_set_power_led(0);
#ifdef TVMS_OPEN
        tvms_config_save();
#endif
        settingManagerSave();
#ifdef INCLUDE_TR069
        extern void app_statistic_store(void);
        app_statistic_store();
#endif
    } else {
        g_standby = 0;
        yx_drv_display_set(1);
        yx_drv_videoout_set(1, 1);
        mid_fpanel_netled(1);
        yx_hi_set_power_led(1);
    }
    return;
}

int mid_fpanel_standby_get(void)
{
    return g_standby;
}

void mid_fpanel_reboot(void)
{
    LogRunOperDebug("Begin to reboot !\n");
    int i = 0;

    char ifname[URL_LEN] = { 0 };
    network_disconnect(network_default_ifname(ifname, URL_LEN));

    //yhw_vout_setMute(1); // yhw_board_enterStandby();
    yx_drv_videoout_set(1, 0);
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
}

static HI_VOID SetExternDevPower(HI_BOOL poweron)
{
    HI_S32 map_fd = -1;
    HI_U32 *tmpaddr = NULL;
    HI_U32 tmp;

    if (!poweron) {
        system("ifconfig eth0 down");
        system("ifconfig eth1 down");
        map_fd = open("/dev/mem", O_RDWR | O_SYNC);
        tmpaddr = (HI_U32 *)mmap((void *)0, 0x1000, PROT_READ | PROT_WRITE, MAP_SHARED, map_fd, 0x101e0000);

        /*0x101e005c 配置管脚复用，降低功耗*/
        tmp = *(tmpaddr + 23);
        tmp &= 0xfffffff8;
        tmp |= 0x00000006;
        *(tmpaddr + 23) = tmp;

        /*0x101e0060 关闭视频DAC*/
        tmp = *(tmpaddr + 24);
        tmp |= 0x3e;
        *(tmpaddr + 24) = tmp;

        /*0x101e0074 关闭音频DAC*/
        tmp = *(tmpaddr + 29);
        tmp = 0xc0000000;
        *(tmpaddr + 29) = tmp;
        /*0x101e0028 关闭EPLL*/
        tmp = *(tmpaddr + 10);
        tmp |= 0x017c0000;
        *(tmpaddr + 10) = tmp;

        /*0x101e0030 关闭VPLL*/
        tmp = *(tmpaddr + 12);
        tmp |= 0x017c0000;
        *(tmpaddr + 12) = tmp;

        /*0x101e003c 除了NAND和DDRC都关闭时钟*/
        *(tmpaddr + 15) = 0xffeffffe;
        munmap((HI_VOID *)tmpaddr, 0x1000);
        close(map_fd);
    }

    //SetExternDevPowerHelp(6,1,poweron);  /* AV codec */

    if (poweron) {
        map_fd = open("/dev/mem", O_RDWR | O_SYNC);
        tmpaddr = (HI_U32 *)mmap((void *)0, 0x1000, PROT_READ | PROT_WRITE, MAP_SHARED, map_fd, 0x101e0000);

        /*0x101e0038 打开所有时钟*/
        *(tmpaddr + 14) = 0xffffffff;

        /*0x101e0030 打开VPLL*/
        tmp = *(tmpaddr + 12);
        tmp &= (~0x017c0000);
        *(tmpaddr + 12) = tmp;

        /*0x101e0028 打开EPLL*/
        tmp = *(tmpaddr + 10);
        tmp &= (~0x017c0000);
        *(tmpaddr + 10) = tmp;

        HI_UNF_GPIO_Open();
        HI_UNF_GPIO_SetDirBit(33, HI_FALSE);
        HI_UNF_GPIO_WriteBit(33, HI_FALSE);
        HI_UNF_GPIO_SetDirBit(35, HI_FALSE);
        HI_UNF_GPIO_WriteBit(35, HI_TRUE);
        usleep(500000);
        HI_UNF_GPIO_Close();
    }
}

