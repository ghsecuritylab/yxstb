
#include "dhcp.h"
#include "pppoe.h"

#include "MessageTypes.h"
#include "MessageValueNetwork.h"

#include "NativeHandler.h"
#include "mid_stream.h"
#include "Assertions.h"

#include "libzebra.h"


void app_stream_init_private_province(void)
{
    mid_stream_voddelay(1800);
    mid_stream_set_size(4600 * 1316, 8192 * 1316);
#if _HW_BASE_VER_ >= 57
	mid_stream_nat(1);
#endif
#if _HW_BASE_VER_ >= 58
	mid_stream_rrs_timeout(2000);
#endif
   // mid_stream_set_arq(1);
   // mid_stream_set_burst(1);
}

void mid_SN17toSN32(char *SN17,char *SN32)
{
	if ((SN17 == NULL) || (SN32 == NULL))
		ERR_OUT("change sn from 17 to 32 error\n");
	
    memcpy(SN32, "00100299006020110000201000000000", 32);
    memcpy(SN32 + 16, SN17 + 4, 4);
    memcpy(SN32 + 23, SN17 + 8, 9);

Err:
    return;
}

char *HwSoftwareVersion(int type)
{
#if defined(hi3716m)
    if (1 == type)
        return "R002C02LVNV31";
    else
        return "IPTV STB R002C02LVNV31";
#else
    if (1 == type)
        return "V100R002C20LVNV16";
    else
        return "IPTV STB V100R002C20LVNV16";
#endif //
}

int HardwareVersion(char *hard_type, int len)
{
    YX_HW_INFO info;

    //ys_cfg_get_hardware_info(&info);
    yhw_env_getHardwareInfo(&info);
    if(strlen(info.product_id) >= len) {
        strncpy(hard_type, info.product_id, len - 1);
        hard_type[len - 1] = '\0';
    } else {
        strcpy(hard_type, info.product_id);
    }

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