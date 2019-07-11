#include <dirent.h>
#include <sys/stat.h>
#include <sys/statfs.h>

#include "mid_sys.h"
#include "libzebra.h"
#include "config/pathConfig.h"

#include "Assertions.h"


void mid_sys_serial_set(char *sn)
{
    if(0 == ys_cfg_set_serial_code(sn))
        LogUserOperDebug("Set serial is ok!\n");
    else
        LogUserOperError("Set serial is fail!\n");
}

void mid_sys_serial(char *buf)
{
    char *sn = NULL;

    if(!buf || (0 != yhw_board_getSerialNum(&sn)))
        return;

#ifdef HUAWEI_C10
    mid_SN17toSN32(sn, buf);
#else
    strcpy(buf, sn);
#endif
    return;
}

#if 0
// 请用StbInfo::STB::Model()
// 机顶盒型号在config.xml里定义。
void mid_sys_stbtype_get(char *buf)
{
    const char* temp = NULL;

    if(!buf) {
        LogUserOperError("buf is NULL! \n");
        return;
    }

    temp = stb_upgrade_type_get();
    strcpy(buf, temp);
    return;
}
#endif

/*flag 为1显示视频层，flag为0隐藏视频层*/
void mid_sys_video_show(int flag)
{
    LogUserOperError("Can't support this function !\n");
    return ;
}

void mid_sys_videoFormat_set(int fmt, int SetToUboot)
{
    int fmt_hd, fmt_sd;

    switch(fmt) {
    case VideoFormat_NTSC:
        fmt_hd = YX_TV_SYSTEM_NTSC;
        fmt_sd = YX_TV_SYSTEM_NTSC;
        break;
    case VideoFormat_PAL:
    case VideoFormat_24P:
        fmt_hd = YX_TV_SYSTEM_PAL;
        fmt_sd = YX_TV_SYSTEM_PAL;
        break;
    case VideoFormat_SECAM:
        fmt_hd = YX_TV_SYSTEM_SECAM;
        fmt_sd = YX_TV_SYSTEM_SECAM;
        break;
    case VideoFormat_480P:
        fmt_hd = YX_TV_SYSTEM_480P;
        fmt_sd = YX_TV_SYSTEM_NTSC;
        break;
    case VideoFormat_576P:
        fmt_hd = YX_TV_SYSTEM_576P;
        fmt_sd = YX_TV_SYSTEM_PAL;
        break;
    case VideoFormat_720P50HZ:
        fmt_hd = YX_TV_SYSTEM_720P50HZ;
        fmt_sd = YX_TV_SYSTEM_PAL;
        break;
    case VideoFormat_720P60HZ:
        fmt_hd = YX_TV_SYSTEM_720P60HZ;
        fmt_sd = YX_TV_SYSTEM_NTSC;
        break;
    case VideoFormat_1080I50HZ:
        fmt_hd = YX_TV_SYSTEM_1080I50HZ;
        fmt_sd = YX_TV_SYSTEM_PAL;
        break;
    case VideoFormat_1080I60HZ:
        fmt_hd = YX_TV_SYSTEM_1080I60HZ;
        fmt_sd = YX_TV_SYSTEM_NTSC;
        break;
    case VideoFormat_1080P24HZ:
        fmt_hd = YX_TV_SYSTEM_1080P24HZ;
        fmt_sd = YX_TV_SYSTEM_PAL;
        break;
    case VideoFormat_1080P25HZ:
        fmt_hd = YX_TV_SYSTEM_1080P25HZ;
        fmt_sd = YX_TV_SYSTEM_PAL;
        break;
    case VideoFormat_1080P30HZ:
        fmt_hd = YX_TV_SYSTEM_1080P30HZ;
        fmt_sd = YX_TV_SYSTEM_NTSC;
        break;
    case VideoFormat_1080P50HZ:
        fmt_hd = YX_TV_SYSTEM_1080P50HZ;
        fmt_sd = YX_TV_SYSTEM_PAL;
        break;
    case VideoFormat_1080P60HZ:
        fmt_hd = YX_TV_SYSTEM_1080P60HZ;
        fmt_sd = YX_TV_SYSTEM_NTSC;
        break;
    case VideoFormat_UNKNOWN:
	case VideoFormat_MAX:
	default: {
	    return;
	}
    }
    yhw_vout_setTVSystem(fmt_hd, fmt_sd);
}

void mid_sys_aspectRatioMode_set(int pMode)
{
    LogUserOperDebug("Set ratio mode to %d\n", pMode);
    yhw_vout_setAspectRatio(pMode, YX_DISPLAY_ASPECT_RATIO_AUTO);
    return ;
}

void mid_sys_Macrovision_set(int level)
{
    int val;

    switch(level) {
    case 0:
        val = YX_MACROVISION_NONE;
        break;
    case 1:
        val = YX_MACROVISION_LEVEL_1;
        break;
    case 2:
        val = YX_MACROVISION_LEVEL_2;
        break;
    case 3:
        val = YX_MACROVISION_LEVEL_3;
        break;
    default:
        val = YX_MACROVISION_NONE;
        break;

    }
    yhw_vout_setMacrovision(val);
}

int mid_sys_Macrovision_get()
{
    YX_MACROVISION_LEVEL *level;

    yhw_vout_getMacrovision(level);
    if(YX_MACROVISION_NONE == level)
        return 0;
    else
        return 1;
}

int mid_sys_hdcp_enableflag_set(int enable)
{
    LogUserOperDebug("Set HDCP mode to %d\n", enable);
    if(0 != ys_set_hdcp_mode(enable)) {
        LogUserOperError("Set HDCP level to %d error\n");
        return -1;
    }
    return 0;
}

int mid_sys_hdcp_enableflag_get(void)
{
    return ys_get_hdcp_mode();
}

int mid_sys_cgmsa_enableflag_set(int enable)
{
    LogUserOperDebug("Set cgms level to %d\n", enable);
    if(enable >= -1 && enable <= 3) {
        if(0 != yhw_vout_setAnalogueProtection(enable)) {
            LogUserOperError("Set cgms level to %d error\n", enable);
            return -1;
        }
    }
    return 0;
}

int mid_sys_cgmsa_enableflag_get(void)
{
    int AnalogueProtectionLevel = 0;

    yhw_vout_getAnalogueProtection(&AnalogueProtectionLevel);
    LogUserOperDebug("Current cgms level is %d\n", AnalogueProtectionLevel);
    return AnalogueProtectionLevel;
}

int mid_sys_HDMIConnect_status_get(void)
{
    LogUserOperError("Can't support this function !\n");
    return 0;
}

int mid_sys_getVisualizationInfo(char *VisualizationInfo, int size)
{
    LogUserOperError("Can't support this function !\n");
    return -1;
}

int mid_sys_setCollectInfoType(int type)
{
    LogUserOperError("Can't support this function !\n");
    return -1;
}

int mid_sys_getCollectInfoType(void)
{
    LogUserOperError("Can't support this function !\n");
    return -1;
}

int mid_sys_getVisualizationStreamInfo(char *visualizationInfo, int size, int infoType)
{
    LogUserOperError("Can't support this function !\n");
    return -1;
}

void mid_sys_setStartCollectDebugInfo(int flag)
{
    LogUserOperError("Can't support this function !\n");
    return -1;
}

int mid_sys_getStartCollectDebugInfo(void)
{
    LogUserOperError("Can't support this function !\n");
    return -1;
}

