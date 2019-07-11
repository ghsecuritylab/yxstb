
#include "dhcp.h"
#include "pppoe.h"

#include "mid_network.h"

#include "MessageTypes.h"
#include "MessageValueNetwork.h"

#include "NativeHandler.h"

#include "http/http_apple.h"
#include "Assertions.h"


void app_stream_init_private_province(void)
{
    int NATRHBinterval = 0;
    //mid_stream_addsize(0, 4600 * 1316);
    mid_stream_nat(1); // C57 need
    sysSettingGetInt("NATRHBinterval", &NATRHBinterval, 0);
    mid_stream_nat_heartbitperiod(NATRHBinterval);

    mid_stream_rrs_timeout(2000); // C58 need
    mid_stream_set_arq(1);
    mid_stream_set_burst(1);
    mid_stream_set_useragent("CU RTSP 1.0");
}

void mid_SN17toSN32(char *SN17,char *SN32)
{
	if( (SN17 == NULL) || (SN32 == NULL) )
		ERR_OUT("change sn from 17 to 32 error\n");

    memcpy(SN32, "00000100000820110000201000000000", 32);
    memcpy(SN32 + 16, SN17 + 4, 4);
    memcpy(SN32 + 23, SN17 + 8, 9);

Err:
    return;
}

char *HwSoftwareVersion(int type)
{
    if(1 == type)
        return "V100R002C15LLNL72";
    else
        return "IPTV STB V100R002C15LLNL72";
}

int HardwareVersion(char *hard_type, int len)
{
#if defined(brcm7405)
    YX_HW_INFO info;

    yhw_env_getHardwareInfo(&info);
    if(strlen(info.product_id) >= len) {
        strncpy(hard_type, info.product_id, len - 1);
        hard_type[len - 1] = '\0';
    } else
        strcpy(hard_type, info.product_id);
#elif defined(hi3716m)
    unsigned int hardwareVer = 0;

    if(!hard_type)
        return -1;

    yhw_board_getHWVersion(&hardwareVer);
    if (hardwareVer == 0x300)
       strcpy(hard_type, "EC2108V3H");
    else
       strcpy(hard_type, "EC2108");
#else
#error "Can't defined platform."
#endif

    return 0;
}

char *SQAVersion(void)
{
    return NULL;
}

char *SQMVersion(void)
{
    return NULL;
}