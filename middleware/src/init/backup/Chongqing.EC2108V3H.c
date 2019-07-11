#include "string.h"
#include "dhcp.h"
#include "pppoe.h"

#include "mid_stream.h"
#include "libzebra.h"

#include "MessageTypes.h"
#include "MessageValueNetwork.h"

#include "NativeHandler.h"

#include "http/http_apple.h"
#include "Assertions.h"


void app_stream_init_private_province(void)
{
    /*
    by liujianhua
    stream库已经做了调整，不用再调用mid_stream_addsize
    参看mid_stream.c里mid_stream_set_size前面的注释
    */
    //mid_stream_addsize(0, 4600 * 1316);
#if _HW_BASE_VER_ >= 57
    mid_stream_nat(1);
#endif
#if _HW_BASE_VER_ >= 58
    mid_stream_rrs_timeout(2000);
#endif
    mid_stream_set_arq(1);
    mid_stream_set_burst(1);
    mid_stream_timeshift_second(0);
    mid_stream_standard(RTSP_STANDARD_CTC_SHANGHAI);
}

int mid_dhcp_option125_check(unsigned char *buf, int len)
{
    return 0;
}

void mid_SN17toSN32(char *SN17,char *SN32)
{
    strcpy(SN32, "00100199006000010000000000000000");
    if(strncasecmp(SN17 + 8, "HWC", 3) == 0) {
        strncpy(SN32 + 4,  "02",  2);
        strncpy(SN32 + 12, "201", 3);
        strncpy(SN32 + 20, "201", 3);
    }else {
        strncpy(SN32 + 12, SN17, 1);
        strncpy(SN32 + 13, SN17 + 2, 2);
        strncpy(SN32 + 20, SN17, 1);
        strncpy(SN32 + 21, SN17 + 2, 2);
    }

    strncpy(SN32 + 16, SN17 + 4, 4);
    strncpy(SN32 + 23, SN17 + 8, 9);

}
#if 0
char *HwSoftwareVersion(int type)
{
    if(type)
        return "V100R002C15LCQD72";
    else
        return "IPTV STB V100R002C15LCQD72";
}
#endif
int HardwareVersion(char *hard_type, int len)
{
    unsigned int hardwareVer = 0;

    if(!hard_type)
        return -1;

    yhw_board_getHWVersion(&hardwareVer);
    if (hardwareVer == 0x300)
        strcpy(hard_type, "EC2108V3");
    else
        strcpy(hard_type, "EC2108");

    return 0;
}

char *SQAVersion(void)
{
    return "IPTV SQA V100R001C01B062-SPC1";
}

char *SQMVersion(void)
{
    return "IPTV SQM V100R001C28B016";
}


